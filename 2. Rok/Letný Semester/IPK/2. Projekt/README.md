
# Packet Sniffer pre IPK

Cieľom bolo vytvoriť sieťový analyzátor, ktorý filtruje a spracováva pakety na vybranom sieťovom rozhraní. Program bol implementovaný v jazyku C. 
Archív obsahuje súbor Makefile, ktorý slúži pre jednoduchú konštrukciu programu.

Archív xbubla02.tar obsahuje: 
- ipk-sniffer.c
- Makefile
- readme.md 
- manual.pdf (dokumentácia)

## Usage:
```javascript
$make
$./ipk-sniffer [-h] [-i | --interface <interface>] [-p <port>] [-t | --tcp] [-u | --udp]
               [--icmp] [--arp] [-n <num>]
```
PORT musí byť v rozmedzí [0 - 65535] 


## Return Codes:
```javascript
0  - Program skončil bez chyby
99 - Vnútorná chyba programu (malloc)
21 - Chyba argumentu na vstupe
22 - Chyba pri zlyhaní funkcie z knižnice PCAP
```


## Author

- Martin Bublavý [xbubla02]

