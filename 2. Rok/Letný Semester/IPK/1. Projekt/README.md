
# Server pre IPK

Cieľom bolo vytvoriť web server, ktorý komunikuje prostredníctvom protokolu HTTP a 
poskytuje rôzne informácie o systéme.

Server bol implementovaný v jazyku C. Na komunikáciu používa TCP/IP protokol. 
Na začiatku vytvorí TCP socket, ktorý umožňuje komunikáciu medzi serverom a klientom a potom
čaká na prichádzajúce pripojenia pomocou systémového volania **listen()**. Po prijatí systémovým 
volaním **accept()** zistí aké si klient žiada dáta, vyhodnotí ich a pošle naspäť.
Spojenie sa potom ruší a server čaká na ďalšie pripojenie.
## Usage Server:
```javascript
$make
$./hinfosvc [PORT]
```
PORT musí byť v rozmedzí [0 - 65535] -> TCP/IP rozsah 


## Usage Client:
[Wanted Data] -> [hostname | load | cpu-name]
```javascript
$GET [IP]:[PORT]/[Wanted Data]
$curl [IP]:[PORT]/[Wanted Data]
```
```javascript
$GET localhost:12345/hostname
$curl localhost:12345/load
$curl 127.0.0.1:12345/cpu-name
```


## Author

- Martin Bublavý [xbubla02]

