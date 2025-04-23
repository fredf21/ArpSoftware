#include "threadforwifilist.h"


ThreadForWifiList::ThreadForWifiList()
{

}

void ThreadForWifiList::DisplayNetworks(HANDLE hClient, const GUID &interfaceGuid)
{
    PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = nullptr;
    if (WlanGetAvailableNetworkList(hClient, &interfaceGuid, 0, nullptr, &pNetworkList) == ERROR_SUCCESS) {
        //std::wcout << L"Réseaux disponibles :" << std::endl;
        for (DWORD i = 0; i < pNetworkList->dwNumberOfItems; ++i) {
            WLAN_AVAILABLE_NETWORK network = pNetworkList->Network[i];
            /*std::wcout << L"SSID: " << std::wstring((wchar_t*)network.dot11Ssid.ucSSID, network.dot11Ssid.uSSIDLength)
                       << L", Signal: " << network.wlanSignalQuality << L"%"
                       << std::endl;*/
        }
        WlanFreeMemory(pNetworkList);
    } else {
        //std::wcerr << L"Erreur lors de la récupération des réseaux disponibles." << std::endl;
    }

}

void ThreadForWifiList::run()
{
    if (WlanOpenHandle(dwMaxClient, nullptr, &dwCurVersion, &hClient) != ERROR_SUCCESS) {
        //std::wcerr << L"Erreur lors de l'ouverture de la session WLAN." << std::endl;
    }

    // Récupération des interfaces Wi-Fi
    if (WlanEnumInterfaces(hClient, nullptr, &pIfList) == ERROR_SUCCESS) {
        for (DWORD i = 0; i < pIfList->dwNumberOfItems; ++i) {
            WLAN_INTERFACE_INFO interfaceInfo = pIfList->InterfaceInfo[i];
            //std::wcout << L"Interface trouvée: " << interfaceInfo.strInterfaceDescription << std::endl;

            // Découverte des réseaux en temps réel
            while (true) {
                if (WlanScan(hClient, &interfaceInfo.InterfaceGuid, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                    DisplayNetworks(hClient, interfaceInfo.InterfaceGuid);
                } else {
                   // std::wcerr << L"Erreur lors du scan des réseaux." << std::endl;
                }
                Sleep(5000); // Attente de 5 secondes avant le prochain scan
            }
        }
        WlanFreeMemory(pIfList);
    } else {
        //std::wcerr << L"Erreur lors de l'énumération des interfaces." << std::endl;
    }

    // Fermeture de la session WLAN
    WlanCloseHandle(hClient, nullptr);
}
