# MQTT ESP32 E-paper Türschild
Ein ESP32-gesteuertes Epaper Display, das durch eingehende MQTT Nachrichten die Anzeige ändert.

## Benötigte Hardware
* ESP32 Development Board
* Adafruit 2.7" b/w epaper Display
* Raspberry Pi 3+ (benötigt WLAN Modul)
* SD-Karte (ca. 4GB)
* Batteriehalter/Powerbank
* MicroUSB Kabel

## Benötigte Software
* Arduino Libraries (durch ArduinoIDE installierbar):
    1. AsyncMqttClient
    2. Adafruit-GFX-Library
    3. GxEPD
    4. WiFi
* Raspbian Image für RaspberryPi
* Mosquitto (auf RaspberryPi)

---

## Installation
### RaspberryPi
1. Brennen von Raspbian auf SD Karte, z.B. mit Etcher(Windows/MacOS/Linux)
2. Erstellen einer Datei namens `SSH` ohne Dateiendung auf der Boot Partition der SD Karte. Dies erlaubt die Verwendung des SSH Protokolls.
3. Pi starten und per Ethernet Kabel mit Netzwerk verbinden. 
4. IP Adresse des Pis herausfinden, z.B. über Routerinterface oder ein Tool wie Nmap. Wird für die nächsten Schritte benötigt. 
5. Anderen Computer im selben Netzwerk verwenden und mit dem Terminal durch `ssh pi@IPADRESSE`SSH Verbindung aufbauen. Standard Password: `raspberry`.
6. Installieren des Mosquitto Server und Clients durch `sudo apt install mosquitto mosquitto-clients`. Nach der Installation wird automatisch ein Mosquitto Server gestartet.

### ESP32
1. Installieren aller benötigten Bibliotheken mithilfe der Bibliotheksverwaltung. Liste siehe oben.
2. Downloaden/Entpacken des Arduino Codes.
3. Installieren des ESP32 Boardtreibers:
    * In den Voreinstellungen unter zusätzliche Boardverwalter `https://dl.espressif.com/dl/package_esp32_index.json` eintragen.
    * Dann unter Werkzeuge > Boardverwalter nach esp32 suchen und installieren.
4. Werkzeuge > Uploadspeed auf 115200 setzen.
5. Den ESP32 anschließen, den richtigen Port unter Werkzeuge einstellen und den Sketch hochladen. Nach dem Kompilieren muss auf dem ESP evtl. die Boot Taste gedrückt werden, um das Hochladen zu ermöglichen.
6. Das Display an den ESP32 anschließen:
    ```
    Display   |   ESP32
    -------------------
    BUSY      |     D4
    RST       |     RX2
    DC        |     TX2
    CS        |     D5
    CLK       |     D18
    DIN       |     D23
    GND       |     GND
    VCC       |     3V3
    ```

## Anpassen des Arduino Sketches:
1. Im Arduino Sketch muss die `WIFI_SSID` und das `WIFI_PASSWORD` auf das verwendete Zielnetzwerk angepasst werden.
2. Zusätzlich muss die `MQTT_HOST` IPAdresse auf die Adresse des Raspberry Pi geändert werden.
3. Optional kann unter `MQTT_PORT` der Port des MQTT Brokers angepasst werden. Für diese Konfiguration allerdings nicht notwendig, es wird der Standart Port 1883 verwendet.

* Zusätzlich können weitere Nachrichten hinzugefügt werden. Dazu muss die reactToPayload() Funktion um die neuen Möglichkeiten erweitert werden. 

---

## Funktionsweise:
Wenn der Raspberry Pi gestartet, und der ESP32 mit Strom versorgt ist, kann man mit einem MQTT Client überprüfen ob alles korrekt funktioniert. Beispielsweise MQTT.fx (Win/MacOS/Linux) oder der Smartphone App MQTTool (iOS). 

1. Nach Starten des Clients muss man sich mit dem Broker/Server verbinden. Dazu gibt man die IP Adresse des Raspberry Pis und den Port 1883 ein.
2. Wenn die Verbindung erfolgreich aufgebaut wurde, können Nachrichten an die gewählten Topics gesendet werden. 
In der Standartkonfiguration abonniert der ESP32 die Topic `doorsign/update` mit Quality of Service (QoS) 2. Aktualisierungen müssen an diese Topic gesendet werden.
Desweiteren kann man die Topic `doorsign/status` mit dem Client abonnieren, um Statusupdates des ESP zu bekommen ohne ihn am Seriellenmonitor auszulesen. Der ESP schickt eine Nachricht an die Status-Topic wenn er die Verbindung aufgebaut hat. Zusätzlich sendet er nach jeder eingegangen Aktualisierungsnachricht eine Nachricht an die Status-Topic mit dem Inhalt der erhaltenen Nachricht um ein einfacheres Debugging und Überprüfen zu ermöglichen.

## Mögliche Aktualisierungen:

Müssen an Topic `doorsign/update` gesendet werden.

* `DA` zeigt das "Professor ist da" Bitmap an.
* `WEG` zeigt das "Professor ist nicht da" Bitmap an.
* `MXXX`, wobei XXX für eine dreistellige Raumnummer steht, zeigt die Nachricht das der Professor in Raum XXX ist an.
* Nachrichten mit anderem Inhalt werden direkt auf das Türschild geschrieben.

---

## ToDo's:

* Bitmaps auslagern
* Zusätzliche Bitmaps erstellen
* Nachrichten mit anderem Inhalt ODER LÄNGER 4 Zeichen schreiben den Inhalt des Payloads
* Ungewollte Zeichen nach eigenen Nachrichten entfernen
* Deepsleep mit Timer funktionsfähig machen
* MQTT Steuerung mit Interface auf Raspberry Pi programmieren
* 'Schilderrahmen' Modell designen 