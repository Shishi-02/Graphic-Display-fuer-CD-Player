# Anforderungsliste – 3,5-Zoll-TFT-Display für einen CD-Player

**Projektpartner:** TBD (AB-Tron ist nur in der Vorlage genannt; Übernahme bestätigen)  
**Ansprechpartner:** TBD (Fabian Büttner ist nur in der Vorlage genannt; Übernahme bestätigen)  
**Plattform:** 4D Systems gen4-ESP32-35CT, ESP32-S3R8, Steuerungs-CPU des CD-Players (Typ TBD)  
**Version:** 1.0 – Technisch geprüfter Anforderungsentwurf, noch nicht freigegeben  
**Stand:** 22.10.2026  
**Grundlagen:** Hochgeladenes gen4-ESP32 Series Datasheet, Revision 1.0 vom 12.09.2023, 43 Seiten [D1]; hochgeladene Vorlage `anforderungsliste(1).md` [V1]; Nutzeranforderungen [A].

## Legende

- **F** = Festanforderung; bei Freigabe verbindlicher Bestandteil des Projekts.
- **W** = Wunschanforderung; optional, Umsetzung nach Freigabe.
- **[D1]** = Wert oder Eigenschaft aus dem hochgeladenen Datenblatt; Seitenangaben beziehen sich ausschließlich auf dessen Revision 1.0.
- **[A]** = Vorgabe des Auftraggebers.
- **[Z]** = Vorläufiger technischer Zielwert oder vorgeschlagene Ausführung; abzustimmen und zu verifizieren.
- **[R]** = Technische Randbedingung beziehungsweise daraus abgeleitete Integrationsanforderung; kein zugesicherter Datenblattwert.
- **TBD** = Noch festzulegen oder nachzuweisen; keine implizite Zusage.
- **[E1] / [E2]** = Ergänzende Primärquellen von Espressif beziehungsweise Texas Instruments; siehe Quellen.

**Vermerk zu vorläufigen Werten:** F/W beschreibt die Verbindlichkeit der Funktion, nicht die Belegqualität eines Zahlenwerts. Auch ein mit F gekennzeichneter Punkt kann einen noch abzustimmenden Zielwert enthalten. Datenblatt-Typwerte sind keine garantierten Maximalwerte. Die Spalte Datum bezeichnet wie in der Vorlage den jeweiligen Termin, nicht das Erstellungsdatum. Termine und persönliche Zuständigkeiten bleiben bis zur Projektabstimmung TBD. Es wurden keine physischen Messungen am Modul oder CD-Player durchgeführt.

