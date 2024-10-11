#include "widget.h"
#include "./ui_widget.h"
#include <QDebug>
#include <QtConcurrent>
#include <QThread>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_wifi_scanner = new WifiScanner(this);
    m_messageBox = new QMessageBox(this);
    pyProcess = new QProcess(this);
    socket = new QTcpSocket(this);
    ui->stopButton->setEnabled(false);
    program_in_using = "python";
    QString gateway_ip = getGatewayIp();
    my_ip = "";
    if(gateway_ip != QString("error"))
    {
        ui->gateway_ip_label->setText(gateway_ip);
        ui->gateway_mac_label->setText(getGatewayMac());
    }
    for(const QHostAddress &address : QNetworkInterface::allAddresses()){
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)){
            //declaration et assignation des variables intermédiaires
            QString address_value = QString(address.toString());
            QString gateway_value = QString(gateway_ip);

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
    ui->my_ip_label->setText(my_ip);
    ui->my_mac_label->setText(my_mac);
    /*for(const QNetworkInterface &eth : QNetworkInterface::allInterfaces()){
        if(eth.type() == QNetworkInterface::Wifi){

        }

    }
    qDebug() << "------------------------------------------------------";
    qDebug() << "-------------------------------------------------------";
    qDebug() << "-------------------------------------------------------";*/
    m_wifi_scanner->scan_wifi_network();
    m_wifi_scanner->getAvailableWifiNetwork();

    ui->operation_output_TextEdit->appendPlainText(m_wifi_scanner->getStringData());
    connect(ui->startButton, &QPushButton::clicked, this, &Widget::startButton_clicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &Widget::stopButton_clicked);

    connect(ui->target_ip_comboBox, &QComboBox::currentTextChanged, this, &Widget::ip_changed);
    getAllIpMac(gateway_ip + "/24");
    startTheServer();
    connectToTheServer();
}

Widget::~Widget()
{
    stopTheServer();
    delete ui;
}

QString Widget::getGatewayIp()
{
    QString program_in_using = "python";
    QStringList arg = QStringList() << "get_gateway_ip.py";
    pyProcess->start(program_in_using, arg);
    pyProcess->waitForFinished(-1);
    QByteArray passerelleIpRawData = pyProcess->readAllStandardOutput();
    QByteArray passerelleIpErrorRawData = pyProcess->readAllStandardError();
    QString passerelleIpData = QString(passerelleIpRawData);
    QString passerelleIpErrorData = QString(passerelleIpErrorRawData);
    if(passerelleIpErrorData != QString("")){
        pyProcess->close();
        return QString("error");
    }
    pyProcess->close();
    return passerelleIpData;
}

QString Widget::getGatewayMac()
{
    QString gateway_ip = getGatewayIp();
    if(gateway_ip != QString("error"))
    {
        QString program_in_using = "python";
        QStringList arg = QStringList() << "get_gateway_mac.py" << gateway_ip;
        pyProcess->start(program_in_using, arg);
        pyProcess->waitForFinished(-1);
        QByteArray passerelleMacRawData = pyProcess->readAllStandardOutput();
        QByteArray passerelleMacErrorRawData = pyProcess->readAllStandardError();
        QString passerelleMacData = QString(passerelleMacRawData);
        QString passerelleMacErrorData = QString(passerelleMacErrorRawData);
        if(passerelleMacErrorData != QString("")){
            pyProcess->close();
            return QString("error");
        }
        return passerelleMacData;
    }
    return QString("error");
}

QString Widget::getMyMac(const QString ip)
{
    if(ip != QString("error"))
    {

        QStringList arg = QStringList() << "get_gateway_mac.py" << ip;
        pyProcess->start(program_in_using, arg);
        pyProcess->waitForFinished(-1);
        QByteArray myMacRawData = pyProcess->readAllStandardOutput();
        QByteArray myMacErrorRawData = pyProcess->readAllStandardError();
        QString myMacData = QString(myMacRawData);
        QString myMacErrorData = QString(myMacErrorRawData);
        if(myMacErrorData != QString("")){
            pyProcess->close();
            return QString("error");
        }
        return myMacData;
    }
    return QString("error");
}

