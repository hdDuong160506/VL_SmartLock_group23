
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ===== WIFI =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== MQTT =====
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

const char* topicStatus =
  "smartlock/group23/duong2026/demo/status";

const char* topicCommand =
  "smartlock/group23/duong2026/demo/cmd";

// ===== SERVO =====
const int SERVO_PIN = 25;
const int LOCK_ANGLE = 0;
const int UNLOCK_ANGLE = 90;

Servo lockServo;

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastSend = 0;

// ===== NHAN LENH TU NODE-RED =====
void onMqttMessage(char* topic,
                   byte* payload,
                   unsigned int length) {
  if (strcmp(topic, topicCommand) != 0) {
    return;
  }

  String command = "";

  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }

  command.trim();

  Serial.print("Received command: ");
  Serial.println(command);

  if (command == "UNLOCK") {
    lockServo.write(UNLOCK_ANGLE);

    Serial.println("Servo -> 90 degrees");
    mqtt.publish(topicStatus, "UNLOCK_ACK");
  }
  else if (command == "LOCK") {
    lockServo.write(LOCK_ANGLE);

    Serial.println("Servo -> 0 degrees");
    mqtt.publish(topicStatus, "LOCK_ACK");
  }
  else {
    Serial.println("Unknown command");
  }
}

// ===== KET NOI WIFI =====
void connectWiFi() {
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
}

// ===== KET NOI MQTT =====
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

// ===== KHOI TAO =====
void setup() {
  Serial.begin(115200);

  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);

  // Vi tri khoa gia lap ban dau
  lockServo.write(LOCK_ANGLE);

  connectWiFi();

  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(onMqttMessage);

  connectMQTT();
}

// ===== VONG LAP =====
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