| Nr. | F/W | Anforderung | Wert | Bemerkung | Datum | Verantw. |
| --- | :---: | --- | --- | --- | --- | --- |
| **1** | | **Bestandsaufnahme und Inbetriebnahme** | | | | |
| 1.1 | F | Displayvariante eindeutig festlegen | [A] gen4-ESP32-35CT | Kapazitiver Touch, ohne CLB; nicht gen4-ESP32-35 ohne Touch und nicht 35CT-CLB. Zusatz „-ND“ ist kein in [D1] definiertes Variantenmerkmal; Lieferantenartikel der Herstellerteilenummer zuordnen. | TBD | TBD |
| 1.2 | F | Hardware- und Dokumentenstand abgleichen | [D1] Dokument Rev. 1.0; gelieferte Hardware: TBD | Typenschild, Platinenrevision, Stückliste und aktuellen Herstellerstand vor Schaltplanfreigabe vergleichen; Widersprüche siehe technische Randbedingungen. | TBD | TBD |
| 1.3 | F | CD-Player-Schaltung und Steuerungs-CPU erfassen | [A] Versorgungsschienen +5, +9, +15 und −15 V; CPU-Typ TBD | Schaltplan, Massebezüge, Toleranzen, verfügbare Ströme, UART-Pegel und Ausschaltsequenz feststellen. | TBD | TBD |
| 1.4 | F | Datenquellen identifizieren | [A] Subchannel → Steuerungs-CPU → ESP32 | Laufwerks-/Decoder-Typ, Subcode-Schnittstelle, TOC-Zugriff und gültige Zustandsmeldungen dokumentieren. Kein direkter Subchannel-Anschluss am Display vorausgesetzt. | TBD | TBD |
| 1.5 | F | Erstinbetriebnahme kontrolliert durchführen | [Z] Strombegrenzte 5-V-Versorgung; Begrenzung nach Lastbudget | Vor Anschluss Pin-1-Lage, Polung, Kurzschlussfreiheit und Masseverbindung prüfen; Boot, Bild, Touch und UART protokollieren. | TBD | TBD |
| 1.6 | F | Fehlende Projektdaten erfassen | TBD | TH-AB-Logo, Einbauraum, CPU-Protokoll, Lautstärkerückmeldung und Abschaltsteuerung beschaffen. | TBD | TBD |
| **2** | | **Elektronik und Spannungsversorgung** | | | | |
| 2.1 | F | Display ausschließlich aus +5 V versorgen | [A] Nominal +5 V DC | +9 V, +15 V und −15 V dürfen weder Versorgung noch GPIO des Displaymoduls erreichen. Bezug ist System-GND, nicht −15 V. | TBD | TBD |
| 2.2 | F | Spannung direkt am Modul einhalten | [Z] 4,75–5,25 V einschließlich Leitungsverlusten und Ripple | [D1] S. 9/37: 4,0–6,0 V, nominal 5,0 V; S. 15: 4,0–5,5 V. Zielbereich liegt innerhalb beider Angaben; Abweichung vor Freigabe klären. | TBD | TBD |
| 2.3 | F | Grenzwerte von Betriebswerten trennen | [D1] VCC absolut: −0,3 bis +6,5 V | S. 37: Absolute-Maximum-Werte sind keine zulässigen Dauerbetriebs- oder Regelziele. Überschwingen beim Ein-/Ausschalten berücksichtigen. | TBD | TBD |
| 2.4 | F | Vorhandene 5-V-Schiene auf Eignung prüfen | TBD: Stromreserve, Ripple, Lastsprünge und Einschaltverhalten | Falls ungeeignet, separate geregelte +5-V-Versorgung vorsehen. Wandler-Eingang und Verlustleistung passend zur gewählten positiven Schiene auslegen. | TBD | TBD |
| 2.5 | F | Versorgungs- und Massekontakte parallel nutzen | [Z] Alle vier 5-V-Pins 26–29 und GND 1/21/25/30 anschließen | [D1] S. 9/15 empfiehlt mindestens je zwei, idealerweise je vier Kontakte. FFC, Stecker und Leiterbahnen nach tatsächlichem Strom dimensionieren. | TBD | TBD |
| 2.6 | F | Typische Stromaufnahme berücksichtigen | [D1] 184 mA bei „Contrast = 15“ | S. 38: nur einfache Bildoperationen, ohne WLAN und ohne eingesteckte microSD. Kein spezifizierter Maximalstrom. | TBD | TBD |
| 2.7 | F | Leistungs- und Strombudget bestimmen | [R] Aus 5 V × 184 mA folgt ca. 0,92 W; [Z] zunächst 5 V / 1 A Quellkapazität reservieren | 0,92 W ist ein abgeleiteter Richtwert unter den genannten Bedingungen. 1 A ist eine Planungsreserve, kein Verbrauchswert oder Nachweis für das Gesamtgerät. Spitzen und Dauerlast messen. | TBD | TBD |
| 2.8 | F | 3,3-V-Anschluss korrekt behandeln | [D1] FFC-Pin 20 ist 3,3-V-Ausgang | Nicht als 5-V-Eingang oder externe Einspeisung verwenden. Datenblatt nennt 100–200 mA nur als unverbindliche Einschätzung; externe Last zunächst 0 mA [Z], Erweiterungen separat budgetieren. | TBD | TBD |
| 2.9 | F | Stromaufnahme bei maximaler Projektlast prüfen | TBD: Spitzenstrom, Dauerstrom, Einschaltstrom | Volle Helligkeit, Animation, Touch, Kommunikation und ggf. microSD/S/PDIF gleichzeitig; Stabilität und Erwärmung dokumentieren. | TBD | TBD |
| 2.10 | F | Schutz und Entkopplung dimensionieren | [R] Verpolung, Überstrom, Überspannung und lokale Pufferung | Vorhandene Modulbeschaltung [D1] S. 36 beachten; zusätzliche Werte nach Messung, Kabel und Netzteil festlegen. Modul-Verpolschutz ersetzt keinen Schutz gegen +15 V. | TBD | TBD |
| 2.11 | F | USB- und Systemversorgung koordinieren | [R] Kein unkontrolliertes Rückspeisen; Ausschaltzustand eindeutig | [D1] S. 23/36: USB-C versorgt das Modul ebenfalls. Diodenpfade und reales Board prüfen; USB darf die zugesicherte Abschaltung nicht unbemerkt verhindern. | TBD | TBD |
| 2.12 | F | Versorgungsausfall und Teilversorgung beherrschen | [R] Keine Fremdspeisung über UART/Reset/GPIO | CPU an / Display aus und umgekehrt prüfen; hochohmige Ausgänge oder geeignete Entkopplung vorsehen. | TBD | TBD |
| **3** | | **Display, Touch und mechanische Integration** | | | | |
| 3.1 | F | Displaytechnologie einhalten | [D1] 3,5 Zoll, IPS-TFT, transmissiv | S. 4/40; 3,5 Zoll entsprechen rechnerisch 88,9 mm Diagonale. Kein 3,2-Zoll-TN-Display. | TBD | TBD |
| 3.2 | F | Native Auflösung verwenden | [D1] 320 × 480 Pixel hochkant | S. 6/40; [Z] UI quer mit 480 × 320 logischen Pixeln durch Rotation. Ausrichtung mit Frontplatte abstimmen. | TBD | TBD |
| 3.3 | F | Farbdarstellung berücksichtigen | [D1] RGB565, 16 Bit | S. 6; rechnerisch 65.536 Farbwerte. Internes Displayinterface ist SPI [D1] S. 4, kein extern anzusteuerndes paralleles RGB-Panel. | TBD | TBD |
| 3.4 | F | Optische Kennwerte berücksichtigen | [D1] 295 cd/m² bei „Contrast = 15“; Kontrast typisch 1000:1 | S. 40; nicht 320 cd/m² der Non-Touch-Ausführung übernehmen. Ablesbarkeit hinter finaler Front prüfen. | TBD | TBD |
| 3.5 | F | Sichtfläche und Blickwinkel berücksichtigen | [D1] Aktive Fläche 48,96 × 73,44 mm hochkant; je 80° in vier Richtungen | Zeichnung S. 34 und Tabelle S. 40; Sichtfenster, Randabdeckung und Einbautoleranzen separat auslegen. | TBD | TBD |
| 3.6 | F | Kapazitiven Touch erhalten und prüfen | [D1] CT mit Focaltech-Controller, intern über I²C | S. 17; konkrete Controllerrevision und Mehrfingerfunktion nicht zugesichert. Rotation von Bild und Touchkoordinaten gemeinsam testen. | TBD | TBD |
| 3.7 | W | Bedienung über Touch ergänzen | [Z] Play/Stop und ggf. Lautstärke | Nutzer fordert zunächst Anzeigen. Bedienbefehle nur nach Freigabe; CPU bestätigt Zustände, Display behauptet keine ungeprüfte Ausführung. | TBD | TBD |
| 3.8 | F | Modulabmessungen einplanen | [D1] 101,1 × 56,6 × 13,1 mm; Masse ca. 47 g | S. 7/34, CT ohne CLB. Keine Maße der CLB-Ausführung verwenden. Maßreihenfolge gemäß Modulzeichnung, nicht automatisch Frontplatten-Breite × Höhe. | TBD | TBD |
| 3.9 | F | Montagepunkte berücksichtigen | [D1] Vier Bohrungen Ø3,2 mm; Lochmittenabstand 48,6 × 93,1 mm | S. 6/34; für M3 vorgesehen. Spannungsfreie Befestigung, Abstandhalter, Schraubenlänge und Anzugsmoment TBD. | TBD | TBD |
| 3.10 | F | Einbauraum und Toleranzen freigeben | TBD: Frontausschnitt, Bautiefe, Kabelradien und Montageabstände | Zeichnung S. 34 nennt allgemeine lineare Toleranz 0,3 mm. Freiraum für USB-C, FFC, microSD und Steckvorgänge zusätzlich vorsehen; mit realem Muster/STEP prüfen. | TBD | TBD |
| 3.11 | F | Touchverträgliche Front gestalten | TBD: Material, Dicke, Luftspalt und Erdung | Keine Funktion durch beliebige zusätzliche Glasscheiben oder Metallblenden voraussetzen; im montierten Zustand prüfen. CT besitzt keinen überstehenden CLB-Kleberahmen. | TBD | TBD |
| 3.12 | F | Temperatur und Wärmeabfuhr prüfen | [D1] Umgebung −20 bis +65 °C; Lagerung −30 bis +80 °C | S. 37; Gehäusezielbereich TBD. Die höhere mögliche Chiptemperatur ist keine Freigabe für das komplette Displaymodul. | TBD | TBD |
| **4** | | **Schnittstellen, Pinbelegung, EMV und Schutz** | | | | |
| 4.1 | F | CPU-Verbindung als UART ausführen | [A] 3,3-V-Logik, gemeinsame GND-Referenz | Kein RS-232 und keine 5-V-UART direkt anschließen. Bei anderem CPU-Pegel geeignete Pegelanpassung vorsehen. | TBD | TBD |
| 4.2 | F | UART richtungsrichtig verdrahten | [D1] Pin 23 = U0RXD/GPIO44; Pin 24 = U0TXD/GPIO43 | CPU-TX → Pin 23; Pin 24 → CPU-RX. S. 9 enthält „U0TX0“ als Schreibweise, S. 10/14 und Schaltplan bestätigen TXD. | TBD | TBD |
| 4.3 | F | Logikgrenzen prüfen | [R] Nominal 0/3,3 V; [D1] ESP32-Eingang absolut −0,3 bis +3,6 V | S. 37 nennt widersprüchlich VIH max. 5,5 V „all pins“. Daraus ausdrücklich keine 5-V-Toleranz ableiten; siehe Randbedingungen und [E1]. | TBD | TBD |
| 4.4 | F | UART-Parameter vereinbaren | [Z] 115.200 Baud, 8N1, ohne Hardware-Handshake | Startwert für Statusdaten, kein Datenblattzwang. CPU-Unterstützung, Fehlerquote, Puffer und Datenbudget bestätigen. | TBD | TBD |
| 4.5 | F | UART0 mit Boot und Programmierung koordinieren | [R] Bootausgaben verwerfen, Protokoll erst nach Synchronisation aktivieren | UART0 wird auch für Programmierung verwendet [D1] S. 10. Externe CPU darf bei Programmierung keinen Ausgangskonflikt erzeugen; Trennmöglichkeit oder freigegebene alternative GPIO-UART vorsehen. | TBD | TBD |
| 4.6 | F | Reset- und Bootsignale korrekt behandeln | [D1] Pin 22 EN-RST aktiv Low, interner 10-kΩ-Pull-up; Pin 19 GPIO0 | GPIO0 beim normalen Boot nicht extern Low ziehen. Für Reset vorzugsweise Open-Drain gegen GND [Z]; niemals 5 V auf EN-RST. S. 14/15. | TBD | TBD |
| 4.7 | F | FFC-Verbindung spezifizieren | [D1] 30-polig, 0,5-mm-Raster, 0,3-mm-Kabeldicke; mitgeliefert 150 mm Typ B | S. 26: Kontakte an entgegengesetzten Kabelseiten. Kontaktlage und Pin-1-Durchgängigkeit an beiden Steckern prüfen; Kabel nicht unter Spannung stecken [R]. | TBD | TBD |
| 4.8 | F | Verfügbare GPIO vor Zusatzfunktionen prüfen | [D1] Pins 16/17/18 standardmäßig nicht verbunden | GPIO20/19 sind USB zugeordnet, GPIO11 dem Touch-Reset; Änderungen erfordern Hardwareumbau [D1] S. 14/16. Kein Umbau für die Basisfunktion vorgesehen [Z]. | TBD | TBD |
| 4.9 | F | Erweiterungsschnittstellen konfliktfrei reservieren | [D1] GPIO17/18 an Pins 2/3 mit I²C-Pull-ups; weitere Funktionen multiplexbar | Interne Display-, Touch-, Speicher- und USB-Verwendung berücksichtigen. GPIO0/3/45/46 wegen Boot-Strapping besonders prüfen [E1]. Konkrete Zusatzbelegung TBD. | TBD | TBD |
| 4.10 | F | Signalqualität und EMV im CD-Player prüfen | TBD: Kabellänge, Flanken, Störpegel und Schutzwerte | UART ist unsymmetrisch. Rückstrompfade kurz halten; Abstand zu Analog-/Taktpfaden, ESD-Schutz für zugängliche Anschlüsse und Filter nach Messung auslegen. | TBD | TBD |
| 4.11 | F | Audiowiedergabe vor Displaystörungen schützen | [R] Keine unzulässige Verschlechterung; Grenzwerte TBD | Mit maximaler Bildaktivität, Helligkeitsregelung und optionalem Funk prüfen; Rauschen und Schaltspitzen an Audioausgängen messen. | TBD | TBD |
| **5** | | **ESP32, Kommunikation und Datenaufbereitung** | | | | |
| 5.1 | F | Vorhandene Rechenplattform nutzen | [D1] ESP32-S3R8, 8 MB Octal-PSRAM, 16 MB externer Quad-SPI-Flash | S. 6/17; nutzbare Kapazität durch Firmware, Partitionen und Reserven kleiner. Kein zusätzlicher ESP32 für die Basisanzeige notwendig. | TBD | TBD |
| 5.2 | F | Reproduzierbare Firmware bereitstellen | [Z] C++ mit unterstützter Displaybibliothek; konkrete Versionen TBD | [D1] S. 24/25 beschreibt Workshop4 mit Arduino-ESP32. Build, Bibliotheken, Boardprofil, Rotation und Flashlayout dokumentieren. | TBD | TBD |
| 5.3 | F | Steuerungs-CPU als Zustandsquelle verwenden | [A/R] CPU liefert bestätigten Playerzustand | ESP32 übernimmt Darstellung und ggf. Bedienanforderungen; Laufwerkssteuerung verbleibt bei der CPU. | TBD | TBD |
| 5.4 | F | Nachrichtenformat spezifizieren | [Z] Typ, Version, Sequenz, Länge, Nutzdaten, Integritätsprüfung | Framing, Prüfsumme/CRC samt Parametern, Byte-Reihenfolge, Wertebereiche, Timeout und Resynchronisation verbindlich dokumentieren; Detailformat TBD. | TBD | TBD |
| 5.5 | F | Vollständigen Status übertragen | [A/R] Wiedergabezustand, Track x/y, Position, Trackdauer, Lautstärke und Gültigkeit | Zusätzlich Disc-/Medienwechselkennung vorsehen [Z], um Daten verschiedener CDs nicht zu vermischen. Fehlerzustand und Verfügbarkeit getrennt von Zahlenwerten übertragen. | TBD | TBD |
| 5.6 | F | Aktualität der Anzeigen sicherstellen | [Z] Status ≥5 Hz oder bei Änderung plus Heartbeat; Zeitdarstellung mindestens 1 Hz | Reaktion auf bestätigten Statuswechsel ≤200 ms ab Empfang [Z]. Tatsächliche CPU-Rate abstimmen. | TBD | TBD |
| 5.7 | F | Kommunikationsausfall erkennen | [Z] Nach 1 s ohne gültigen Status als veraltet markieren | Werte nicht als aktuell weiterführen; „Verbindung unterbrochen“ anzeigen. Wiederanlauf erst mit vollständigem gültigem Status. | TBD | TBD |
| 5.8 | F | Ungültige Daten sicher behandeln | [R] Keine Abstürze, Überläufe oder unbestätigten Zustandswechsel | Teiltelegramme, falsche Länge/CRC, unbekannte Version und Werte außerhalb des Bereichs verwerfen; Fehlerzähler vorsehen [Z]. | TBD | TBD |
| 5.9 | F | Subchannel-Daten in der CPU auswerten | [A] Kein Rohdatenstrom zum Display erforderlich | Track/Index und Zeiten aus verfügbaren gültigen Decoderinformationen; genaue Subcode-Schnittstelle und Prüflogik TBD. Q-Subcode und TOC unterscheiden. | TBD | TBD |
| 5.10 | F | Trackanzahl und Trackdauer belastbar bestimmen | [R] TOC/Decoder-Metadaten plus aktuelle Position | Nicht voraussetzen, dass ein einzelner laufender Subchannel-Datensatz alle Angaben enthält. Tracklänge ggf. aus Folgetrack-Start bzw. Lead-out berechnen; Pregap, Index 00 und Datentracks festlegen. | TBD | TBD |
| 5.11 | F | Lautstärkedaten aus richtiger Quelle beziehen | [A/R] Rückmeldung der CPU bzw. Lautstärkestufe | Lautstärkeeinstellung ist kein verlässlich aus CD-Subcode ableitbarer Wert. Bei analogem Regler ohne Rückmeldung ist eine zusätzliche Erfassung erforderlich, Ausführung TBD. | TBD | TBD |
| 5.12 | F | Medienablage und Fehlerverhalten definieren | [Z] Kritische Texte/Statusanzeige ohne microSD verfügbar | Logo/Animation im Flash oder auf microSD nach Bibliotheksprüfung; fehlende/defekte Karte darf die Statuskommunikation nicht blockieren. [D1] S. 18/19: SPI-kompatible Karte, FAT16/FAT32; konkrete Kapazität und Formatierung mit Bibliothek testen, keine 32-GB-Obergrenze belegt. | TBD | TBD |
| 5.13 | F | Nicht benötigten Funk deaktivieren | [Z] WLAN/Bluetooth im Normalbetrieb aus | Reduziert Last und potenzielle Audiostörungen; keine Netzwerkfunktion vom Auftraggeber gefordert. | TBD | TBD |
| 5.14 | F | Blockierende Grafikabläufe vermeiden | [R] Kommunikation und Fehlererkennung laufen während Animationen weiter | Watchdog, Empfangspuffer und Speichergrenzen festlegen; CPU-Neustart und Display-Neustart unabhängig behandeln. | TBD | TBD |
| **6** | | **Benutzeroberfläche, Start/Abschaltung und optionale Visualisierung** | | | | |
| 6.1 | F | Play und Stop eindeutig anzeigen | [A] Symbole und/oder Text für Play/Stop | Nur bestätigten CPU-Zustand darstellen. Zusätzlich Laden, keine Disc und Fehler unterscheidbar [Z]; Pause anzeigen, falls CPU sie unterstützt. | TBD | TBD |
| 6.2 | F | Tracknummer und Gesamtzahl darstellen | [A] „Track x von y“, z. B. „1 von 8“ | 8 ist ein Beispiel, keine feste Obergrenze. [Z] Format für 1–99 Tracks; unterstützte Disc-Arten und tatsächlicher Wertebereich TBD. Unbekannte Zahl als „—“. | TBD | TBD |
| 6.3 | F | Trackzeit darstellen | [A] „00:00 / 04:30“ als vergangene Zeit / Trackdauer | Nicht ohne Kennzeichnung Disc-Gesamtzeit verwenden. Countdown/Pregap- und Überlaufformat TBD; unmögliche Werte nicht stillschweigend als gültig anzeigen. | TBD | TBD |
| 6.4 | F | Lautstärke darstellen | [A] Balken und Prozentwert; [Z] 0–100 % | CPU-Skala verbindlich auf Prozent abbilden; Prozentwert ist Stellwert, kein Schalldruckpegel. Mute gesondert anzeigen, falls verfügbar. | TBD | TBD |
| 6.5 | F | TH-AB-Startbildschirm anzeigen | [A] TH-AB-Logo während des Starts | Schreibweise gemäß aktuellem Auftrag. Originalgrafik und Nutzungsfreigabe TBD; keine erfundene endgültige Markengrafik. | TBD | TBD |
| 6.6 | F | Startverhalten spezifizieren | [Z] Erstes sichtbares Startbild ≤2 s, betriebsbereite UI ≤5 s ab stabilen 5 V | Noch kein gemessener Datenblattwert. Bis gültiger CPU-Status vorliegt „Initialisierung“ anzeigen; Fehler bei ausbleibender Verbindung erkennbar machen. | TBD | TBD |
| 6.7 | F | Abschaltanimation ausführen | [A] Animation bei regulärem Ausschalten; [Z] Dauer ca. 1 s | Nicht als Garantie bei abruptem Netzausfall verstehen; Inhalt/Design TBD. | TBD | TBD |
| 6.8 | F | Geordnetes Ausschaltprotokoll realisieren | [Z] SHUTDOWN-Anforderung → Animation → READY → Versorgung aus | CPU/Power-Controller hält Displayversorgung bis READY, maximal 2 s [Z]; Timeout führt deterministisch zur Abschaltung. Konkrete Nachrichtennamen sind Vorschläge. | TBD | TBD |
| 6.9 | F | Energie bis zum Animationsende bereitstellen | [R] Geschaltete 5-V-Schiene mit ausreichender Nachlaufzeit | Ein harter Netzschalter ohne Halteschaltung verhindert die Animation. Falls auch Netzausfall animiert werden soll: zusätzliche Energiespeicherung, frühzeitige Erkennung und Dimensionierung TBD. | TBD | TBD |
| 6.10 | F | Lesbarkeit und Helligkeit abstimmen | TBD: Schriftgröße, Farben, Mindestkontrast und Dimmstufen | Test im tatsächlichen Betrachtungsabstand, bei hell/dunkel und im Gehäuse; Helligkeitssteuerung auf Flicker und Audiostörungen prüfen. | TBD | TBD |
| 6.11 | W | Audiosynchrone Visualisierung über S/PDIF ergänzen | [A] Optionale Pegel-/Spektrumanimation; Umfang TBD | Separate Machbarkeitsentscheidung. Statusanimation aus Play/Stop ist keine Messung des Audiosignals. | TBD | TBD |
| 6.12 | W | S/PDIF-Empfang technisch korrekt umsetzen | [R] Passender Receiver → PCM/I²S → Auswertung → Grafikdaten | Beispiel eines Receivers mit I²S-Ausgang: DIR9001 [E2], noch keine Bauteilfestlegung. Kein direkter Anschluss von S/PDIF an UART; Datenverarbeitung zusätzlich erforderlich. | TBD | TBD |
| 6.13 | W | Audioauswertung und Datenrate festlegen | [Z] Peak/RMS oder Frequenzbänder, 20–30 Aktualisierungen/s | Entweder CPU/Zusatzprozessor sendet kompakte Werte per UART, oder ESP32 empfängt I²S an freigegebenen GPIO. Rohes CD-PCM passt nicht durch den vorgeschlagenen 115.200-Baud-Link. | TBD | TBD |
| 6.14 | W | S/PDIF-Abgriff rückwirkungsarm auslegen | TBD: optischer, koaxialer oder interner Abgriff | Eingangspegel, Impedanz, Puffer/Übertrager und Masseführung nach Quelle und Receiver bestimmen. Bestehenden Audioausgang nicht durch parallele Fehlterminierung belasten. | TBD | TBD |
| 6.15 | W | S/PDIF-Ausfälle und Messpunkt kennzeichnen | [R] Kein Lock / ungültiges Signal → keine scheinbar gültige Pegelanzeige | Messpunkt vor/nach Lautstärkestellung dokumentieren; digitaler Audiopegel ist nicht automatisch die eingestellte Lautstärke. CD-Subchannel und S/PDIF-Channel-Status nicht gleichsetzen. | TBD | TBD |
| **7** | | **Prüfung und Dokumentation** | | | | |
| 7.1 | F | Schaltplan- und Pinprüfung dokumentieren | [R] 100 % der angeschlossenen FFC-Pins gegen Anlage A prüfen | Stecker-Kontaktseite, Kabeltyp, TX/RX, GND und Spannungsnetze vor Einschalten durchmessen; reale Hardwareversion einbeziehen. | TBD | TBD |
| 7.2 | F | Versorgung unter allen Betriebszuständen prüfen | [Z] 4,75–5,25 V am Modul; keine Brownouts/Neustarts | Start, maximale Grafiklast, Touch, SD-Zugriff, Standby, Abschalten sowie USB-Servicebetrieb messen. Strommaxima und Ripple mit Messbandbreite protokollieren. | TBD | TBD |
| 7.3 | F | Funktionsabnahme mit Referenzdaten durchführen | [R] Alle Pflichtanzeigen stimmen mit CPU/Referenz überein | Play/Stop, erster/letzter Track, verschiedene Trackanzahlen, Zeitgrenzen, Lautstärke min/max, unbekannte Daten, Discwechsel, keine Disc und Laufwerksfehler prüfen. | TBD | TBD |
| 7.4 | F | Kommunikations- und Wiederanlauftest durchführen | [Z] ≥8 h Dauerbetrieb ohne unbeabsichtigten Neustart | Fehlerhafte/verlorene Telegramme, Kabelunterbrechung und getrennte CPU-/Display-Resets injizieren; Timeout und Resynchronisation nachweisen. | TBD | TBD |
| 7.5 | F | Ein-/Ausschaltablauf prüfen | [Z] 100 reguläre Zyklen ohne Fehlstart | Animation, READY/Timeout, plötzliches Abschalten, Restversorgung und USB-Sonderfall prüfen. Zielanzahl ist vorläufig. | TBD | TBD |
| 7.6 | F | Mechanik, Touch und thermischen Betrieb prüfen | [R] Kollisionsfrei, stabil, keine Fehlbedienung oder thermische Ausfälle | Alle Touchbereiche in finaler Rotation und Frontplatte testen; Gehäuse-Maximaltemperatur und Freiräume dokumentieren. | TBD | TBD |
| 7.7 | F | Audio-/EMV-Vergleich durchführen | TBD: zulässige Rausch-/Störgrenzen und Prüfverfahren | Identische Bedingungen mit Display inaktiv/aktiv/maximaler Grafiklast; Abnahmekriterien vor Messung vereinbaren. | TBD | TBD |
| 7.8 | W | S/PDIF-Option separat abnehmen | [R] Funktion, Lock-Verlust, Kanäle und Audiopfad-Rückwirkung prüfen | Referenzsignale mit bekanntem Pegel/Frequenz; geforderte Bandbreite, Genauigkeit und Latenz TBD. | TBD | TBD |
| 7.9 | F | Entwicklungsunterlagen bereitstellen | [R] Schaltplan, Anschlussplan, Stückliste, Firmware und Build-Anleitung | Zusätzlich Protokollspezifikation, UI-/Grafikdateien, Versionsstände und ggf. editierbares Adapter-PCB-Projekt. Kein Modul-PCB-Redesign pauschal gefordert. | TBD | TBD |
| 7.10 | F | Abnahme nachvollziehbar dokumentieren | [R] Soll/Ist, Messbedingungen, bestanden/nicht bestanden und Restpunkte | Zielwerte vor Abnahme freigeben; ungeklärte elektrische Grenzwerte oder Pinbelegungen verhindern die Hardwarefreigabe. | TBD | TBD |
| **8** | | **Termine und Verantwortlichkeiten** | | | | |
| 8.1 | F | Bestandsanalyse und Datenblattklärung abschließen | TBD | Steuerungs-CPU, Quellen und widersprüchliche Grenzwerte klären. | TBD | TBD |
| 8.2 | F | Versorgung und Anschlussplan freigeben | TBD | Pinplan, Strombudget, Masse- und Ausschaltkonzept. | TBD | TBD |
| 8.3 | F | Mechanik und UI-Entwurf freigeben | TBD | Frontplatte, Touchumfang, TH-AB-Logo und Darstellungsformate. | TBD | TBD |
| 8.4 | F | UART-Protokoll und Basisfirmware fertigstellen | TBD | Vollständige Pflichtanzeigen und Fehlerbehandlung. | TBD | TBD |
| 8.5 | W | Über S/PDIF-Option entscheiden | TBD | Aufwand, Hardware, Messpunkt, Datenpfad und Rechenbudget bewerten. | TBD | TBD |
| 8.6 | F | Gesamtsystem testen und dokumentieren | TBD | Abnahme der freigegebenen F-Anforderungen und beauftragten W-Anforderungen. | TBD | TBD |

