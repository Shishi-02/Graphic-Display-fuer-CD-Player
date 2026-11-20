# UART-Testbefehle – CD-Player Display

**Testumgebung:** gen4-ESP32-35CT · Windows / Docklight V2.4 · USB-C (USB CDC)  
**Grundlage:** 3. Zwischenbericht vom 20.11.2026, Kapitel 3.1 und 3.2; Firmware `Display-fuer-CD-Player-USB-UART`.

## 1. Einstellungen

| Parameter | Wert |
|---|---|
| COM-Port | COM3 (falls unverändert) |
| Baudrate | 115200 |
| Datenbits / Parität / Stoppbits | 8 / None / 1 (8N1) |
| Flusskontrolle | Keine |
| Format | ASCII, Großbuchstaben, dezimale Parameter, Doppelpunkt `:` |
| Endbyte | **LF = `0A` (HEX) = 10 (dezimal) = `\n` (C/C++)** |
| CR | Nicht erlaubt (`0D`) |
| Maximale Länge | 64 Bytes einschließlich LF, also höchstens 63 Nutzdatenbytes |
| Firmware-Konfiguration | `CD_USE_USB_CDC 1`, `CD_EXTENDED_PROTOCOL 0` |

Docklight simuliert den PIC24. TX = PC → ESP32, RX = ESP32 → PC. USB-C simuliert das Nachrichtenprotokoll, nicht die elektrischen UART-Signale.

## 2. Endbyte in Docklight

**Jede einzelne Nachricht muss mit genau einem LF (`0A`) enden.** Ohne LF wird sie nicht ausgewertet. CRLF ist nicht zulässig. Die ASCII-Spalte zeigt nur die Nutzdaten; das LF ist in jeder HEX-Spalte bereits enthalten. Nicht die sichtbaren Zeichen `\n`, `<LF>` oder `0A` als ASCII-Text anhängen.

In Docklight: F6 → Send Sequence öffnen → Edit Mode **HEX** → vollständige Bytes aus der Tabelle einfügen → OK → F5 → Sequenz senden. Im HEX-Reiter kontrollieren, ob das letzte Byte `0A` ist. Andere Programme, die den COM-Port belegen, vorher schließen.

Beispiel:

| ASCII (Nutzdaten) | HEX (vollständig) | Länge |
|---|---|---|
| `STATE:INIT` | `53 54 41 54 45 3A 49 4E 49 54 0A` | 11 Bytes |

## 3. Send Sequences (TX) – PIC24 → Display

### 3.1 Zustände

| ASCII (Nutzdaten) | HEX (inkl. LF) | Bedeutung |
|---|---|---|
| `STATE:INIT` | `53 54 41 54 45 3A 49 4E 49 54 0A` | Initialisierung; Anzeige READING |
| `STATE:LOADING` | `53 54 41 54 45 3A 4C 4F 41 44 49 4E 47 0A` | CD wird eingelesen; READING |
| `STATE:NO_DISC` | `53 54 41 54 45 3A 4E 4F 5F 44 49 53 43 0A` | Keine CD; NO DISC |
| `STATE:STOP` | `53 54 41 54 45 3A 53 54 4F 50 0A` | Stopp / Disc-Übersicht |
| `STATE:PLAY` | `53 54 41 54 45 3A 50 4C 41 59 0A` | Wiedergabe |
| `STATE:PAUSE` | `53 54 41 54 45 3A 50 41 55 53 45 0A` | Pause |
| `STATE:SEARCH_FWD` | `53 54 41 54 45 3A 53 45 41 52 43 48 5F 46 57 44 0A` | Vorwärtssuche |
| `STATE:SEARCH_BWD` | `53 54 41 54 45 3A 53 45 41 52 43 48 5F 42 57 44 0A` | Rückwärtssuche |
| `STATE:ERROR` | `53 54 41 54 45 3A 45 52 52 4F 52 0A` | Laufwerksfehler |

