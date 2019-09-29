 
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <GxEPD.h>
#include <GxGDEW027W3/GxGDEW027W3.h>  // Library for the 2.7" b/w epaper display
#include <Fonts/FreeMonoBold9pt7b.h>  //FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h> //library for using spi pins to send data to epaper display
#include <GxIO/GxIO.h>
#include "bitmaps.h"  //external file containing the bitmaps

//NOT FULLY WORKING
//Definitions for the deep sleep timer
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  20        /* Time ESP32 will go to sleep (in seconds) */ // 600 for 10 minutes
RTC_DATA_ATTR int bootCount = 0;

//Sets SPI pin definitions (Do not change if using the pins connections from the readme)
GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4


//IMPORTANT: Do change these values according to your network
//Defining the wifi network the mqtt broker is in.
#define WIFI_SSID "XXXXXX"
#define WIFI_PASSWORD "XXXXXX"

//Defining the MQTT-Broker location (enter IP of the broker)
#define MQTT_HOST IPAddress(192, 168, 0, 153)
#define MQTT_PORT 1883

AsyncMqttClient mqttClient;

// Function that prints the wakeup reason after esp woke up from deep sleep
void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeupReason;
  wakeupReason = esp_sleep_get_wakeup_cause();

  switch(wakeupReason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeupReason); break;
  }
}

// Function that uses the given credentials to connect to the wifi network the broker is in
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// Function that tries to connect to mqtt broker
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

// Function that prints information about different wifi events
void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
      // If wifi connected and ip assigned, print connected and the ip, then tries to connect to mqtt
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
        
        // If wifi disconnected, print disconnected
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        break;
    }
}

// Function that executes if connected to mqtt
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  //subscribing to the update channel to listen for doorsign updates
  //Change this if you want to change the name of the channels you recieve updates on
  uint16_t packetIdSub = mqttClient.subscribe("doorsign/update", 2);
  Serial.print("Subscribing to doorsign/update at QoS 2, packetId: ");
  Serial.println(packetIdSub);

  //publish to status channel that esp is connected and subscribed
  uint16_t packetIdPub1 = mqttClient.publish("doorsign/status", 2, true, "Connected.");
  Serial.println("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub1);
}

//Function that executes when you are disconnected from mqtt broker
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    connectToMqtt();
  } else {
    connectToWifi();
  }
}

//Function that executes when you subscribe to a channel
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

//Function that executes when you unsubscribe from a channel
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

//Function that executes when you recieve a message at a subscribed channel
//First the message properties are printed to the serial monitor, then a reaction function is called.
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);

  //Publishes a message to the doorsign/status topic with the payload message
  String statusUpdateStr = "Recieved update: ";
  statusUpdateStr.concat(payload);
  char statusUpdateChar[50];
  statusUpdateStr.toCharArray(statusUpdateChar, 50);
  mqttClient.publish("doorsign/status", 2, true, statusUpdateChar);

  //calling the reaction function which changes the display according to the payload message
  reactToPayload(payload);
}

//Function that executes when a message is published
void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

//Method for calling the correct printing method depending on the payload. After reacting the esp will go to deep sleep.
void reactToPayload(char* payload) {
  String payloadMessage;
  uint8_t length = strlen(payload);
  for (int i = 0; i < length; i++){
    payloadMessage += (char)payload[i];
  }
  Serial.println("[PAYLOAD]: ");
  Serial.println(payloadMessage);

  if (payloadMessage.startsWith("DA")) {
    Serial.println("Printing profDa...");
    printProfDa();
  } else if (payloadMessage.startsWith("WEG")) {
    Serial.println("Printing profNichtDa...");
    printProfNichtDa();
  } else if (payloadMessage.startsWith("M")) {
    Serial.println("Printing profAtLocation");
    printProfAtLocation(payloadMessage);
  } else {
    Serial.println("Printing message...");
    printMessage(payloadMessage);
  }
  Serial.println();
  activateDeepSleep();
}

//Displaying the predefined bitmap when the professor is at university
void printProfDa(){
  display.setRotation(45);
  display.fillScreen(GxEPD_WHITE);
  display.update();
  display.drawBitmap(0, 0, profDa, 264, 176, GxEPD_BLACK);
  display.update();
  delay(5000);
}

//Displaying the predefined bitmap when the prof is not at university
void printProfNichtDa(){
  display.setRotation(45); 
  display.fillScreen(GxEPD_WHITE);
  display.update();
  display.drawBitmap(0, 0, profNichtDa, 264, 176, GxEPD_BLACK);
  display.update();
  delay(5000);
}

//Printing that the professor is a specified room in the M-building 
//IMPORTANT: The payload must have the format: Mxxx where xxx is the room number.
void printProfAtLocation(String payloadMessage){
  String location;
  for (int i = 0; i < 4; i++){
    location += (char)payloadMessage[i];
  }
  initializeFont();
  display.println("Der Professor ist gerade in Raum :");
  display.println(location);
  display.update();
  delay(5000);
}

//Function to write a custom message on the epaper display
void printMessage(String payloadMessage) {
  initializeFont();
  String message;
  //Creates a substring of the payloadMessage begining at the start and cutting after a '.'
  //This is used to cut of random bytes and characters from the mqtt transmition
  for (int i=0; i < payloadMessage.length(); i++) {
    if (payloadMessage.charAt(i) == '.') {
      message = payloadMessage.substring(0, i);
    }
  }
  display.println(message);
  display.update(); 
  delay(5000);
}

//Initializing, clearing, setting the font and rotation of the display to write a basic text.
void initializeFont() {
  //Defining which font to use (e.g. &FreeMonoBold9pt7b, &FreeMonoBold12pt7b, &FreeMonoBold18pt7b)
  const GFXfont* f = &FreeMonoBold12pt7b;
  display.setRotation(45);
  display.fillScreen(GxEPD_WHITE);
  display.update();
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
}

void activateDeepSleep() {
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  delay(1000);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  printWakeupReason();
  
  
  //First we configure the wake up source. We set the ESP32 to wake up every 10 minutes
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  
  //Initialise epaper display 
  display.init(115200); //enable diagnostic output on serial

  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();

  //After 30 seconds of being 'awake' with no message, the esp will go to deep sleep.
  delay(30000);
  activateDeepSleep();
}

void loop() {
  // put your main code here, to run repeatedly:
}