## Offene Punkte

- [ ] Projektpartner, Ansprechpartner, Termine und Verantwortlichkeiten bestätigen.
- [ ] Lieferantenartikel „gen4-ESP32-35CT-ND“ auf tatsächliche Herstellerteilenummer und Boardrevision abbilden.
- [ ] Abweichende Angaben zu Versorgung, VIH und UART-Anzahl mit Hersteller-/Chipdokumentation verbindlich auflösen.
- [ ] Steuerungs-CPU, deren UART-Pegel, Pinbelegung, Protokoll und verfügbare Ressourcen bestimmen.
- [ ] Belastbarkeit und Toleranzen der +5-/+9-/+15-/−15-V-Schienen sowie Massebezug messen.
- [ ] Stromspitzen, Einschaltstrom, Wärmeentwicklung und zusätzliche Lasten ermitteln.
- [ ] Laufwerksdecoder, Subcode-/TOC-Zugriff, Disc-Typen und Zeitsemantik festlegen.
- [ ] Lautstärkerückmeldung, Skalierung und Mute-Semantik klären.
- [ ] Abschaltsteuerung und 5-V-Nachlauf realisieren; Verhalten bei hartem Netztrennen vereinbaren.
- [ ] TH-AB-Originalgrafik, Animationsinhalt und Nutzungsrechte bereitstellen.
- [ ] Frontausschnitt, Rotation, Betrachtungsabstand, Touchfunktion und endgültige Mechanik freigeben.
- [ ] Grafikbibliothek, Firmwareversionen, Medienablage und Wiederherstellungsweg festlegen.
- [ ] S/PDIF-Abgriff, Receiver, PCM-Verarbeitung, Messpunkt und GPIO-Budget nur bei Freigabe der Option spezifizieren.
- [ ] Vorläufige Zielwerte einschließlich Zeiten, Versorgungstoleranz und Tests freigeben.
- [ ] Audio-/EMV-Grenzwerte und Prüfbedingungen definieren.

