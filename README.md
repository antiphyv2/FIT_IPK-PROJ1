# Dokumentace IPK-24 TCP/UDP Chatovacího klienta 


**Autor:** Samuel Hejníček  
**Datum:** 1. dubna 2024  


## Obsah
- [Dokumentace IPK-24 TCP/UDP Chatovacího klienta](#dokumentace-ipk-24-tcpudp-chatovacího-klienta)
  - [Obsah](#obsah)
  - [Úvod](#úvod)
    - [Stručný popis](#stručný-popis)
    - [Spuštění a ovládání programu](#spuštění-a-ovládání-programu)
  - [Základní teorie k pochopení programu](#základní-teorie-k-pochopení-programu)
    - [TCP Protokol](#tcp-protokol)
    - [UDP Protokol](#udp-protokol)
    - [Funkce poll](#funkce-poll)
  - [Implementace projektu](#implementace-projektu)
    - [Návrh](#návrh)
    - [Zpracování vstupních argumentů](#zpracování-vstupních-argumentů)
    - [Připojení ke klientovi](#připojení-ke-klientovi)
    - [Příjmání zpráv](#příjmání-zpráv)
    - [Odesílání zpráv](#odesílání-zpráv)
    - [Ztráta příchozích paketů u UDP](#ztráta-příchozích-paketů-u-udp)
    - [Kontrola čísla příchozích paketů u UDP](#kontrola-čísla-příchozích-paketů-u-udp)
  - [Testování programu](#testování-programu)
    - [TCP klient](#tcp-klient)
    - [UDP klient](#udp-klient)
  - [Funkce nad rámec zadání](#funkce-nad-rámec-zadání)
  - [Zdroje](#zdroje)


## Úvod

Tento soubor je dokumentací k prvnímu projektu do předmětu [IPK], Počítačové komunikace a sítě, veškeré zdrojové soubory jsou dostupné [zde](https://git.fit.vutbr.cz/xhejni00/IPK_Project_1).

### Stručný popis

Program slouží jako chatovací klient schopný komunikace se vzdáleným serverem pomocí protokolu IPK24-CHAT, který zajíšťuje definovaný formát zpráv posílaných mezi sebou.Tento protokol je navíc postaven nad transportní protokoly TCP/UDP, přičemž jeden z nich klient k připojení na vzdálený server používá. Se serverem klient komunikuje pomocí několika definovaných zpráv viz [Spuštění a ovládání programu](#spuštění-a-ovládání-programu). V případě, že chce klient ukončit ukončit komunikaci se serverem, může tak učinit pomocí CTRL-C, které server informuje o konci spojení a ukončí program.

### Spuštění a ovládání programu

Offer a detailed narrative of the project, including:
- The architecture of the UDP/TCP client.
- How it establishes connections and manages data transmission.
- Any specific libraries or C++ features used extensively in the project.

## Základní teorie k pochopení programu

Highlight interesting or complex sections of the source code, explaining:
- The problem they solve.
- How they work and why they were implemented in this particular way.
- Any challenges faced and how they were overcome.

### TCP Protokol
### UDP Protokol
### Funkce poll

## Implementace projektu

### Návrh
### Zpracování vstupních argumentů
### Připojení ke klientovi
### Příjmání zpráv
### Odesílání zpráv
### Ztráta příchozích paketů u UDP 
### Kontrola čísla příchozích paketů u UDP

## Testování programu

### TCP klient
### UDP klient
Detail the testing methodologies and validation procedures employed, such as:
- Unit tests and integration tests that were conducted.
- Any test frameworks used.
- Examples of tests and their outcomes.
- How these tests prove the reliability and functionality of the UDP/TCP client.

## Funkce nad rámec zadání

Describe any additional functionality that extends beyond the basic requirements, including:
- Advanced error handling and recovery mechanisms.
- Performance optimization techniques.
- Any user-friendly features or enhancements.

## Zdroje

List all the sources used during the development of the project. This could include:
- Books, articles, and papers on networking and protocol theory.
- Documentation for any libraries or frameworks used.
- Websites, forums, and discussion threads that provided useful insights or solutions.

Remember, a good bibliography not only credits sources but also helps others find resources for learning and problem-solving.
