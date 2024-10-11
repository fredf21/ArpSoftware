La classe WiFiScanner utilise un simple netsh pour scanner les reseaux a
proximités. La class Widget utilise dans un premier temps le network
manager pour déterminer l'adresse ip et macde la machine ensuite il
lance un script python qui utilise le paquet scapy pour determiner l
adresse ip et mac de la passerelle respectivement get_gateway_ip et
get_gateway_mac ensuite il demarre le script get_all_mac_ip pour
recevoir toutes les adresse ip et mac connectées à la passerelle qu'il
range dans un tableau de type QMap ayant l adresse ip comme clef et l
adresse mac comme valeur, il affiche ensuite ce tableau dans un
QComboBox qui liste les valeurs qui sont les clef et affiche l adresse
ip et le mac du dernier element du tableau dans deux QLabel qui
represente la clef(adresse ip) et valeur(adresse mac) enfin il lance un
le script arp_spoofing qui est un serveur qui communique par socket; ce
serveur python attend des commandes de l application Qt et en fontion de
cette commande il lance une tache de fond qui boucle infiniment proceder
a des attaques arp entre la machine choisi et la passerelle ou une
commande STOP pour arrêter l'attaque ou EXIT pour arrêter le serveur.
