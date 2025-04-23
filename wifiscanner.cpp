#include "wifiscanner.h"
#include <QMetaObject>
#include <QDebug>

WifiScanner::WifiScanner(QObject* receiver) : m_receiver(receiver)

{
}

WifiScanner::~WifiScanner()
{

}


void WifiScanner::run()
{
    if (WlanOpenHandle(dwMaxClient, nullptr, &dwCurVersion, &hClient) != ERROR_SUCCESS) {
        qDebug() << "Erreur lors de l'ouverture de la session WLAN.";
    }

    // Récupération des interfaces Wi-Fi
    if (WlanEnumInterfaces(hClient, nullptr, &pIfList) == ERROR_SUCCESS) {
        if(pIfList->dwNumberOfItems > 0) {
            WLAN_INTERFACE_INFO interfaceInfo = pIfList->InterfaceInfo[0];
            qDebug() << "Interface trouvée: " << interfaceInfo.strInterfaceDescription;

            // Découverte des réseaux en temps réel
            if (WlanScan(hClient, &interfaceInfo.InterfaceGuid, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                DisplayNetworks(hClient, interfaceInfo.InterfaceGuid);
            }
            else {
                    qDebug() << "Erreur lors du scan des réseaux.";
            }
        }
        WlanFreeMemory(pIfList);
    }
    else {
        qDebug()<< "Erreur lors de l'énumération des interfaces.";
    }

    // Fermeture de la session WLAN
    WlanCloseHandle(hClient, nullptr);
}

void WifiScanner::DisplayNetworks(HANDLE hClient, const GUID &interfaceGuid)
{
    PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = nullptr;
    if (WlanGetAvailableNetworkList(hClient, &interfaceGuid, 0, nullptr, &pNetworkList) == ERROR_SUCCESS) {
        qDebug()<< "Réseaux disponibles :";
        for (DWORD i = 0; i < pNetworkList->dwNumberOfItems; ++i) {
            WLAN_AVAILABLE_NETWORK network = pNetworkList->Network[i];
            QString ssid = QString::fromUtf8(reinterpret_cast<const char*>(network.dot11Ssid.ucSSID), network.dot11Ssid.uSSIDLength);
            QMetaObject::invokeMethod(m_receiver, "getAvailableWifiNetwork", Qt::QueuedConnection, Q_ARG(QString, ssid), Q_ARG(QString,  QString::number(network.wlanSignalQuality)));
            qDebug() << "SSID: " << ssid << ", Signal: " << network.wlanSignalQuality << "%";
        }
        WlanFreeMemory(pNetworkList);
    } else {
        qDebug() << "Erreur lors de la récupération des réseaux disponibles.";
    }

}


