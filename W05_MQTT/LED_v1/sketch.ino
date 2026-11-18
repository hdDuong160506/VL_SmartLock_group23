
#include <WiFi.h>
#include <PubSubClient.h>

// WIFI WOKWI
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT BROKER
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

// MQTT TOPICS
const char* topicStatus =
  "smartlock/group23/duong2026/demo/status";

const char* topicCommand =
  "smartlock/group23/duong2026/demo/cmd";

// LED GIA LAP
const int LED_PIN = 2;

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastSend = 0;

// XU LY LENH NHAN TU NODE-RED
void onMqttMessage(char* topic,
                   byte* payload,
                   unsigned int length) {

  String command = "";

  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }

  command.trim();

  Serial.print("Received command: ");
  Serial.println(command);

  if (command == "SIM_LED_ON") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
    mqtt.publish(topicStatus, "LED_ON_ACK");
  }
  else if (command == "SIM_LED_OFF") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
    mqtt.publish(topicStatus, "LED_OFF_ACK");
  }
  else {
    Serial.println("Unknown command");
  }
}

// KET NOI WIFI
void connectWiFi() {
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
}

// KET NOI MQTT
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");

    String clientId =
      "SmartLock23-" + String(esp_random(), HEX);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT connected!");

      mqtt.subscribe(topicCommand);
      Serial.println("Subscribed to commands");

      mqtt.publish(topicStatus, "ESP32 online");
    }
    else {
      Serial.print("Failed, state=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();

  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(onMqttMessage);

  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();

  if (millis() - lastSend >= 5000) {
    lastSend = millis();

    mqtt.publish(topicStatus, "ESP32 is running");
  }
}
