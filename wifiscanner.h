#ifndef WIFISCANNER_H
#define WIFISCANNER_H

#include <QObject>
#include <QtNetwork>

class WifiScanner : public QObject
{
    Q_OBJECT
public:
    explicit WifiScanner(QObject *parent = nullptr);
    void scan_wifi_network();
    QStringList getAvailableWifiNetwork();
    QString getRawStringData();
    QString getStringData();

private:
    QStringList m_available_wifi_network;
    QString m_rawStringData;
    QString m_stringData;
};

#endif // WIFISCANNER_H
