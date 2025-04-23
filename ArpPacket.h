#ifndef ARPPACKET_H
#define ARPPACKET_H
static_assert(true);
#pragma pack(push, 1)
struct ArpPacket {
    unsigned char dest_mac[6];  // MAC destination
    unsigned char src_mac[6];   // MAC source
    unsigned short eth_type;    // Type Ethernet (0x0806 pour ARP)

    unsigned short htype;       // Type de matériel (Ethernet = 1)
    unsigned short ptype;       // Type de protocole (IPv4 = 0x0800)
    unsigned char hlen;         // Longueur adresse matérielle (6)
    unsigned char plen;         // Longueur adresse protocole (4)
    unsigned short operation;   // 1 = Requête ARP, 2 = Réponse ARP

    unsigned char sender_mac[6]; // MAC source
    unsigned char sender_ip[4];  // IP source
    unsigned char target_mac[6]; // MAC destination
    unsigned char target_ip[4];  // IP destination
};
#pragma pack(pop)

#endif // ARPPACKET_H
