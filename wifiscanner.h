#ifndef WIFISCANNER_H
#define WIFISCANNER_H

#include <QRunnable>
#include <windows.h>
#include <wlanapi.h>
#include <QObject>

class WifiScanner : public QRunnable
{
public:
    explicit WifiScanner(QObject * receiver);
    ~WifiScanner() override;

    void DisplayNetworks(HANDLE hClient, const GUID& interfaceGuid);

private:

    QObject * m_receiver;
    HANDLE hClient = nullptr;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;
    // QRunnable interface
public:
    void run() override;
};

#endif // WIFISCANNER_H
