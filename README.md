# Dokumentace IPK-24 TCP/UDP Chatovacího klienta 


**Autor:** Samuel Hejníček  
**Datum:** 1. dubna 2024  


## Obsah
- [Dokumentace IPK-24 TCP/UDP Chatovacího klienta](#dokumentace-ipk-24-tcpudp-chatovacího-klienta)
  - [Obsah](#obsah)
  - [Úvod](#úvod)
    - [Stručný popis](#stručný-popis)
    - [Spuštění programu](#spuštění-programu)
    - [Podporované příkazy](#podporované-příkazy)
  - [Stručná teorie k programu](#stručná-teorie-k-programu)
    - [TCP Protokol](#tcp-protokol)
    - [UDP Protokol](#udp-protokol)
    - [Funkce poll](#funkce-poll)
  - [Implementace projektu](#implementace-projektu)
    - [Návrh](#návrh)
    - [Zpracování vstupních argumentů](#zpracování-vstupních-argumentů)
    - [Způsob zahájení komunikace](#způsob-zahájení-komunikace)
    - [Příjmání a odesílání zpráv](#příjmání-a-odesílání-zpráv)
    - [Ztráta příchozích paketů u UDP](#ztráta-příchozích-paketů-u-udp)
    - [Kontrola čísla příchozích paketů u UDP](#kontrola-čísla-příchozích-paketů-u-udp)
    - [Ukončení programu](#ukončení-programu)
  - [Testování programu](#testování-programu)
    - [TCP klient](#tcp-klient)
    - [UDP klient](#udp-klient)
  - [Možná vylepšení](#možná-vylepšení)
  - [Zdroje](#zdroje)


## Úvod

Tento soubor je dokumentací k prvnímu projektu do předmětu [IPK], Počítačové komunikace a sítě, veškeré zdrojové soubory jsou dostupné [zde](https://git.fit.vutbr.cz/xhejni00/IPK_Project_1). Projekt je napsán v C++ (standardu 20) a určen pro platformu Linux. 

### Stručný popis

Program slouží jako chatovací klient schopný komunikace se vzdáleným serverem pomocí protokolu IPK24-CHAT, který zajíšťuje definovaný formát zpráv posílaných mezi sebou.Tento protokol je navíc postaven nad transportní protokoly TCP/UDP, přičemž jeden z nich klient k připojení na vzdálený server používá. Se serverem klient komunikuje pomocí několika definovaných zpráv viz [Spuštění a ovládání programu](#spuštění-a-ovládání-programu). V případě, že chce klient ukončit ukončit komunikaci se serverem, může tak učinit pomocí CTRL-C, které server informuje o konci spojení a ukončí program.

### Spuštění programu

Ke kompilaci programu stačí zadat příkaz make (ve složce se zdrojovými soubory), který vytvoří spustitelný soubor `./ipk24chat-klient`
Ten lze následně spustit s následujícími parametry:

| Argument | Hodnota       | Typ                    | Popis                            |
|----------|---------------|------------------------|-------------------------------------------|
| `-t`     | Od uživatele  | `tcp` nebo `udp`       | Transportní protokol použit k připojení   |
| `-s`     | Od uživatele  | IP adresa nebo jméno   | IP adresa nebo jméno serveru              |
| `-p`     | 4567          | `uint16`               | Serverový port                            |
| `-d`     | 250           | `uint16`               | UDP potvrzení timeoutu (v ms).            |
| `-r`     | 3             | `uint8`                | Maximální počet opakování odeslání zprávy |
| `-h`     |               |                        | Vypíše nápovědu a skončí program.         |

Příklad: 

```sh
./ipk24chat-client -t tcp -s localhost -p 8312
```

Parametry `-t`  a `-s` jsou povinné, jelikož bez nich se program správně nikdy nemůže připojit k serveru a skončí s chybovým hlášením, zbytek argumentů je dobrovolný a hodnota v tabulce udává výchozí hodnotu. 

### Podporované příkazy
* `/auth <username> <key> <displayname>` 
Ověření totožnosti při připojení na server. Nejprve je nutné zadat uživatelské jméno, klíč (secret) a následně přezdívku, tedy jméno, které bude použito pro veřejné vystupování na serveru.
* `/rename <displayname>` Změní přezdívku, pod kterou uživatel posílá zprávy
* `/join <channellID>` Připojí se do jiného chatovacího kanálu
* `/help` Vypíše seznam podporovaných příkazů

## Stručná teorie k programu

### TCP Protokol
TCP protokol je protokol transportní vrstvy používán na spolehlivou výměnu dat. TCP umožňuje zasílání kontinuálního proudu bytů a přenos je spojovaný a spolehlivý, což se pojí s vyšší režíí. Aby data vůbec mohla být zasílána je nutné vytvořit spojení, které se děje v pomocí tzv. 3-way handshake mechanismu. TCP protokol zajišťuje, že zprávy odeslané z jednoho zařízení dorazí do druhého zařízení ve stejném pořádí jako byly odeslány. Pokud je v TCP zjištěna ztráta paketu je automaticky spuštěn proces opětovného odeslání. Jednotlivé zprávy jsou odděleny \r\n, aby jej chatovací klient mohl rozlišit

### UDP Protokol
UDP protokol je protokol transportní vrstvy, který nezajišťuje spolehlivou výměnu dat. UDP je na bázi "best effort delivery", který znamená, že u dat není zaručeno pořadí doručení paketů ani to, že data budou doručena v pořádku. Narozdíl od TCP nemusí navázat žádné spojení před odesláním samotných dat, nemá žádné mechanismy pro opětovné odesílání dat nebo potvrzení přijetí a tyto věci jsou tak řešeny specificky v chatovacím klientovi (opětovné odeslání zprávy, vypršení timeoutu). 

### Funkce poll
Funkce poll je funkce standardní knihovny jazyka C, která slouží ke sledování více file descriptorů současně na přicházející události z různých zdrojů. V kontextu chatovacího klienta je přicházející událostí příchozí zpráva od serveru na soket nebo čtení uživatelského vstupu, přičemž funkce poll není blokující a není nutný vícevláknový přístup v programování. Tato funkce je nutná, jelikož např. samotné čekání na zprávu je blokující operace a uživatel by tak musel čekat na zprávu od serveru, která ale není v daném okamžiku vůbec nutná.

## Implementace projektu

### Návrh
Projekt je rozdělen do několik zdrojových souborů a díky programovacímu jazyku C++ je napsán se snahou využití objektově orientovaného programování.
* `main.cpp a main.hpp` - Funkce main a funkce pro korektní ukončení programu s dealokací paměti, statická třída pro odchycení CTRL-C
* `socket.cpp a socket.hpp` - Třída socket pro uchovávání informací o socketu, jeho tvorby a záník
* `arg_parser.cpp a arg_parser.hpp` - Statická třída pro zpracování vstupních argumentů
* `messages.cpp a messages.hpp` - Abstraktní třída NetworkMessage a jednotlivé podtřídy pro TCP a UDP zprávy
* `clients.cpp a clients.hpp` - Abstraktní třída NetworkClient a jednotlivé podtřídy pro TCP a UDP klienta

### Zpracování vstupních argumentů
Na začátku programu dochází ke zpracování argumentů od uživatele pomocí statické metody `parse_args` a jejich uložení do specifické struktury, v případě nezadaných volitelných argumentů jsou dané hodnoty nastaveny na výchozí a v případě zadání chybného atributu například příliš vysokého čísla portu nebo špatně zvoleného transportního protokolu je program ukončen s chybou.

### Způsob zahájení komunikace

### Příjmání a odesílání zpráv
### Ztráta příchozích paketů u UDP 
### Kontrola čísla příchozích paketů u UDP
### Ukončení programu

## Testování programu

### TCP klient
### UDP klient
Detail the testing methodologies and validation procedures employed, such as:
- Unit tests and integration tests that were conducted.
- Any test frameworks used.
- Examples of tests and their outcomes.
- How these tests prove the reliability and functionality of the UDP/TCP client.

## Možná vylepšení
Chatovací klient jistě není dokonalý a obsahuje několik věcí, které by mohly být v budocnu vylepšeny, mezi ně patří například:

* Vylepšení mechanismu timeoutů, kdy doba do vypršení timeoutu aktuálně nereflektuje skutečnou dobu, pokud před zprávou CONFIRM dorazí jiná zpráva
* Refaktorizace kódu, kdy je možné více sjednotit funkce pro hlavní logiku jak TCP tak UDP klienta, vyčlenění funkcí do dalších tříd (logika zpracovávání zpráv)
* Přídání dalších podporovaných příkazů
* Přidání časového razítka při odeslaných a přijatých zprávách
## Zdroje

List all the sources used during the development of the project. This could include:
- Books, articles, and papers on networking and protocol theory.
- Documentation for any libraries or frameworks used.
- Websites, forums, and discussion threads that provided useful insights or solutions.

Remember, a good bibliography not only credits sources but also helps others find resources for learning and problem-solving.