### 3.2 Track- und Zeitinformationen

Zeitwerte werden in ganzen Sekunden übertragen (Festlegung im vorhandenen Code; im Bericht nicht ausdrücklich als Einheit definiert). Zulässiger Zeitbereich: 0–5999 s. Tracknummer vor den zugehörigen Trackzeiten senden. Gesamtzeit = vergangene Zeit + verbleibende Zeit.

| ASCII (Nutzdaten) | HEX (inkl. LF) | Bedeutung |
|---|---|---|
| `TOTAL_TRACKS:17` | `54 4F 54 41 4C 5F 54 52 41 43 4B 53 3A 31 37 0A` | 17 Titel (0–99 zulässig) |
| `TRACK:1` | `54 52 41 43 4B 3A 31 0A` | Aktueller Titel 1 (1–99; höchstens TOTAL_TRACKS, falls bekannt) |
| `TRACK:2` | `54 52 41 43 4B 3A 32 0A` | Beispiel: Titel 2 |
| `TRACK_ELAPSED:0` | `54 52 41 43 4B 5F 45 4C 41 50 53 45 44 3A 30 0A` | Vergangene Trackzeit: 00:00 |
| `TRACK_REMAINING:202` | `54 52 41 43 4B 5F 52 45 4D 41 49 4E 49 4E 47 3A 32 30 32 0A` | Restzeit: 03:22 |
| `DISC_ELAPSED:0` | `44 49 53 43 5F 45 4C 41 50 53 45 44 3A 30 0A` | Vergangene Disc-Zeit: 00:00 |
| `DISC_REMAINING:2552` | `44 49 53 43 5F 52 45 4D 41 49 4E 49 4E 47 3A 32 35 35 32 0A` | Disc-Restzeit: 42:32 |

### 3.3 Beispielwerte während der Wiedergabe

| ASCII (Nutzdaten) | HEX (inkl. LF) | Bedeutung |
|---|---|---|
| `TRACK_ELAPSED:62` | `54 52 41 43 4B 5F 45 4C 41 50 53 45 44 3A 36 32 0A` | Vergangene Trackzeit: 01:02 |
| `TRACK_REMAINING:140` | `54 52 41 43 4B 5F 52 45 4D 41 49 4E 49 4E 47 3A 31 34 30 0A` | Restzeit: 02:20 |
| `DISC_ELAPSED:62` | `44 49 53 43 5F 45 4C 41 50 53 45 44 3A 36 32 0A` | Vergangene Disc-Zeit: 01:02 |
| `DISC_REMAINING:2490` | `44 49 53 43 5F 52 45 4D 41 49 4E 49 4E 47 3A 32 34 39 30 0A` | Disc-Restzeit: 41:30 |
| `TRACK_ELAPSED:63` | `54 52 41 43 4B 5F 45 4C 41 50 53 45 44 3A 36 33 0A` | Zeitupdate nach einer Sekunde |
| `TRACK_REMAINING:139` | `54 52 41 43 4B 5F 52 45 4D 41 49 4E 49 4E 47 3A 31 33 39 0A` | Zeitupdate nach einer Sekunde |
| `DISC_ELAPSED:63` | `44 49 53 43 5F 45 4C 41 50 53 45 44 3A 36 33 0A` | Zeitupdate nach einer Sekunde |
| `DISC_REMAINING:2489` | `44 49 53 43 5F 52 45 4D 41 49 4E 49 4E 47 3A 32 34 38 39 0A` | Zeitupdate nach einer Sekunde |

## 4. Empfohlener Testablauf

### Test 1 – Initialisierung

Die ersten beiden Zeilen nacheinander senden: `STATE:INIT`, `STATE:LOADING`. Soll: READING.

### Test 2 – CD eingelesen / Inhaltsübersicht

Die folgenden Zeilen einzeln senden (die HEX-Werte stehen in Abschnitt 3):

