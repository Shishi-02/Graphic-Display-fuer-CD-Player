# CD-Player Display – USB-C-Test / Hardware-UART

PlatformIO-Projekt fuer 4D Systems gen4-ESP32-35CT. Das vorhandene grafische Design
mit Rollenziffern, Fortschrittsanzeige und sieben Bedienelementen ist erhalten.

## 1. Umschalten

In `src/include/CommunicationConfig.h`:

```cpp
#define CD_USE_USB_CDC 1
#define CD_UART_PORT Serial0
#define CD_UART_RX_PIN 44
#define CD_UART_TX_PIN 43
#define CD_BAUD 115200
#define CD_EXTENDED_PROTOCOL 0
```

`1` = USB-C (native USB CDC, `Serial`), `0` = Hardware-UART.
USB-C verwendet die fest verdrahteten USB-Datenleitungen und keine UART-GPIOs.
Beim Umschalten auf 0 erneut bauen und flashen. Original-Pinbelegung:
FFC 23 / GPIO44 = RX, FFC 24 / GPIO43 = TX, gemeinsame Masse, 3,3-V-Logik.
PIC-TX an ESP-RX und umgekehrt; PIC-Pinbelegung vor Anschluss pruefen.
Alternativ kann `CD_UART_PORT` auf `Serial1` mit passenden freien GPIOs gesetzt
werden. Die UART arbeitet mit 115200, 8N1, ohne Flusskontrolle.

Das vorhandene PlatformIO-Boardprofil aktiviert native USB-CDC. Bei einer
abweichenden Boardkonfiguration muss `ARDUINO_USB_CDC_ON_BOOT=1` gelten.
USB-C dient nur zur Simulation des Nachrichtenprotokolls, nicht der elektrischen
UART-Signale oder der FFC-Leitung.

## 2. Kommunikation nach Zwischenbericht 20.11.2026 (Kap. 3.1/3.2)

ASCII, Grossbuchstaben, Doppelpunkt, Dezimalzahlen, exakt ein LF (0x0A),
kein CR, maximal 64 Bytes inklusive LF. Ungueltige/zu lange Zeilen werden bis
zum LF verworfen. Der RX-Parser verarbeitet max. 256 Bytes je Schleifendurchlauf.
Eine TX-Queue (8 Nachrichten) und separate Task trennen potenziell blockierende
Schreibzugriffe von der GUI; keine `if (Serial)` / `while (!Serial)` /
`readStringUntil()`. USB-TX-Timeout 20 ms ist kein garantiertes Ende-zu-Ende-Limit.
Bei voller Queue oder von write() gemeldetem Fehler erscheint ein Hinweis.
Vor dem Schreiben werden ueber 250 ms alte Queue-Eintraege verworfen.
Keine automatische Wiederholung, keine Liefergarantie. Ein abgebrochener
Suchlauf-Befehl muss die PIC-Seite sicher behandeln (z.B. eigener Timeout,
noch abzustimmen).

Der PIC24 bleibt Quelle des Zustands: Bedieneingaben aendern den Zustand nicht
optimistisch; das Display wartet auf empfangene Statusmeldungen. Standardmaessig
sendet das Display keine GET_STATUS-Abfragen und verlangt kein ACK, da beide
in der neuen Tabelle 2 nicht explizit beschrieben sind. Der PC/PIC muss beim
Start einen initialen Zustand senden. In STOP/PAUSE gibt es keinen
Verbindungs-Timeout, da dort keine periodische Nachricht spezifiziert ist.
Bei PLAY/Suchlauf wird nach 6 s ohne gueltige Meldung OFFLINE angezeigt
(lokale Festlegung); danach erneut einen vollstaendigen Zustand senden.

Der alte, aus dem Originalprojekt uebernommene GET_STATUS/ACK/STATUS_END-Modus
kann mit `CD_EXTENDED_PROTOCOL=1` separat aktiviert werden. Nur nach
Abstimmung verwenden; die alte Beschreibung steht in `README_Protokoll_Bisher.md`.
MODE:REPEAT_TRACK/DISC, weitere alte MODE-Werte, ACK/ERR und ERROR_CODE
werden weiterhin erkannt, sind aber im neuen Bericht nicht im Detail definiert.

## 3. Touch und Nachrichten

