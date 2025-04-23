#ifndef SPOOFINGWORKER_H
#define SPOOFINGWORKER_H

#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <pcap.h>
#include "ArpPacket.h"
class SpoofingWorker : public QThread
{
    Q_OBJECT
public:
    explicit SpoofingWorker(QString& interface_name, QString &gatewayIp, QString &targetIp, ArpPacket& gateway_packet,  ArpPacket& target_packet, QObject *parent = nullptr);
    ~SpoofingWorker();
    void setInterfaceName(const QString& interface_name);
    void setGatewayIp(const QString& gatewayIp);
    void setTargetIp(const QString& targetIp);
    void setGatewayPacket(const ArpPacket& packet);
    void setTargetPacket(const ArpPacket& packet);
    static size_t count;

signals:
    void spoofingResultOperation(bool value);
    void spoofingThreadStatus(bool value);

public slots:
    void pause();
    void resume();


    // QThread interface
protected:
    void run() override;
private:
    QMutex m_mutex;
    QWaitCondition m_pause_condition;
    bool m_pause;
    pcap_t *m_gateway_handle;
    pcap_t *m_target_handle;
    ArpPacket& m_gateway_packet;
    ArpPacket& m_target_packet;

    QString& m_interface_name;
    QString& m_gatewayIp;
    QString& m_targetIp;
    bool value;
};

#endif // SPOOFINGWORKER_H
