#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "*** Enter Wifi SSID ***"
#define WIFI_PASSWORD "*** Enter Wifi Password ***"

#define API_KEY "*** Enter API key ***"
#define DATABASE_URL "*** Enter Firebase database URL ***" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
unsigned long startTime = millis();
int bpm = 0;
float bodyTemperature,roomTemperature;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  dht_sensor.begin();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  particleSensor.enableDIETEMPRDY();
}

void loop()
{
  if (millis() - startTime < 60000) 
  {
    long irValue = particleSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }

      bodyTemperature = particleSensor.readTemperature();
      roomTemperature = dht_sensor.readTemperature();
    }
    
    if (irValue < 50000){
      Serial.print("Finger is not detected!");
    }else{
      Serial.print("Finger scanning...!");
      }
      
    Serial.println();
  }
  else
  {
    bpm = beatAvg;
    Serial.print("BPM: ");
    Serial.print(bpm);
    Serial.print("  Body Temperature: ");
    Serial.print(bodyTemperature,2);
    Serial.print("  Room Temperature: ");
    Serial.println(roomTemperature,2);
    Serial.println("Loop completed for 10 seconds.");

    if (Firebase.ready() && signupOK){
      Firebase.RTDB.setInt(&fbdo, "test/body/BPM",bpm);
      Firebase.RTDB.setFloat(&fbdo, "test/body/Body Temperature",bodyTemperature);
      Firebase.RTDB.setFloat(&fbdo, "test/room/Room Temperature",roomTemperature);
    }
    while (1);
  }
}
