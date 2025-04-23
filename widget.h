#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QProcess>
#include <QMessageBox>
#include <QStringListModel>
#include <winsock2.h>
#include "wifiscanner.h"
#include "wificrackerworker.h"
#include "spoofingworker.h"
#include "ArpPacket.h"


//C:\Users\fredf\QtProjects\ArpSoftware\npcap-sdk-1.13

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    WifiScanner *m_wifi_scanner;
    QString getGatewayIp();
    QString getGatewayMac();
    QString getMyMac(const QString ip);
    QString getAllIpMac(QString ip_range, int& length);

    void stopArpSpoofingThread();
    void stopWifiCrackerThread();

    pcap_if_t* getInterfaceByIP();
    void convertIpAddress(const QString& ipString, uint8_t ipBytes[4]);
    void refreshOtherAddressInfo();
    void refreshMyAddressInfo();
    QString getCurrentConnectedWifiSsid();
    void reScanWifi();
public slots:
    void getAvailableWifiNetwork(QString ssid, QString signal);
private slots:


    void wifiRescanButton_clicked();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief WifiCracker socket slots
    void wifiResultOPeration(bool value, QString passwordValue, bool final);


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief ArpSpoofing socket slots

    void spoofingThreadStarted();
    void spoofingThreadStatus(bool value);
    void spoofingResultOPeration(bool value);
    void spoofingThreadStopped();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief ArpSpoofing and WifiCracker button slots
    void wifiCrackerButton_clicked();

    void stopWifiCrackingButton_clicked();

    void startButton_clicked();

    void stopButton_clicked();

    void targetMacChanged(const QString& value);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief refresh interface
    void refreshButton_clicked();
    void reloadMyInfoButton_clicked();

private:
    /////////////////////////////////////////
    /// \brief UI variables
    Ui::Widget *ui;
    WifiScanner *m_wifiscanner;
    QMessageBox *m_arp_messageBox;
    QMessageBox *m_wifi_messageBox;

    QString my_ip;
    QString my_mac;
    QMap<QString, QString> m_list_of_address;


    QString m_program_in_using;
    QString m_gateway_ip;

    QProcess *m_pyProcessForArp;
    QProcess *m_currentWifiProcess;


    QStringListModel *m_wifiListModel;
    QStringList m_list;
    QString m_interface_name;

    QFont gui_font;

    SpoofingWorker *m_spoofingWorker;
    wifiCrackerWorker  *m_wifiCrackerWorker;
    ArpPacket* packet_to_send_to_Gateway;
    ArpPacket* packet_to_send_to_targetIp;

    pcap_if_t *pcap_iface;//interface du reseau connectE
    QString pcap_ifaceName;
    QString m_selectedip;
    QString m_selectedssid;
    bool m_arp_spoofing_is_running;
    bool m_wifi_cracking_is_running;
    int m_wifi_count;


};
#endif // WIDGET_H
