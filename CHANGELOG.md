# CHANGELOG UDP/TCP chatovacího klienta IPK24-CHAT

Všechny změny jsou k nahlédnutí v oficiálním repozitáři zde:  https://git.fit.vutbr.cz/xhejni00/IPK_Project_1.


Jednotlivé commity v aktivní fázi vývoje projektu (od 25. 3. 2024) byly v souladu s Conventional commits viz: https://www.conventionalcommits.org/en/v1.0.0/

## Významné změny programu dle data

### 25. 3. 2024
- Přidán TCP klient

### 27. 3. 2024
- Vylepšení zpracování zpráv a argumentů od uživatele
  
### 31. 3. 2024
- Přidán UDP klient

### 1. 4. 2024
- Přidána podpora timeoutů pro UDP klienta

## Známá omezení / Návrhy k vylepšení
- Program neumí korektně pracovat se správnou hodnotou timeout časovače pokud přijde jiná zpráva před zprávou CONFIRM
- Při lokálním testování občas dochází k neuklizeným blokům paměti (still reachable), chyby ani pády se neobjevily
- Vhodná refaktorizace části kódu