```text
STATE:STOP
TOTAL_TRACKS:17
TRACK:1
TRACK_ELAPSED:0
TRACK_REMAINING:202
DISC_ELAPSED:0
DISC_REMAINING:2552
```

Soll: 17 Tracks, Gesamtspielzeit 42:32. Track 1 hat eine Dauer von 03:22.

### Test 3 – Play und Zeitangaben

Play auf dem Display berühren: Docklight empfängt `PLAY` + LF. Erst nach der Zustandsmeldung des simulierten PIC24 wechselt die Anzeige. Folgende Zeilen ohne längere Pause senden:

```text
STATE:PLAY
TRACK_ELAPSED:62
TRACK_REMAINING:140
DISC_ELAPSED:62
DISC_REMAINING:2490
```

Soll: Track 01, Zeit 01:02 und Fortschrittsanzeige. Danach die Zeitwerte im Sekundentakt aktualisieren:

```text
TRACK_ELAPSED:63
TRACK_REMAINING:139
DISC_ELAPSED:63
DISC_REMAINING:2489
```

Soll: 01:03. Bei weiterer Wiedergabe vergangene Zeiten jeweils erhöhen und verbleibende Zeiten jeweils verringern.

### Test 4 – Zyklische Nachrichten / Timeout

In Docklight eine Send Sequence mit `TRACK_ELAPSED:62` + LF erstellen. Unter **Additional Settings → Repeat** die Option **Send periodically** aktivieren und **1 second** einstellen. Nach `STATE:PLAY` die periodische Sequenz starten. Voraussetzung: gültige Tracknummer wurde bereits übertragen.

Für einen realistischeren Test alle vier Zeittelegramme (jedes mit LF) in einer gemeinsamen Send Sequence hintereinander übertragen und jede Sekunde wiederholen. Vollständiger HEX-Block für konstante Beispielwerte:

```text
54 52 41 43 4B 5F 45 4C 41 50 53 45 44 3A 36 32 0A 54 52 41 43 4B 5F 52 45 4D 41 49 4E 49 4E 47 3A 31 34 30 0A 44 49 53 43 5F 45 4C 41 50 53 45 44 3A 36 32 0A 44 49 53 43 5F 52 45 4D 41 49 4E 49 4E 47 3A 32 34 39 30 0A
```

Damit bleibt die Kommunikation aktiv, die Zeit steht jedoch bei 01:02. Für hochzählende Zeitwerte sind veränderliche Sequenzen oder ein Skript notwendig. Periodisches Senden vor Pause/Stop/Trackwechsel beenden oder anpassen.

**Timeout-Test:** Während PLAY die zyklischen Nachrichten stoppen. Nach ca. 6 s ohne gültige Nachricht sollte VERBINDUNG UNTERBROCHEN erscheinen. Dies ist die Software-Verbindungsüberwachung, nicht zwingend ein USB-Abbruch. Zur Wiederherstellung alle Status- und Mediendaten aus Test 1–3 neu übertragen (INIT/LOADING setzen die Mediendaten zurück). In STOP/PAUSE ist der Timeout im Standardmodus nicht aktiv.

### Test 5 – Bedienung und weitere Zustände

- Pause / Stop: Taste antippen, RX-Befehl prüfen und mit `STATE:PAUSE` bzw. `STATE:STOP` antworten.
- Nächster Titel: Weiter antippen, RX `NEXT` prüfen, dann `TRACK:2` und die neuen Trackzeitwerte senden (z. B. `TRACK_ELAPSED:0` und `TRACK_REMAINING:202`).
- Suchlauf: Weiter/Zurück halten; RX prüfen, `STATE:SEARCH_FWD` bzw. `STATE:SEARCH_BWD` senden und Zeitwerte weiter übertragen. Nach Loslassen RX `SEARCH_STOP` prüfen und passenden Zustand zurückmelden, z. B. `STATE:PLAY`.
- Auswurf: RX `EJECT` prüfen, `STATE:NO_DISC` senden. Soll: NO DISC; Mediendaten werden gelöscht.
- Fehler: `STATE:ERROR` senden. Soll: LAUFWERKSFEHLER.

