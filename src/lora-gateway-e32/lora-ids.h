#include <map>

// ########## DEVICE IDS ##########
std::map<byte, String> LORA_DEVICE_IDS {
  {0x1, "lora-gateway-e32"}, // 1
  {0xA, "mailbox-sensor"} // 10
};

String getLoRaDeviceNameById(byte deviceId) {
  // C++v20 introduces ::contains() for maps...
  auto search = LORA_DEVICE_IDS.find(deviceId);

  return search != LORA_DEVICE_IDS.end() ? LORA_DEVICE_IDS[deviceId] : "unknown";
}
// ################################

#define LORA_BROADCAST_ID 0xFF // 255

// ########## MESSAGE TYPES ##########
struct LoRaBase {
  virtual String getMqttTopicName() = 0;
  virtual std::map<String, String> getMqttMessages() = 0;
};

#define LORA_MESSAGE_ID_CUSTOM 0x0 // 0
struct LoRaMessageCustom : LoRaBase {
  String message;

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

#define LORA_MESSAGE_ID_TIMESTAMP 0xFE // 254
struct LoRaMessageTimestamp : LoRaBase {
  time_t timestamp;
};


LoRaBase* mapLoRaMessageToStruct(String message, byte msgId) {
  switch (msgId) {
    case LORA_MESSAGE_ID_MAILBOX:
    {
      std::vector<String> splittedMessage = strSplit('|', message);
      LoRaMessageMailbox *mailboxMessage = new LoRaMessageMailbox;

      mailboxMessage->duration = splittedMessage.at(0).toInt();
      mailboxMessage->distance = splittedMessage.at(1).toFloat();
      mailboxMessage->humidity = splittedMessage.at(2).toFloat();
      mailboxMessage->temperature = splittedMessage.at(3).toFloat();

      return mailboxMessage;
    }

    case LORA_MESSAGE_ID_TIMESTAMP:
      break;

    case LORA_MESSAGE_ID_CUSTOM:
    default:
    {
      LoRaMessageCustom *customMessage = new LoRaMessageCustom;
      customMessage->message = message;

      return customMessage;
    }
  }
}
// ###################################
