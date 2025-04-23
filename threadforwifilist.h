#ifndef THREADFORWIFILIST_H
#define THREADFORWIFILIST_H

#include <QObject>
#include <QThread>
#include <QRunnable>
#include <windows.h>
#include <wlanapi.h>

class ThreadForWifiList : public QRunnable
{
public:
    explicit ThreadForWifiList();
    void DisplayNetworks(HANDLE hClient, const GUID& interfaceGuid);


private:

    HANDLE hClient = nullptr;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;

    // QRunnable interface
public:
    void run() override;
};

#endif // THREADFORWIFILIST_H