## Anlage A – Geprüfte Anschlussbelegung des 30-poligen User-FFC

Quelle: [D1], S. 8–16 und Schaltplan S. 36. Nummern sind **FFC-Kontaktnummern**, nicht ESP32-Gehäusepins. Richtungen gelten aus Sicht des Displays. Die folgende Tabelle ist kein Ersatz für die Prüfung der Kontaktseite des Gegensteckers.

| FFC-Pin | Signal / GPIO | Funktion und Verwendung |
| --- | --- | --- |
| 1 | GND | Versorgungs-/Signalmasse; anschließen. |
| 2 | GPIO17 | Freier GPIO, für I²C mit Pull-up vorgesehen; Basisprojekt unbenutzt. |
| 3 | GPIO18 | Freier GPIO, für I²C mit Pull-up vorgesehen; Basisprojekt unbenutzt. |
| 4 | GPIO16 | GPIO; Basisprojekt unbenutzt. |
| 5 | GPIO15 | GPIO; Basisprojekt unbenutzt. |
| 6 | GPIO48 | GPIO; Basisprojekt unbenutzt. |
| 7 | GPIO47 | GPIO; Basisprojekt unbenutzt. |
| 8 | GPIO38 | GPIO; Basisprojekt unbenutzt. |
| 9 | GPIO39 | GPIO; Basisprojekt unbenutzt. |
| 10 | GPIO40 | GPIO; Basisprojekt unbenutzt. |
| 11 | GPIO6 | GPIO; Basisprojekt unbenutzt. |
| 12 | GPIO5 | GPIO; Basisprojekt unbenutzt. |
| 13 | GPIO3 | GPIO/Strapping; unbenutzt lassen, sofern Bootverhalten nicht geprüft. |
| 14 | GPIO45 | Strapping; keine freie Nutzung ohne Prüfung der Boot-Konfiguration. |
| 15 | GPIO46 | Strapping; Ein-/Ausgabefähigkeiten anhand [E1] prüfen, nicht als beliebiger Ausgang einplanen. |
| 16 | GPIO20, standardmäßig NC | Intern USB D+; FFC erst nach Hardwareänderung nutzbar. |
| 17 | GPIO19, standardmäßig NC | Intern USB D−; FFC erst nach Hardwareänderung nutzbar. |
| 18 | GPIO11, standardmäßig NC | Intern Touch-Reset; beim CT-Modul für Touch erhalten. |
| 19 | GPIO0 | Boot/Download; normal nicht extern Low halten. |
| 20 | 3,3 V OUT | Ausgang des Modulnetzes; im Basisprojekt keine externe Last. |
| 21 | GND | Masse; anschließen. |
| 22 | EN-RST | Eingang, Low = Reset/Disable, interner 10-kΩ-Pull-up auf 3,3 V. |
| 23 | U0RXD / GPIO44 | Eingang; an CPU-TX mit 3,3-V-Pegel. |
| 24 | U0TXD / GPIO43 | Ausgang; an CPU-RX mit kompatiblem 3,3-V-Pegel. |
| 25 | GND | Masse; anschließen. |
| 26 | 5V IN | An geregelte +5 V. |
| 27 | 5V IN | An dieselbe geregelte +5-V-Schiene. |
| 28 | 5V IN | An dieselbe geregelte +5-V-Schiene. |
| 29 | 5V IN | An dieselbe geregelte +5-V-Schiene. |
| 30 | GND | Masse; anschließen. |

