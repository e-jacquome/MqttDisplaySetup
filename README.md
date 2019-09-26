# MQTT ESP32 E-paper Türschild
Ein ESP32 gesteuertes Epaper Display das durch eingehende MQTT Nachrichten die Anzeige ändert.

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
4. IP Adresse des Pis herausfinden. z.B. über Routerinterface oder ein Tool wie Nmap. Wird für die nächsten Schritte benötigt. 
5. Anderen Computer im selben Netzwerk verwenden und mit dem Terminal durch `ssh pi@IPADRESSE`SSH Verbindung aufbauen. Standard Password: `raspberry.
6. Installieren des Mosquitto Server und Clients durch `sudo apt install mosquitto mosquitto-clients`. Nach der Installation wird automatisch ein Mosquitto Server gestartet.

### ESP32
1. Installieren aller benötigten Bibliotheken mithilfe der Bibliotheksverwaltung. Liste siehe oben.
2. Downloaden/Entpacken des Arduino Codes.
3. Installieren des ESP32 Boardtreibers:
    * In den Voreinstellungen unter zusätzliche Boardverwalter `https://dl.espressif.com/dl/package_esp32_index.json` eintragen.
    * Dann unter Werkzeuge > Boardverwalter nach esp32 suchen und installieren.
4. Werkzeuge > Uploadspeed auf 115200 setzen.
5. Den ESP32 anschließen, den richtigen Port unter Werkzeuge einstellen und den Sketch hochladen. Nach dem Kompilieren muss auf dem ESP evtl. die Boot Taste gedrückt werden um das Hochladen zu ermöglichen.
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

