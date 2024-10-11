#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include "wifiscanner.h"
#include <QMessageBox>

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
    QString getAllIpMac(QString ip_range);
    void startTheServer();
    void stopTheServer();
    void connectToTheServer();


private slots:
    void startButton_clicked();
    void stopButton_clicked();
    void ip_changed(QString ip_address);

private:
    Ui::Widget *ui;
    WifiScanner *m_wifiscanner;
    QProcess *pyProcess;
    QMessageBox *m_messageBox;
    QString my_ip;
    QString my_mac;
    QMap<QString, QString> list_of_address;
    QTcpSocket *socket;
    QString program_in_using;

};
#endif // WIDGET_H