**Basisverdrahtung:** CPU-TX → FFC 23; CPU-RX ← FFC 24; +5 V → FFC 26–29; gemeinsame Masse → FFC 1/21/25/30. Reset optional über geeigneten Open-Drain-Treiber an FFC 22. Die CPU-Pinnummern bleiben mangels CPU-Schaltplan TBD.

## Anlage B – Kritische technische Randbedingungen und Prüfergebnis

### 1. Datenblattwidersprüche dürfen nicht als Freigabe verwendet werden

Das hochgeladene Dokument ist Rev. 1.0, nicht automatisch der aktuelle Freigabestand. Seine Spannungstabelle und Pinliste nennen 4,0–6,0 V, Abschnitt 4.5 dagegen 4,0–5,5 V. Der vorgeschlagene Bereich 4,75–5,25 V hält Abstand zu beiden Grenzen. 9 V, +15 V und −15 V sind unabhängig von dieser Unstimmigkeit unzulässig am Moduleingang.

Auf S. 37 steht bei VIH ein Maximalwert von 5,5 V für „all pins“, obwohl dieselbe Seite für ESP32-Eingänge absolut höchstens 3,6 V und die Pinbeschreibungen ausdrücklich 3,3-V-Logik nennen. Auch eine dort aufgeführte IO-Expander-Grenze darf nicht auf die direkt angebundenen UART-GPIO übertragen werden. Für die Integration gilt ausschließlich die 3,3-V-Domäne. [E1] nennt für den Chip VIH mindestens 0,75 × VDD und VIL höchstens 0,25 × VDD; bei 3,3 V sind das 2,475 V bzw. 0,825 V. Ausgangspegel der konkreten CPU, Masseversatz und Störreserve müssen dazu passen. Absolute Grenzwerte sind keine Zielpegel.

