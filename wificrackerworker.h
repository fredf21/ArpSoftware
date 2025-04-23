#ifndef WIFICRACKERWORKER_H
#define WIFICRACKERWORKER_H

#include <QFile>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <windows.h>
#include <wlanapi.h>
class wifiCrackerWorker : public QThread
{
    Q_OBJECT
public:
    explicit wifiCrackerWorker(QString &ssid, QObject *parent = nullptr);
    ~wifiCrackerWorker();
    bool connectToWifi(QString &ssid, const QString &password, QString& response);
    QString returnProfile(QString& ssid, const QString& password) const;
    void setSSid(QString &ssid);

signals:
    void wifiResultOperation(bool value, QString password_value, bool final);
    void wifiThreadStatus(bool value);

public slots:
    void wifiCrackingPause();
    void wifiCrackingResume();
    void wifiOperationResult(bool value);

    // QThread interface
protected:
    void run() override;

private:
    QMutex m_mutex;
    QWaitCondition m_pause_condition;
    bool m_pause;
    QString m_ssid;
    bool m_value;

    std::unique_ptr<QFile> m_passwordFile;
    bool m_passwordFile_is_open;
    QString m_response_from_connectToWifi;
};

#endif // WIFICRACKERWORKER_H
