#include <SPI.h>
#include <LoRa.h>

#include <WiFi.h>
#include <ETH.h>
// Using https://pubsubclient.knolleary.net/
#include <PubSubClient.h>
// Using https://github.com/mcxiaoke/ESPDateTime
#include <DateTime.h>

#include "utils.h"
#include "config.h"
#include "lora-config.h"

#define PIN_LORA_NSS 15
#define PIN_LORA_RST 16
#define PIN_LORA_DIO0 18
#define PIN_LORA_TXEN 2
#define PIN_LORA_RXEN 4
#define PIN_JMPR_ETH 32
#define PIN_JMPR_WIFI 33
#define PIN_ETH_ENABLE 5

byte lastKnownActiveJumper = -1;
int counter = 0;
unsigned long lastHealthCheck = 0;
unsigned long lastTimestampBroadcast = 0;
unsigned int timestampBroadcastInterval = 10000; // Milliseconds
bool loraInitDone = false;
bool mqttInitDone = false;
bool dtInitDone = false;
SPIClass spiE32(HSPI);

// Used for Wi-Fi and MQTT
const byte maxConnTries = 15;

String mqttBaseTopic = "lora-gateway-e32";
String mqttStatusBaseTopic = mqttBaseTopic + "/status";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);  


void connectMqtt(bool sleepOnMqttUnreachable = false) {
  if (!mqttClient.connected()) {
    int mqttConnTries = 0;
    while (!mqttClient.connected() && mqttConnTries < maxConnTries) {
      mqttConnTries++;
      Serial.printf("Connecting to MQTT server at %s on port %d ...\n", mqttServer, mqttPort);

      if (!mqttClient.connect(getLoRaDeviceNameById(LORA_DEVICE_ID).c_str(), mqttUser, mqttPassword)) {
        Serial.print("Failed with state ");
        Serial.println(mqttClient.state());
        delay(1000);
      }
    }

    if (!mqttClient.connected() && mqttConnTries >= maxConnTries) {
      Serial.print("Maximum number of MQTT connection attempts reached. ");

      Serial.print("Rebooting...\n");
      ESP.restart();
    }

    mqttInitDone = true;

    Serial.println("Connected to MQTT server!");
  }
}

void connectWifi(bool sleepOnWiFiUnreachable = false) {
  turnOffEthernet();

  if (WiFi.status() != WL_CONNECTED) {
    int wifiConnTries = 0;

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword, wifiChannel);
    while (WiFi.status() != WL_CONNECTED && wifiConnTries < maxConnTries) {
      wifiConnTries++;
      Serial.printf("Connecting to Wi-Fi %s ...\n", wifiSsid);
      delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED && wifiConnTries >= maxConnTries) {
      Serial.print("Maximum number of Wi-Fi connection attempts reached. ");

      Serial.print("Rebooting...\n");
      ESP.restart();
    }
  }
}

void turnOffWifi() {
  Serial.println("Turning off Wi-Fi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  byte waitTries = 0;
  // Give it a max of 30 seconds to turn off, after we just accept it didn't work...
  while (WiFi.status() != WL_NO_SHIELD && WiFi.status() != WL_DISCONNECTED && waitTries++ <= 30) {
    delay(1000);
  }
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet connected");
      
      ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      ETH.setHostname(getLoRaDeviceNameById(LORA_DEVICE_ID).c_str());

      break;

    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.printf("Ethernet link up! IP address: %s\n", ETH.localIP().toString().c_str());
    case SYSTEM_EVENT_STA_GOT_IP:
      if (event == SYSTEM_EVENT_STA_GOT_IP) Serial.printf("Connected to Wi-Fi! IP address: %s\n", WiFi.localIP().toString().c_str());

      if (!dtInitDone) {
        setupDateTime();
      }

      if (!mqttInitDone) {
        connectMqtt(true);
      }

      break;

    case SYSTEM_EVENT_ETH_DISCONNECTED:
    case SYSTEM_EVENT_STA_STOP:
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.print((event == SYSTEM_EVENT_ETH_DISCONNECTED ? "Ethernet" : "Wi-Fi"));
      Serial.println(" disconnected");

      if (mqttInitDone) {
        mqttClient.disconnect();
        mqttInitDone = false;
      }

      break;
  }
}