Auch die UART-Zählung in [D1] ist uneinheitlich: S. 6 nennt drei Kanäle sowie an anderer Stelle sinngemäß weitere UARTs, S. 11 nennt zwei. Der ESP32-S3 besitzt insgesamt drei Hardware-UART-Controller [E1]. Sicher am FFC herausgeführt ist UART0 an Pins 23/24; weitere nutzbare Ports hängen von GPIO-Zuordnung und belegten Ressourcen ab. Nicht alle Tabellenangaben zu Analogfunktionen und GPIO sind als garantierte freie Modulressourcen zu übernehmen.

### 2. CT, RGB-Farbe und mechanische Ausführung auseinanderhalten

gen4-ESP32-35CT ist ein intelligentes Displaymodul mit integriertem ESP32-S3R8, SPI-Anbindung des LCD und kapazitivem Touch. RGB565 beschreibt das Farbformat, keine notwendige RGB-Parallelverbindung zur Steuerungs-CPU. Die CPU muss daher keine Vollbilder übertragen; Statusnachrichten genügen, wenn die Grafik auf dem ESP32 erzeugt wird.

Die Maßzeichnung auf S. 34 wurde visuell geprüft: 101,1 × 56,6 mm Außenmaß, 13,1 mm Tiefe für CTP, Lochmitten 93,1 × 48,6 mm, Ø3,2-mm-Bohrungen und aktive Fläche 73,44 × 48,96 mm. Die Frontöffnung ist daraus nicht ohne Montage-, Abdeckungs- und Toleranzkonzept ableitbar. Die 12,4-mm-Tiefe gehört zur Non-Touch-Ausführung. Das CLB-Modell mit überstehender Glasfront ist mechanisch anders.

