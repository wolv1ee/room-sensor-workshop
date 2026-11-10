/*
  Room Sensor Network Workshop - Dev Club WMU

  Reads a DHT22 (temperature + humidity) and a PIR sensor (motion),
  shows the readings on an OLED screen, and publishes them over MQTT.

  BEFORE UPLOADING:
    1. Fill in the WORKSHOP SETTINGS with the values shown in the room.
    2. Put your name in NAME (lowercase, no spaces).
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===================== WORKSHOP SETTINGS =====================
// Copy these from the slide shown in the room.

// Wi-Fi
const bool  USE_ENTERPRISE = false;              // true only for eduroam-style login
const char* WIFI_SSID      = "PASTE_WIFI_NAME";
const char* WIFI_USER      = "";                 // only used if USE_ENTERPRISE is true
const char* WIFI_PASSWORD  = "PASTE_WIFI_PASSWORD";

// MQTT broker
const bool  USE_CLOUD = false;                   // true = HiveMQ Cloud, false = Raspberry Pi
const char* MQTT_HOST = "PASTE_BROKER_ADDRESS";
const char* MQTT_USER = "";                      // only used if USE_CLOUD is true
const char* MQTT_PASS = "";                      // only used if USE_CLOUD is true

// ===================== YOUR NAME =====================
// Lowercase, no spaces. Leave empty to use the board's chip ID.
String NAME = "";

// Heat alert limit in Celsius (Challenge level 1: try lowering this)
const float HEAT_LIMIT_C = 28.0;

// ===================== PINS (already wired) =====================
#define DHT_PIN 4
#define PIR_PIN 27
#define LED_PIN 2
// OLED: SDA = GPIO 21, SCL = GPIO 22

// ===================== OBJECTS =====================
DHT dht(DHT_PIN, DHT22);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
WiFiClient plainNet;
WiFiClientSecure secureNet;
PubSubClient mqtt;

const uint16_t MQTT_PORT = USE_CLOUD ? 8883 : 1883;

String base;                     // becomes room/<name>
float tempC = NAN, hum = NAN;
bool motion = false, lastMotion = false;
bool remoteLed = false;
unsigned long lastRead = 0;

// ===================== SCREEN =====================
void showStatus(const char* msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(NAME);
  display.setCursor(0, 28);
  display.print(msg);
  display.display();
}

// Challenge level 2: change what the screen shows in here
void drawScreen(bool hot) {
  static unsigned long lastDraw = 0;
  if (millis() - lastDraw < 250) return;
  lastDraw = millis();

  display.clearDisplay();

  // Top line: your name
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(NAME);

  // Big temperature
  display.setTextSize(2);
  display.setCursor(0, 18);
  if (isnan(tempC)) {
    display.print("--.- C");
  } else {
    display.print(tempC, 1);
    display.print(" C");
  }

  // Humidity
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Humidity: ");
  if (isnan(hum)) {
    display.print("--");
  } else {
    display.print(hum, 0);
    display.print("%");
  }

  // Status line
  display.setCursor(0, 54);
  if (hot) display.print("HOT! ");
  display.print(motion ? "Motion" : "Still");

  display.display();
}

// ===================== MQTT MESSAGES IN =====================
// Challenge level 3: messages on room/<name>/led turn the LED on or off
void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();
  msg.toLowerCase();
  remoteLed = (msg == "on" || msg == "1" || msg == "true");
}

// ===================== CONNECTIONS =====================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  showStatus("Joining Wi-Fi...");
  WiFi.mode(WIFI_STA);
  if (USE_ENTERPRISE) {
    WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, WIFI_USER, WIFI_USER, WIFI_PASSWORD);
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("Wi-Fi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    showStatus("Connecting to broker...");
    String clientId = "esp32-" + NAME;
    String statusTopic = base + "/status";
    const char* user = USE_CLOUD ? MQTT_USER : NULL;
    const char* pass = USE_CLOUD ? MQTT_PASS : NULL;

    if (mqtt.connect(clientId.c_str(), user, pass, statusTopic.c_str(), 0, true, "offline")) {
      Serial.println("MQTT connected");
      mqtt.publish(statusTopic.c_str(), "online", true);
      mqtt.subscribe((base + "/led").c_str());
    } else {
      Serial.print("MQTT failed, state ");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  dht.begin();
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);

  // Make a name from the chip ID if none was given
  if (NAME.length() == 0) {
    uint64_t mac = ESP.getEfuseMac();
    char id[7];
    snprintf(id, sizeof(id), "%06x", (uint32_t)((mac >> 24) & 0xFFFFFF));
    NAME = "node-" + String(id);
  }
  NAME.trim();
  NAME.toLowerCase();
  NAME.replace(" ", "-");
  base = "room/" + NAME;

  // Stop here if the settings were not filled in
  if (String(WIFI_SSID).startsWith("PASTE") || String(MQTT_HOST).startsWith("PASTE")) {
    showStatus("Fill in settings!");
    Serial.println("Fill in WORKSHOP SETTINGS at the top of the code, then upload again.");
    while (true) delay(1000);
  }

  if (USE_CLOUD) {
    secureNet.setInsecure();     // encrypted, but skips checking the server certificate
    mqtt.setClient(secureNet);
  } else {
    mqtt.setClient(plainNet);
  }
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);

  showStatus("Starting...");
}

// ===================== LOOP =====================
void loop() {
  connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  // Motion: publish only when it changes
  motion = digitalRead(PIR_PIN) == HIGH;
  if (motion != lastMotion) {
    mqtt.publish((base + "/motion").c_str(), motion ? "1" : "0");
    lastMotion = motion;
  }

  // Temperature and humidity every 5 seconds
  if (lastRead == 0 || millis() - lastRead >= 5000) {
    lastRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      tempC = t;
      hum = h;
      mqtt.publish((base + "/temp").c_str(), String(t, 1).c_str());
      mqtt.publish((base + "/hum").c_str(), String(h, 1).c_str());
    }
  }

  // LED: on for motion or a remote command, blinking when too hot
  bool hot = !isnan(tempC) && tempC > HEAT_LIMIT_C;
  bool blinkOn = (millis() / 250) % 2;
  digitalWrite(LED_PIN, (motion || remoteLed || (hot && blinkOn)) ? HIGH : LOW);

  drawScreen(hot);
  delay(50);
}