## 5. Receive Sequences (RX) – Display → PIC24

Diese Befehle werden durch die Touch-Bedienung ausgelöst. Sie werden in Docklight **empfangen**, nicht als Steuerzustände an das Display gesendet. Alle enthalten ebenfalls das Endbyte LF.

| ASCII (Nutzdaten) | HEX (inkl. LF) | Bedeutung |
|---|---|---|
| `PLAY` | `50 4C 41 59 0A` | Play antippen |
| `PAUSE` | `50 41 55 53 45 0A` | Pause antippen |
| `STOP` | `53 54 4F 50 0A` | Stop antippen |
| `NEXT` | `4E 45 58 54 0A` | Weiter kurz drücken / loslassen |
| `PREVIOUS` | `50 52 45 56 49 4F 55 53 0A` | Zurück kurz drücken / loslassen |
| `SEARCH_FWD_START` | `53 45 41 52 43 48 5F 46 57 44 5F 53 54 41 52 54 0A` | Weiter mindestens 500 ms halten (Play/Pause) |
| `SEARCH_BWD_START` | `53 45 41 52 43 48 5F 42 57 44 5F 53 54 41 52 54 0A` | Zurück mindestens 500 ms halten (Play/Pause) |
| `SEARCH_STOP` | `53 45 41 52 43 48 5F 53 54 4F 50 0A` | Suchlauftaste loslassen |
| `REPEAT` | `52 45 50 45 41 54 0A` | Repeat kurz drücken |
| `SHUFFLE` | `53 48 55 46 46 4C 45 0A` | Repeat mindestens 500 ms halten |
| `EJECT` | `45 4A 45 43 54 0A` | Auswurf antippen |
| `TRACK:5` | `54 52 41 43 4B 3A 35 0A` | Direktauswahl über Software-API; kein numerisches Touchfeld vorhanden |

## 6. Hinweise zum Protokoll

- Im Standardmodus (`CD_EXTENDED_PROTOCOL 0`) ist kein ACK erforderlich. Nach Touchbefehlen den tatsächlich eingetretenen Zustand mit `STATE:...` melden.
- `GET_STATUS`, `ACK` und `STATUS_END` gehören zum optionalen erweiterten Altprotokoll; für diesen Test sind sie nicht nötig. ROM-Bootmeldungen des ESP32 werden nicht bestätigt.
- Die Firmware akzeptiert zusätzlich ältere `MODE:`-Meldungen; deren genaue Semantik ist im neuen Bericht noch nicht festgelegt. Für einen separaten Repeat-Anzeigetest nach Abstimmung:

| ASCII (Nutzdaten) | HEX (inkl. LF) | Bedeutung |
|---|---|---|
| `MODE:NORMAL` | `4D 4F 44 45 3A 4E 4F 52 4D 41 4C 0A` | Repeat aus |
| `MODE:REPEAT_TRACK` | `4D 4F 44 45 3A 52 45 50 45 41 54 5F 54 52 41 43 4B 0A` | Track wiederholen |
| `MODE:REPEAT_DISC` | `4D 4F 44 45 3A 52 45 50 45 41 54 5F 44 49 53 43 0A` | Disc wiederholen |
| `MODE:SHUFFLE` | `4D 4F 44 45 3A 53 48 55 46 46 4C 45 0A` | Shuffle; vorhandene GUI hat keine separate Shuffle-Markierung |

**Prüfstatus:** ASCII/HEX-Zuordnung automatisch geprüft, anhand des Quellcodes und der Testvorgaben erstellt; diese Datei ist eine Testanleitung und kein Nachweis der Hardware-Funktionsprüfung.
