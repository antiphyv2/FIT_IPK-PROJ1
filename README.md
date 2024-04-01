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
    - [Obsah souborů](#obsah-souborů)
    - [Zpracování vstupních argumentů](#zpracování-vstupních-argumentů)
    - [Způsob zahájení komunikace](#způsob-zahájení-komunikace)
    - [Příjmání a odesílání zpráv](#příjmání-a-odesílání-zpráv)
    - [Kontrola syntaxe zpráv](#kontrola-syntaxe-zpráv)
    - [Validace zpráv dle konečného automatu](#validace-zpráv-dle-konečného-automatu)
    - [Blokování uživatelského vstupu](#blokování-uživatelského-vstupu)
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
Funkce poll je funkce standardní knihovny jazyka C, která slouží ke sledování více file descriptorů současně na přicházející události z různých zdrojů. V kontextu chatovacího klienta je přicházející událostí příchozí zpráva od serveru na socket nebo čtení uživatelského vstupu, přičemž funkce poll není blokující a není nutný vícevláknový přístup v programování. Tato funkce je nutná, jelikož např. samotné čekání na zprávu je blokující operace a uživatel by tak musel čekat na zprávu od serveru, která ale není v daném okamžiku vůbec nutná.

## Implementace projektu

### Obsah souborů
Projekt je rozdělen do několik zdrojových souborů a díky programovacímu jazyku C++ je napsán se snahou využití objektově orientovaného programování.
* `main.cpp a main.hpp` - Funkce main a funkce pro korektní ukončení programu s dealokací paměti, statická třída pro odchycení CTRL-C
* `socket.cpp a socket.hpp` - Třída socket pro uchovávání informací o socketu, jeho tvorby a záník
* `arg_parser.cpp a arg_parser.hpp` - Statická třída pro zpracování vstupních argumentů
* `messages.cpp a messages.hpp` - Abstraktní třída NetworkMessage a jednotlivé podtřídy pro TCP a UDP zprávy
* `clients.cpp a clients.hpp` - Abstraktní třída NetworkClient a jednotlivé podtřídy pro TCP a UDP klienta

### Zpracování vstupních argumentů
Na začátku programu dochází ke zpracování argumentů od uživatele pomocí statické metody `parse_args` a jejich uložení do specifické struktury, v případě nezadaných volitelných argumentů jsou dané hodnoty nastaveny na výchozí a v případě zadání chybného atributu například příliš vysokého čísla portu nebo špatně zvoleného transportního protokolu je program ukončen s chybou.

### Způsob zahájení komunikace
Po zpracování argumentů dochází dle parametru typu protokolu k vytvoření instance TCP či UDP klienta a volání odpovídající metody, která zahají hlavní logiku programu. Následně pro oba klienty platí, že dochází k vytvoření socketu a volání metody `dns_lookup`, která pro případné zadané doménové jméno najde odpovídající IP adresu, v opačném případě se program ukončí. U TCP je navíc ještě zavolána funkce `connect`, která se serverem naváže stabilní spojení (narozdíl od UDP). V neposlední řadě dojde k vytvoření struktury pro funkci poll a její naplnění file descriptory pro socket a standardní vstup.

### Příjmání a odesílání zpráv
Veškerá komunikace se děje v jediném `while loopu`, kdy podmínka kontroluje zdali je příchozí událost ze standardního vstupu či jde o příchozí zprávu ze serveru a dojde k pokračování v odpovídající větvi. Příjmání zpráv je u TCP řešeno pomocí funkce `recv` a dochází k načítání po 1 bytu, dokud není nalezen ukončovač `/r/n`, u UDP je načitání prováděno pomocí funkce `recvfrom`, jelikož po úspešném příjetí zprávy je potřeba změnit port, na který budou následující zprávy odesílány. Zpráva je narozdíl od TCP načtena naráz, jelikož zpráva do něj přijde vždy 1 (u TCP by takto mohlo v bufferu skončit zpráv více). Kvůli zmíněné změne portu pro UDP je používáná funkce `sendto` a pro TCP pouze funkce `send`.

### Kontrola syntaxe zpráv
Během příjmání a odesílání zpráv dochází simultánně ke kontrole zpráv od uživatele, které jsou kontrolovány pomocí funkce `check_user_message`, zdali se jedná o příkaz a následně zformátovány do vhodného tvaru pro odeslání v závislosti na TCP/UDP protokolu pomocí funkce `process_outgoing_message` a rovněž dochází ke kontrole zpráv od serveru (funkce `process_inbound_message`), které jsou rozpoznány a předány ke kontrole dále.

### Validace zpráv dle konečného automatu
Po úspěšné kontrole příchozí zprávy dochází k ověření, že příchozí zpráva může být v daném stavu přijata (výčet stavů automatu je definován pomocí `enum` stavů, stavy `ERR` a `BYE` nejsou přímo implementovány z důvodu jejich nepotřebnosti), v pozitivním případě dochází k jejímu výpisu na odpovídající standardní výstup, v opačném případě dochází k chybě vedoucí k poslání chybové hlášky zpět k serveru nebo rovnou ukončení programu. Rovněž odesílání zpráv může stavy FSM nastavovat (poslání `auth` zprávy udělá přechod do `AUTH` stavu).

### Blokování uživatelského vstupu
Pokud dojde k odeslání zprávy, který vyžaduje odpověď tj. `CONFIRM` a `REPLY` u UDP a `REPLY` u TCP, dojde k zablokování vstupu od uživatele (respektive poll nebude brát v potaz uživatelský vstup). Po přijatí očekáváné zprávy dojde opět k odblokování a zpracování zpráv, které mohl uživatel v mezičase zadat.

### Ztráta příchozích paketů u UDP 
Kvůli podstatě UDP komunikace zmíněné dříve je u UDP klienta nastaven na socket časový interval , který určuje dobu, do které na jakoukoliv odeslanou uživatelskou zprávu musí přijít zpráva typu `CONFIRM`. Pokud do daného intervalu zpráva nepřijde, je zvýšen čítač pokusů odeslané zprávy a zpráva je odeslána znovu. Tento proces se opakuje do té doby než je dosažen maximální limit počtu opětovně odeslaných zpráv a program je ukončen. Na potvrzovací zprávu se čeká i v případě odeslání zprávy `BYE` od klienta. Správné a včasné přijetí zprávy `CONFIRM` zajišťuje funkce `handle_timeout`. V případě, že před zprávou `CONFIRM` dorazí zpráva jiného typu, je zpracována a zvalidována odpovídajícím způsobem popsaným výše.

### Kontrola čísla příchozích paketů u UDP
U UDP komunikace může dojít k přijetí zprávy s duplicitním `MessageID`. V takovém případě je daná zpráva zahozena, a tedy ignorována. Validace probíhá tím způsobem, že u každé přijaté zprávy je její `MessageID` uloženo do vektoru `seen_ids`. U každé následující přijaté zprávy je nejprve nahlédnuto do tohoto vektoru a v případě nalezení daného identifikátoru je zpráva ignorována.

### Ukončení programu
Ukončení programu je realizováno pomocí příkazu CTRL-C, příkazu CTRL-D (tedy poslání konce souboru) nebo pokud je konec v souladu s konečným automatem ze zadání projektu, tedy např. server pošle `BYE` zprávu. V každém případě se volá funkce `exit_program`, která dle předaných parametrů rozhodne, zdali je třeba ještě před koncem poslat `BYE` zprávu (pokud ano, je zpráva poslána a v případě UDP je také očekávána zpráva `CONFIRM`), program ukončí a dealokuje paměť. 

## Testování programu
Testování probíhalo po celou dobu vývoje programu. Zahrnovalo jak kontrolu úniků paměti a původce neoprávněního přístupu do ní (pomocí funkce `valgrind`), tak nástroje díky kterým bylo možné dívat se na odeslané a přijaté zprávy klienta. Mezi testovací nástroje patřily jak nástroje od velkých společností, tak např. referenční fakultní server nebo studentské testy.

### TCP klient



### UDP klient
Detail the testing methodologies and validation procedures employed, such as:
- Unit tests and integration tests that were conducted.
- Any test frameworks used.
- Examples of tests and their outcomes.
- How these tests prove the reliability and functionality of the UDP/TCP client.

## Možná vylepšení
Chatovací klient není dokonalý a obsahuje několik věcí, které by mohly být v budocnu vylepšeny, mezi ně patří například:

* Vylepšení mechanismu timeoutů, kdy doba do vypršení timeoutu aktuálně nereflektuje skutečnou dobu, pokud před zprávou CONFIRM dorazí jiná zpráva
* Refaktorizace kódu, kdy je možné více sjednotit funkce pro hlavní logiku jak TCP tak UDP klienta, vyčlenění funkcí do dalších tříd (logika zpracovávání zpráv)
* Přídání dalších podporovaných příkazů
* Přidání časového razítka při odeslaných a přijatých zprávách
## Zdroje
- Linux manual page - poll(2). [online]. [cit. 2024-04-01]. Dostupné z: https://man7.org/linux/man-pages/man2/poll.2.html
- Transmission Control Protocol. In: *Wikipedia: the free encyclopedia*. [online]. 31. 1. 2024. [cit. 2024-04-01]. Dostupné z: https://cs.wikipedia.org/wiki/Transmission_Control_Protocol
- User Datagram Protocol. In: *Wikipedia: the free encyclopedia*. [online]. 18. 11. 2023. [cit. 2024-04-01]. Dostupné z: https://cs.wikipedia.org/wiki/User_Datagram_Protocol
- DOSTÁL R. Sockety a C/C++: funkce poll a závěr. [online].  [cit. 2024-04-01]. Dostupné z: https://www.root.cz/clanky/sokety-a-c-funkce-poll-a-zaver
- IPK Project 1: Client for a chat server using IPK24-CHAT protocol [online]. [cit. 2024-04-01]. Dostupné z: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201