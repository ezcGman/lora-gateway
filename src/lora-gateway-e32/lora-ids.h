#include <map>

// ########## DEVICE / RECIPIENT IDS ##########
std::map<byte, String> LORA_DEVICE_IDS {
  {0x1, "lora-gateway-e32"}, // 1
  {0xA, "mailbox-sensor"}, // 10
  {0xB, "washing-machine-sensor"}, // 11
  {0xC, "dryer-sensor"} // 12
};

// Special type of "recipient": Broadcast, so for everybody
#define LORA_BROADCAST_ID 0xFF // 255
// ################################

// ########## MESSAGE TYPES ##########
String MESSAGE_DELIMITER = String('|');

struct LoRaBase {
  virtual String toLoRaMessage() = 0;
  virtual void fromLoRaMessage(std::vector<String> splittedMessage) = 0;
  virtual String getMqttTopicName() = 0;
  virtual std::map<String, String> getMqttMessages() = 0;
};

#define LORA_MESSAGE_ID_CUSTOM 0x0 // 0
struct LoRaMessageCustom : LoRaBase {
  String message;

  String toLoRaMessage() {
    return String(message);
  }

  void fromLoRaMessage(std::vector<String> splittedMessage) {
    this->message = splittedMessage.at(0);
  }

  String getMqttTopicName() {
    return "custom";
  }

  std::map<String, String> getMqttMessages() {
    std::map<String, String> mqttMessages;

    mqttMessages["message"] = message;

    return mqttMessages;
  }
};

#define LORA_MESSAGE_ID_MAILBOX 0x1 // 1
struct LoRaMessageMailbox : LoRaBase {
  long duration;
  float distance;
  float humidity;
  float temperature;

  String toLoRaMessage() {
    return String(duration) + MESSAGE_DELIMITER + String(distance) + MESSAGE_DELIMITER + String(humidity) + MESSAGE_DELIMITER + String(temperature);
  }

  void fromLoRaMessage(std::vector<String> splittedMessage) {
    this->duration = splittedMessage.at(0).toInt();
    this->distance = splittedMessage.at(1).toFloat();
    this->humidity = splittedMessage.at(2).toFloat();
    this->temperature = splittedMessage.at(3).toFloat();
  }

  String getMqttTopicName() {
    return "mailbox";
  }

  std::map<String, String> getMqttMessages() {
    std::map<String, String> mqttMessages;

    mqttMessages["duration"] = String(duration);
    mqttMessages["distance"] = String(distance);
    mqttMessages["humidity"] = String(humidity);
    mqttMessages["temperature"] = String(temperature);

    return mqttMessages;
  }
};

#define LORA_MESSAGE_ID_POWER_CONSUMPTION 0x2 // 2
struct LoRaMessagePowerConsumption : LoRaBase {
  float voltage;
  float amps;
  float watts;

  String toLoRaMessage() {
    return String(this->voltage) + MESSAGE_DELIMITER + String(this->amps) + MESSAGE_DELIMITER + String(this->watts);
  }

  void fromLoRaMessage(std::vector<String> splittedMessage) {
    this->voltage = splittedMessage.at(0).toFloat();
    this->amps = splittedMessage.at(1).toFloat();
    this->watts = splittedMessage.at(2).toFloat();
  }

  String getMqttTopicName() {
    return "power-consumption";
  }

  std::map<String, String> getMqttMessages() {
    std::map<String, String> mqttMessages;

    mqttMessages["voltage"] = String(voltage);
    mqttMessages["amps"] = String(amps);
    mqttMessages["watts"] = String(watts);

    return mqttMessages;
  }
};

#define LORA_MESSAGE_ID_TIMESTAMP 0xFE // 254
struct LoRaMessageTimestamp : LoRaBase {
  time_t timestamp;
};
// ###################################

// ########## FUNCTIONS ##########
String getLoRaDeviceNameById(byte deviceId) {
  // C++v20 introduces ::contains() for maps...
  auto search = LORA_DEVICE_IDS.find(deviceId);

  return search != LORA_DEVICE_IDS.end() ? LORA_DEVICE_IDS[deviceId] : "unknown";
}

LoRaBase* mapLoRaMessageToStruct(String message, byte msgId) {
  std::vector<String> splittedMessage = strSplit('|', message);
  LoRaBase *loRaMessage = nullptr;

  switch (msgId) {
    case LORA_MESSAGE_ID_MAILBOX:
    {
      loRaMessage = new LoRaMessageMailbox;
    }

    case LORA_MESSAGE_ID_POWER_CONSUMPTION:
    {
      loRaMessage = new LoRaMessagePowerConsumption;
    }

    case LORA_MESSAGE_ID_TIMESTAMP:
      break;

    case LORA_MESSAGE_ID_CUSTOM:
    default:
    {
      loRaMessage = new LoRaMessageCustom;
    }
  }
  if (loRaMessage != nullptr) {
    loRaMessage->fromLoRaMessage(splittedMessage);
  }

  return loRaMessage;
}

bool sendLoRaMessage(byte messageID, LoRaBase *loRaMessage, byte recipientId = 0, byte senderId = 0) {
  if (recipientId == 0) recipientId = LORA_GATEWAY_ID;
  if (senderId == 0) senderId = LORA_DEVICE_ID;
  String message = loRaMessage->toLoRaMessage();

  LoRa.beginPacket();
  LoRa.write(recipientId); // Set recipient ID
  LoRa.write(senderId); // Set sender ID
  LoRa.write(messageID); // Set message ID
  LoRa.write(message.length()); // Message length
  LoRa.print(message); // The concated message
  LoRa.endPacket();

  return true;
}
// ###################################
