# Display für CD-Player

Die UART-Anbindung folgt Abschnitt 2 des Steuerungsberichts vom 06.11.2026.

## Start und Anschluss

PlatformIO-Projekt öffnen, bauen und auf das gen4-ESP32-35CT laden. Nach Start erscheint „WARTE AUF STEUERUNG“, bis der PIC antwortet. Der frühere Automatikdemo-Modus ist entfernt.

UART0 verwendet 115200 Baud, 8N1, ohne Flusskontrolle. RX ist GPIO44 / FFC 23 und wird mit PIC-U1TX (Pin 33) verbunden. TX ist GPIO43 / FFC 24 und geht an PIC-U1RX (Pin 34). Der Bericht nennt gemeinsame Masse und 3,3-V-Signalpegel.

Jede ASCII-Zeile hat höchstens 64 Byte einschließlich LF. CR und Prüfsummen sind im Bericht nicht vorgesehen.

## Bestehende Displayfunktionen

| Taste | Gesendete Zeile vor LF |
|---|---|
| Zurück | PREVIOUS |
| Pause | PAUSE |
| Play | PLAY |
| Stopp | STOP |
| Weiter | NEXT |
| Auswurf | EJECT |
| Loop | REPEAT |

Die Firmware sendet nach dem Start GET_STATUS und nach beendeter Antwort regelmäßig weitere Statusabfragen (Abstand mindestens zwei Sekunden). So werden auch Pause, STOP und Änderungen durch die Fernbedienung synchronisiert.

Ein Befehl desselben Typs bleibt bis zu seinem ACK gesperrt. ACK allein verändert keine angezeigten Wiedergabedaten. ERR wird als Fehler angezeigt. Da der Bericht ERR keinem bestimmten Befehl zuordnet, bleiben offene Befehle bis zu ACK oder STATE:INIT gesperrt. Nach drei Sekunden ohne ACK erscheint „BESTAETIGUNG FEHLT“. Offene Befehle werden nicht automatisch wiederholt. Nach sechs Sekunden ohne verwertbare Meldung erscheint „VERBINDUNG UNTERBROCHEN“. Diese Zeitgrenzen sind Festlegungen der Displayseite.

## Daten aus der Steuerung

- TOTAL_TRACKS liefert die Trackanzahl.
- TRACK liefert die aktuelle Nummer, auf UART beginnend bei 1.
- TRACK_ELAPSED liefert die vergangene Trackzeit in Sekunden.
- TRACK_ELAPSED + TRACK_REMAINING ergeben die Trackdauer für Auswahl und Fortschrittsbalken.
- DISC_ELAPSED + DISC_REMAINING ergeben TOTAL.
- INIT/LOADING zeigen READING. STOP zeigt die Übersicht. PLAY und Suchlauf zeigen die gemeldete Spielzeit. PAUSE hält die empfangene Zeit. NO_DISC zeigt „NO DISC“, ERROR zeigt einen Fehler.
- Ein bestätigter Titelwechsel bei STOP zeigt die Track-Auswahl. Periodische STOP-Statusabfragen behalten diese Ansicht.
- MODE:REPEAT_TRACK markiert Loop mit „1“. MODE:REPEAT_DISC markiert es mit „CD“. Die übrigen dokumentierten Modi heben die Markierung auf.

Für fehlende Zahlen erscheinen Striche. Der Fortschrittspunkt erscheint erst, wenn Spielzeit und positive Trackdauer vorliegen. Die Darstellung ist auf 99 Tracks und 99:59 begrenzt. Größere Zeitwerte werden verworfen.

Ein Status-Schnappschuss wird bis STATUS_END gesammelt. Die Feldreihenfolge ist dabei frei. Asynchrone Track-Zeitmeldungen gehören zum zuletzt gemeldeten TRACK; die Steuerung muss bei einem Titelwechsel TRACK vor den neuen Track-Zeiten senden. Für die Summenbildung werden je zwei neu empfangene Zeitfelder kombiniert. Das Protokoll enthält keine Sequenz- oder Medienkennung zur Erkennung verlorener bzw. vermischter Zeitpaare.

Die Darstellung verwendet weiterhin die vergangene Trackzeit. TIME_MODE, AB_STATE und VOLUME erhalten keine zusätzlichen Bedienelemente. Ihre Nachrichten können empfangen werden und werden für diese Oberfläche ignoriert.

## Tischtest

An einem 3,3-V-UART-Adapter LF als Zeilenende wählen. Auf GET_STATUS beispielsweise folgende Antwort senden, jede Zeile mit LF:

    ACK:GET_STATUS
    STATE:STOP
    TRACK:1
    TOTAL_TRACKS:17
    TRACK_ELAPSED:0
    TRACK_REMAINING:202
    DISC_ELAPSED:0
    DISC_REMAINING:2552
    MODE:NORMAL
    ERROR_CODE:0
    STATUS_END

Erwartet werden 17 Tracks und 42:32. Die Anfrage regelmäßig beantworten, damit die Verbindung aktuell bleibt.

Nach einem Play-Tipp muss PLAY empfangen werden. ACK:PLAY quittiert den Befehl. Erst STATE:PLAY schaltet die Anzeige auf Wiedergabe. Danach beispielsweise TRACK_ELAPSED:62 und TRACK_REMAINING:140 senden. Erwartet werden 01:02 und etwa 31 Prozent Fortschritt.

Beim Loop-Tipp muss REPEAT empfangen werden. Mit ACK:REPEAT und MODE:REPEAT_TRACK die Titelwiederholung bestätigen. Ein Wechsel per Fernbedienung auf MODE:REPEAT_DISC muss „CD“ am Loop-Symbol anzeigen.

## Mit der Steuerungsgruppe abzugleichen

Der Bericht beschreibt einen geplanten Vertrag, dessen PIC-Firmware noch nicht vollständig umgesetzt ist.

REPEAT ist ein Moduswechsel. Der Vertrag beschreibt weder die Reihenfolge des Zyklus noch einen direkten Befehl zum Setzen oder Abschalten ausschließlich der Titelwiederholung. Jeder Display-Tipp sendet deshalb genau ein REPEAT und wartet auf MODE. Für die bisher gewünschte direkte Ein/Aus-Umschaltung von Repeat Track benötigt die Steuerungsseite eine präzisere Vereinbarung.

EJECT ist im Bericht technisch offen. Ein bestätigter Auswurfzustand ist nicht spezifiziert; das Display sendet EJECT und zeigt den tatsächlich gemeldeten Zustand, insbesondere NO_DISC.

Der Gesamtstatus muss die beiden CD-Zeitfelder auch im STOP-Zustand bereitstellen, damit TOTAL verfügbar ist. Die ausgewählte Trackdauer benötigt beide Track-Zeitfelder auch vor PLAY. Nicht vorhandene Werte werden mit Strichen dargestellt.