void connectEthernet(bool sleepOnEthDown = false) {
  turnOffWifi();

  Serial.println("Turning on Ethernet...");

  ETH.begin(
    (uint8_t) 0, 
    (int)     5, 
    (int)     23, 
    (int)     18, 
    (eth_phy_type_t)   ETH_PHY_LAN8720,
    (eth_clock_mode_t) ETH_CLOCK_GPIO17_OUT
  );
}

void turnOffEthernet() {
  Serial.println("Turning off Ethernet...");

  if (digitalRead(PIN_ETH_ENABLE) != LOW) {
    // ETH lib doesn't have any disconnect or down or sth similar method...
    digitalWrite(PIN_ETH_ENABLE, LOW);

    byte waitTries = 0;
    // Give it a max of 30 seconds to turn off, after we just accept it didn't work...
    while (ETH.linkUp() && waitTries++ <= 30) {
      delay(1000);
    }
  }
}

void connectNetwork() {
  if (lastKnownActiveJumper == PIN_JMPR_ETH) {
    connectEthernet(true);
  }
  else {
    connectWifi(true);
  }  
}

void checkEthWiFiJumperAndApply() {
  if (digitalRead(PIN_JMPR_ETH) == LOW && lastKnownActiveJumper != PIN_JMPR_ETH) {
    lastKnownActiveJumper = PIN_JMPR_ETH;
    connectNetwork();
  }
  else if (digitalRead(PIN_JMPR_WIFI) == LOW && lastKnownActiveJumper != PIN_JMPR_WIFI) {
    lastKnownActiveJumper = PIN_JMPR_WIFI;
    connectNetwork();
  }
}

bool isNetworkUp() {
  if (lastKnownActiveJumper == PIN_JMPR_ETH && ETH.linkUp() && ETH.localIP().toString() != "0.0.0.0") {
    return true;
  }
  else if (lastKnownActiveJumper == PIN_JMPR_WIFI && WiFi.status() == WL_CONNECTED) {
    return true;
  }

  return false;
}

bool setupDateTime() {
  Serial.println("Setting up DateTime...");
  DateTime.setServer("pool.ntp.org");
  DateTime.begin();
  String mqttInitStatusBaseTopic = mqttStatusBaseTopic + "/dateTimeInitStatus";

  int dateTimeTries = 0;
  while (!DateTime.isTimeValid() && dateTimeTries < 10) {
     dateTimeTries++;
     Serial.println("Failed to get time from server. Retrying...");
     delay(3000);
  }

  if (!DateTime.isTimeValid()) {
    if (isNetworkUp()) mqttClient.publish(mqttInitStatusBaseTopic.c_str(), "failed", true);
    return false;
  }

  dtInitDone = true;

  if (isNetworkUp()) mqttClient.publish(mqttInitStatusBaseTopic.c_str(), "success", true);

  return true;
}

void connectLoRa() {
  if (!loraInitDone) {
    int loraConnTries = 0;
    String mqttInitStatusBaseTopic = mqttStatusBaseTopic + "/loRaInitStatus";

    LoRa.setSPI(spiE32);
    LoRa.setPins(PIN_LORA_NSS, PIN_LORA_RST, PIN_LORA_DIO0);

    while (!LoRa.begin(868E6) && loraConnTries < maxConnTries) {
      loraConnTries++;
      delay(1000);
    }
  
    if (loraConnTries >= maxConnTries) {
      Serial.println("Starting LoRa failed!");
      if (isNetworkUp()) mqttClient.publish(mqttInitStatusBaseTopic.c_str(), "failed", true);
      delay(1000);

      ESP.restart();
    }

    Serial.println("Starting LoRa success!");
    if (isNetworkUp()) mqttClient.publish(mqttInitStatusBaseTopic.c_str(), "success", true);

    loraInitDone = true;

    LoRa.setSyncWord(LORA_SYNC_WORD);
  }
}