### 3. 184 mA sind kein Netzteil-Maximalwert

Der Datenblattwert gilt ohne WLAN, ohne microSD und bei einfachen Grafikoperationen. Versorgung, FFC und Schutz müssen anhand der höchsten tatsächlichen Last einschließlich Boot und Zusatzfunktionen ausgelegt werden. Eine 1-A-Quellreserve ist ein Entwurfsansatz, keine Behauptung, das Modul benötige 1 A oder könne diese Last über jeden Kontakt führen. Externe Lasten am 3,3-V-Ausgang belasten den Modulregler zusätzlich.

### 4. Trackdaten benötigen eine belastbare Quelle

Der Datenpfad lautet Laufwerk/Decoder → Steuerungs-CPU → UART → Display. Das Displaydatenblatt beschreibt keine CD-Subcode-Decodierung. Welche Track-/Index-/Zeitinformationen verfügbar sind, muss am konkreten Laufwerksdecoder geprüft werden. Gesamtzahl und Trackdauer können TOC-Auswertung benötigen; die Dauer des letzten Tracks kann beispielsweise den Lead-out erfordern. Laufende Q-Informationen, TOC und gegebenenfalls CD-Text sind unterschiedliche Datenquellen. Für die Basisanzeige wird CD-Text nicht vorausgesetzt. Lautstärke stammt aus dem Lautstärkesteller bzw. der CPU und nicht aus dem Audiodatenträger.