| Bedienung | Display -> PIC24 (jeweils + LF) |
|---|---|
| Play | PLAY |
| Pause | PAUSE |
| Stop | STOP |
| Weiter, kurz (Loslassen) | NEXT |
| Zurueck, kurz (Loslassen) | PREVIOUS |
| Weiter halten (ab 500 ms, bei Play/Pause) | SEARCH_FWD_START |
| Zurueck halten (ab 500 ms, bei Play/Pause) | SEARCH_BWD_START |
| Nach Suchlauf loslassen | SEARCH_STOP |
| Loop kurz | REPEAT |
| Loop halten (ab 500 ms) | SHUFFLE |
| Auswurf | EJECT |
| API `Communication::selectTrack(5, millis())` | TRACK:5 |

Langdruck und Shuffle-Belegung sind lokale Vorschlaege, nicht im PDF
festgelegte Touchgesten. Eine numerische Track-Auswahl besitzt das vorhandene
Design nicht; der Befehl TRACK:<nr> ist als API implementiert, aber noch nicht
an ein direktes Touch-Eingabefeld gebunden. Suchlauf-Zustaende werden wie im
Originalprojekt im Wiedergabebild dargestellt. Ohne MODE-Rueckmeldung wird
keine neue Repeat-Markierung angenommen.

## 4. PC-Test (HTerm, Docklight oder Arduino Serial Monitor)

- Display per USB-C anschliessen, richtigen COM-Port waehlen.
- 115200 / 8N1 / keine Flusskontrolle, Zeilenende LF (nicht CRLF).
- Andere Programme schliessen, die den COM-Port belegen.
- Nach dem Flashen im Standardmodus folgende Zeilen in dieser Reihenfolge
  senden (jede Zeile einzeln mit genau einem LF):

```
STATE:INIT
STATE:LOADING
STATE:STOP
TOTAL_TRACKS:17
TRACK:1
TRACK_ELAPSED:0
TRACK_REMAINING:202
DISC_ELAPSED:0
DISC_REMAINING:2552
```

Erwartete Darstellung: 17 Tracks, Gesamtdauer 42:32.
Play antippen: PC empfaengt `PLAY` + LF. Die Anzeige aendert sich erst bei:

```
STATE:PLAY
TRACK_ELAPSED:62
TRACK_REMAINING:140
DISC_ELAPSED:62
DISC_REMAINING:2490
```

Danach 1 Hz neue Zeitwerte senden, z.B. 63/139 und 63/2489. Erwartet werden
01:02, dann 01:03; Fortschrittsanzeige. `STATE:PAUSE` stoppt die Anzeige
auf dem zuletzt vom PIC gemeldeten Wert; `STATE:NO_DISC` leert Mediendaten.
Bei Trackwechsel `TRACK:<nr>` vor seinen Zeitwerten senden.

## 5. Annahmen und offene Punkte

Das PDF legt dezimale Zahlen fest, aber keine eindeutige Zeiteinheit,
Wertebereiche oder exakte MODE-/Quittungssemantik. Aus dem Originalprojekt
uebernommen: Sekunden, TRACK 1..99, TOTAL_TRACKS 0..99, Zeiten bis 5999 s.
Track- und Disc-Dauer werden aus je zwei neu empfangenen Zeitwerten gebildet.
Alle vier Zeitwerte werden gespeichert; vorhandene GUI zeigt Track-Zeit bzw.
Disc-Gesamtdauer. Werte ausserhalb der Anzeigekapazitaet werden nicht angezeigt.
Keine Sequenz-ID oder Zeitpaar-ID vorhanden; Reihenfolge mit Steuerungsgruppe
abstimmen. Bei STOP muss der PIC beide Disc-Zeitwerte und bei Track-Auswahl
beide Track-Zeitwerte liefern, sonst erscheinen Striche. Repeat/Shuffle-Status,
Suchlauf-Sicherheitsverhalten und Initialstatus beim Verbindungsaufbau abgleichen.

Host-Test mit Arduino/FreeRTOS-Mocks ersetzt keinen ESP32-Build, USB-/UART-
Praxistest, Touch-Test oder FFC-Signalqualitaetsnachweis. Alte .pio-Buildartefakte
wurden nicht mitgeliefert, damit keine veraltete Firmware geflasht wird.