void onLoRaReceive(int packetSize) {
  if (!loraInitDone || packetSize == 0) return;          // if there's no packet, return

  Serial.println("Incoming message!");

  // read packet header bytes:
  byte recipientId = LoRa.read();          // Read recipient ID
  byte senderId = LoRa.read();             // Read sender ID
  byte msgId = LoRa.read();                // Read message ID
  byte messageLength = LoRa.read();        // incoming msg length
  String message = "";                     // payload of packet

  while (LoRa.available()) {               // can't use readString() in callback, so
    message += (char)LoRa.read();          // add bytes one by one
  }

  if (message.length() != messageLength) { // check length for error
    Serial.println("error: message length does not match length");
    return;                                // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipientId != LORA_DEVICE_ID && recipientId != LORA_BROADCAST_ID) {
    Serial.println("This message is not for me.");
    return;                                // skip rest of function
  }

  LoRaBase* messageStruct = mapLoRaMessageToStruct(message, msgId);
  std::map<String, String> mqttMessagesMap = messageStruct->getMqttMessages();
  std::map<String, String>::iterator mqttMessage = mqttMessagesMap.begin();

  String mqttMessageBaseTopic = mqttBaseTopic + "/devices/" + getLoRaDeviceNameById(senderId) + "/messages/" + messageStruct->getMqttTopicName() + "/";
  for (auto mqttMessage : mqttMessagesMap) {
    if (isNetworkUp()) mqttClient.publish((mqttMessageBaseTopic + mqttMessage.first).c_str(), mqttMessage.second.c_str(), true);
  }

  // if message is for this device, or broadcast, print details:
  #ifdef LORA_GATEWAY_DEBUG
  Serial.println("Received from: 0x" + String(senderId, HEX));
  Serial.println("Sent to: 0x" + String(recipientId, HEX));
  Serial.println("Message ID: " + String(msgId));
  Serial.println("Message length: " + String(messageLength));
  Serial.println("Message: " + message);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  #endif
}

void sendTimestampBroadcast() {
  if (!loraInitDone || !dtInitDone) return;

  Serial.println("Sending timestamp broadcast...");

  loRaSetTransmittingMode();
  delay(100);

  String message = String(DateTime.getTime());
  Serial.println("Current timestamp: " + message);
  LoRa.beginPacket();
  LoRa.write(LORA_BROADCAST_ID); // Set recipient ID
  LoRa.write(LORA_DEVICE_ID); // Set sender ID
  LoRa.write(LORA_MESSAGE_ID_TIMESTAMP); // Set message ID to LORA_MESSAGE_ID_TIMESTAMP
  LoRa.write(message.length());
  LoRa.print(message);
  LoRa.endPacket();

  delay(100);
  loRaSetReceivingMode();
  delay(100);
}

void loRaSetRfMode(bool receiving = true) {
  digitalWrite(PIN_LORA_RXEN, (receiving ? HIGH : LOW));
  digitalWrite(PIN_LORA_TXEN, (receiving ? LOW : HIGH));
}
void loRaSetReceivingMode() { loRaSetRfMode(true); }
void loRaSetTransmittingMode() { loRaSetRfMode(false); }

void sendLastUpdatedAndUptime() {
  String strDateTime = DateTime.format("%Y-%m-%dT%H:%M:%S%z");
  char charDateTime[strDateTime.length()];
  strDateTime.toCharArray(charDateTime, strDateTime.length());
  if (isNetworkUp()) mqttClient.publish((mqttStatusBaseTopic + "/lastUpdated").c_str(), charDateTime, true);

  char charUptime[10];
  sprintf(charUptime, "%d", millis() / 1000);
  if (isNetworkUp()) mqttClient.publish((mqttStatusBaseTopic + "/uptimeInSeconds").c_str(), charUptime, true);

  lastHealthCheck = millis();
}

 
void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Switch the module to receiving mode
  pinMode(PIN_LORA_TXEN, OUTPUT);
  pinMode(PIN_LORA_RXEN, OUTPUT);
  pinMode(PIN_JMPR_ETH, INPUT_PULLUP);
  pinMode(PIN_JMPR_WIFI, INPUT_PULLUP);
  pinMode(PIN_ETH_ENABLE, OUTPUT);

  digitalWrite(PIN_ETH_ENABLE, LOW);

  mqttClient.setServer(mqttServer, mqttPort);

  WiFi.onEvent(WiFiEvent);
  checkEthWiFiJumperAndApply();

  sendLastUpdatedAndUptime();

  loRaSetReceivingMode();
  connectLoRa();

  Serial.println("LoRa Gateway ready!");
}

void loop() {
  if (loraInitDone) {
    onLoRaReceive(LoRa.parsePacket());

    if (millis() - lastTimestampBroadcast > timestampBroadcastInterval) {
      sendTimestampBroadcast();
      lastTimestampBroadcast = millis();
    }
  }

  if (millis() - lastHealthCheck > 5000) {
    checkEthWiFiJumperAndApply();

    if (isNetworkUp()) sendLastUpdatedAndUptime();

    // Updated in sendLastUpdatedAndUptime()
    // lastHealthCheck = millis();
  }
}
                                                                           
