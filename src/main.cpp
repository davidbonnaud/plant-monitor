#include <Arduino.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// WiFi connection parameters
const char* ssid = "The Homestead";
const char* password = "2001hondansx";

// Telegram Bot Authorization Token (Get from Botfather)
#define botToken "6290422799:AAHFBzo_hfEOIOJf0HaWyXjd3EANhspGI84"
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
long checkTelegramDueTime;
int checkTelegramDelay = 1000;

// People to send messages to
#define CHAT_ID "6094819483"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

// Initialize Sensor
#define sensorPin A0
long moistureSensorDueTime;
int moistureSensorDelay = 5000;
int soilMoistureValue = 0;

void handleNewMessages(int numNewMessages);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    client.setTrustAnchors(&cert);
  #endif

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  long now = millis();
  if (now >= checkTelegramDueTime) {
    Serial.println("----- Checking Telegram -----");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    Serial.println(bot.last_message_received);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    checkTelegramDueTime = now + checkTelegramDelay;
  }
  
  now = millis();
  if (now >= moistureSensorDueTime) {
    Serial.println("----- Checking Moisture Sensor -----");
    soilMoistureValue = analogRead(sensorPin);
    Serial.println(soilMoistureValue);
    if (soilMoistureValue > 0) {
      String s = "Soil Moisture Normal";
      bot.sendMessage(CHAT_ID, s, "");
    }
    
    moistureSensorDueTime = now + moistureSensorDelay;
  }
  
}

void handleNewMessages(int numNewMessages) {
  Serial.println(String(numNewMessages));
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";
    if (text == "/start") {
      String welcome = "Welcome " + from_name + " to the Soil Moisture Bot!";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/moisture") {
      String moisture = "The soil moisture is " + String(soilMoistureValue);
      bot.sendMessage(chat_id, moisture, "");
    }
    else {
      String reply = "Sorry " + from_name + ", I don't understand that command.";
      bot.sendMessage(chat_id, reply, "");
    }
  }
}