QString Widget::getAllIpMac(QString ip_range){
        QStringList arg = QStringList() << "get_all_mac_ip.py" << ip_range;
        pyProcess->start(program_in_using, arg);
        pyProcess->waitForFinished(-1);
        QByteArray allMacRawData = pyProcess->readAllStandardOutput();
        QByteArray allMacErrorRawData = pyProcess->readAllStandardError();
        QString allMacData = QString(allMacRawData);
        QString allMacErrorData = QString(allMacErrorRawData);
        if(allMacErrorData != QString("")){
            pyProcess->close();
             return QString("error");
        }
        list_of_address.clear();
        QList<QString> splitarray = allMacData.split('\n');
        for(auto& info : splitarray){
            QVector<QString> splitinfo = info.split(' ');
            QString previous_key;
            for(int i=0; i<splitinfo.size(); i++){
                if(splitinfo[i].contains('\r')){
                    splitinfo[i].removeLast();

                }
                else{
                    if(splitinfo[i] != "" &&splitinfo[i]!= my_ip && splitinfo[i] != ui->gateway_ip_label->text()){
                        ui->target_ip_comboBox->addItem(splitinfo[i]);
                        previous_key = splitinfo[i];
                    }

                }
                if(i%2 != 0 && !splitinfo.empty() && splitinfo[i] != my_mac && splitinfo[i] != ui->gateway_mac_label->text()){
                    list_of_address.insert(previous_key, splitinfo[i]);
                }

            }

        }
        for(auto [key, value] : list_of_address.asKeyValueRange()){
            if(key == ui->target_ip_comboBox->currentText()){
                ui->target_mac_label->setText(list_of_address.value(key));
            }
        }

        return allMacData;
}

void Widget::startTheServer()
{
      // Port du serveur
    QStringList arg = QStringList() << "arp_spoofing.py";
    pyProcess->start(program_in_using, arg);
    if(!pyProcess->waitForStarted()){
        m_messageBox->setText("Le démarrage du serveur a échoué");
    }

}

void Widget::stopTheServer()
{

    if(pyProcess->state() == QProcess::Running){
        QString command = "EXIT";
        QTextStream stream(socket);
        stream << command;
        socket->flush();
        pyProcess->terminate();
        if(!pyProcess->waitForFinished()){
            m_messageBox->setText("impossible d'arréter le serveur faites le manuellement dans les processus de windows");
            m_messageBox->exec();
        }
    }
}

void Widget::connectToTheServer()
{
    QString host = "127.0.0.1";  // Adresse du serveur
    quint16 port = 8000;
    socket->connectToHost(host, port);
    if (socket->waitForConnected(3000)) {
        ui->operation_output_TextEdit->appendPlainText("\nConnected to server.");
    } else {
        m_messageBox->setText("Connexion au serveur échoué");
        m_messageBox->exec();
    }
}

void Widget::startButton_clicked()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        QString command = ui->target_ip_comboBox->currentText()+ " "+
                         ui->target_mac_label->text()+ " "+
                         ui->my_mac_label->text()+ " "+
                         ui->gateway_ip_label->text()+ " "+
                         ui->target_mac_label->text();

        QTextStream stream(socket);
        stream << command;
        socket->flush();
    } else {
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        m_messageBox->setText("Vous n etes pas au serveur");

    }
}

void Widget::stopButton_clicked()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        QTextStream stream(socket);
        stream << "STOP";
        socket->flush();
        qDebug() << "Sent 'STOP' command.";
    } else {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        m_messageBox->setText("Vous n etes pas au serveur");
        qDebug() << "Not connected to server.";
    }
}

void Widget::ip_changed(QString ip_address)
{
    for(auto [key, value] : list_of_address.asKeyValueRange()){
        if(key == ip_address){
            ui->target_mac_label->setText(list_of_address.value(key));
        }
    }
}
