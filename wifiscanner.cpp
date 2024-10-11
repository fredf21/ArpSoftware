#include "wifiscanner.h"

WifiScanner::WifiScanner(QObject *parent)
    : QObject{parent}
{}
void WifiScanner::scan_wifi_network()
{
    QProcess process;
    process.start("netsh", QStringList() << "wlan" << "show" << "network");
    process.waitForFinished();
    QByteArray rawData = process.readAllStandardOutput();
    m_rawStringData = QString(rawData);
    m_available_wifi_network.clear();
    m_stringData = "";
    QStringList dataline = m_rawStringData.split("\n");
    for(const QString& line : dataline){
        if(line.trimmed().startsWith("SSID")){
            QString SSID = line.section(":",1).trimmed();
            if(SSID.isEmpty() == false){
                m_available_wifi_network.append(SSID);
                m_stringData = m_stringData + "\n" + "SSID Name : " + SSID;
            }

        }
    }
}
QStringList WifiScanner::getAvailableWifiNetwork()
{
    return m_available_wifi_network;
}

QString WifiScanner::getRawStringData()
{
    return m_rawStringData;
}

QString WifiScanner::getStringData()
{
    return m_stringData;
}