### 5. Abschaltanimation erfordert Strom und ein Vorwarnsignal

Die CPU muss das Ausschalten ankündigen, während die +5-V-Schiene noch stabil ist. Nach abgeschlossener Animation wird die Versorgung abgeschaltet; ein Timeout verhindert unbegrenztes Warten. EN-RST ersetzt keine vollständige elektrische Abschaltung des Moduls. Bei hartem Trennen des Netzteils kann ohne Speicher keine Animation zugesichert werden. Nur als Größenordnung: Bei konstant 184 mA, 1 s Überbrückung und 1 V zulässigem Spannungsabfall ergibt C = I × t / ΔU bereits rund 0,184 F. Dieser idealisierte Wert berücksichtigt weder ESR noch Spitzenstrom, Verluste oder Toleranzen und ist keine Kondensatorauslegung; im engeren Projekt-Spannungsfenster wäre noch mehr Kapazität nötig. Eine gesteuerte Versorgung ist für den normalen Ausschaltvorgang zweckmäßiger.

### 6. S/PDIF ist weder UART noch automatisch eine Lautstärkemessung

Ein S/PDIF-Receiver stellt üblicherweise digitale Audiodaten etwa über I²S bereit; der DIR9001 ist hierfür ein Herstellerbeispiel [E2]. Für eine Visualisierung müssen ein Prozessor und Firmware daraus Pegel oder Spektralwerte berechnen. Erst diese reduzierten Werte lassen sich wirtschaftlich über UART übertragen. Alternativ kann der ESP32 I²S empfangen, sofern GPIO, Taktung, DMA und Rechenbudget geprüft sind.

Bei CD-PCM mit 44.100 Abtastungen/s, zwei Kanälen und 16 Bit entstehen rechnerisch 1.411.200 Bit/s reine Audionutzdaten. Der vorgeschlagene UART mit 115.200 Baud und 8N1 trägt höchstens 11.520 Nutzbytes/s vor Protokolloverhead. Das reicht für Status- und reduzierte Visualisierungsdaten, nicht für diesen PCM-Strom. S/PDIF-Channel-Status ist nicht mit den CD-Subchannel-Daten gleichzusetzen. Die am gewählten Abgriff tatsächlich transportierten Informationen müssen geprüft werden. Bei einem Abgriff vor der Lautstärkeregelung zeigt eine Pegelanimation den digitalen Audioinhalt, nicht die tatsächliche Ausgangslautstärke.

## Quellen und Nachweisgrenzen

- **[V1] Hochgeladene Vorlage:** `anforderungsliste(1).md`, Stand 16.10.2026. Übernommen wurden Projektkopf, Legende, sieben Tabellenspalten einschließlich Nummerierung, acht Themenblöcke, Termin-/Verantwortungsspalten und offene Punkte. Sensor-/Web-Anforderungen wurden durch die CD-Display-Anforderungen ersetzt. Personen und Firmen wurden nicht ungeprüft als Projektbeteiligte übernommen.
- **[D1] Hochgeladenes Datenblatt:** `2835C6A3A5448A6E9A19F34F27F9AAB74850A920F46D1F6E993AFE0AFD11C815.pdf`, „gen4-ESP32 Series“, Revision 1.0, 12.09.2023, 43 Seiten. Relevante Fundstellen: S. 4/6/7 Modulart und Eckdaten; S. 8–16 User-FFC und Schnittstellen; S. 17–19 Controller und Speicher; S. 23–26 USB, Software und Kabel; S. 34 Mechanik; S. 36 Schaltplan; S. 37–41 elektrische/optische Werte; S. 42 Revision. Diese hochgeladene Fassung ist die primäre Zahlenquelle dieser Liste.
- **Aktueller Herstellerzugang:** [4D Systems – gen4-ESP32 Series Dokumentation](https://resources.4dsystems.com.au/datasheets/esp32/gen4-esp32/) und [Ressourcen für gen4-ESP32-35CT](https://resources.4dsystems.com.au/downloads/esp32/gen4-ESP32-35CT/), abgerufen am 22.10.2026. Dort verlinkte neuere Revisionen müssen vor Hardwarefreigabe mit dem gelieferten Modul abgeglichen werden; Seitenzahlen aus [D1] nicht auf andere Revisionen übertragen.
- **[E1] Espressif:** [ESP32-S3 Series Datasheet](https://documentation.espressif.com/esp32_s3_datasheet_en.pdf), ergänzend [ESP32-S3 Hardware Design Guidelines – Schematic Checklist](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html), abgerufen am 22.10.2026. Verwendet zur Einordnung der 3,3-V-Logik, UART-Anzahl und Boot-Strapping-Risiken.
- **[E2] Texas Instruments:** [DIR9001 – Digital Audio Interface Receiver](https://www.ti.com/product/DIR9001), abgerufen am 22.10.2026. Belegt den beispielhaften S/PDIF-zu-I²S-Datenpfad; keine Festlegung auf diesen Receiver.

**Prüfstatus:** Dokumentenprüfung einschließlich visueller Kontrolle der relevanten Maßzeichnung, des Schaltplans und der widersprüchlichen Grenzwerttabelle abgeschlossen. Die Liste ist als vollständiger Anforderungsentwurf für den beschriebenen Umfang nutzbar. Hardwarefreigabe, abschließende Dimensionierung und Abnahme erfordern die gekennzeichneten TBD-Angaben, Projektentscheidungen und Messungen.
