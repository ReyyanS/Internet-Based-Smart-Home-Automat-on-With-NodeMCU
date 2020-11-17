
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SimpleTimer.h>;
#include <BlynkSimpleEsp8266.h>;
#include <dht11.h>
#include <Servo.h>
#include <MQ2.h>
# define PIN_HUMIDITY V0
#define PIN_TEMPATURE V1
#define PIN_FAHRENHEIT V2
#define PIN_DEW V3
int rain = 0;
int dhtPin = D0;
int servo = D1;
int buzzer = D2;
int pirPin = D3;
int smoke = A0;
// Distance Sensor
int echoPin = D5;
int trigPin = D6;
char auth[] = "8ced87edeedd492b9b964e5050d4938b";
const char * ssid = "FF";
const char * password = "eafj6127";
dht11 DHT11;
Servo Servo1;
MQ2 mq2(smoke);
HTTPClient http;
SimpleTimer timer;
void ConnectWifi() {
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}
void setup() {
  pinMode(dhtPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Servo1.attach(servo);
  Servo1.write(0);
  Serial.begin(9600);
  Serial.println();
  ConnectWifi();
  Blynk.begin(auth, ssid, password);
  timer.setInterval(1000L, SendSensorData);
  mq2.begin();
}
void loop() {
  Dht();
  int rain = Rain();
  if (rain == 1 || rain == 0) Servo(90);
  else Servo(0);
  if(Distance() <= 10) {
    digitalWrite(buzzer, HIGH);
    delay(3000);
  }
  else digitalWrite(buzzer, LOW);
  if(HasMovement()) {
    Serial.println("\nHareket algilandi!\n");
    Blynk.notify("Hareket algilandi!");
    Blynk.email("bozd4g@gmail.com", "Uyarı: Hareket algilandi!", "Evinizde bir hareket var!");
  }
  else Serial.println("Hareket yok.");
  Blynk.run();
  timer.run();
  delay(1000);
}
void SendSensorData() {
  Blynk.virtualWrite(PIN_HUMIDITY, (int) DHT11.humidity);
  Blynk.virtualWrite(PIN_TEMPATURE, (int) DHT11.temperature);
  Blynk.virtualWrite(PIN_FAHRENHEIT, (int) DHT11.fahrenheit());
  Blynk.virtualWrite(PIN_DEW, (float) DHT11.dewPoint());
}
void Dht() {
  int chk = DHT11.read(dhtPin);
  // Sensörden gelen verileri serial monitörde yazdırıyoruz.
  Serial.print("\nNem (%): ");
  Serial.println((float) DHT11.humidity, 2);
  Serial.print("Sicaklik (Celcius): ");
  Serial.println((float) DHT11.temperature, 2);
  Serial.print("Sicaklik (Fahrenheit): ");
  Serial.println(DHT11.fahrenheit(), 2);
  Serial.print("Sicaklik (Kelvin): ");
  Serial.println(DHT11.kelvin(), 2);
  // Çiğ Oluşma Noktası, Dew Point
  Serial.print("Cig Olusma Noktasi: ");
  Serial.println(DHT11.dewPoint(), 2);
  Serial.println();
}

long Distance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  long resTime = pulseIn(echoPin, HIGH); /* ses dalgasinin geri dönmesi için geçen sure ölçülüyor */
  long distance = (resTime / 2) / 29.1; /* ölçülen sure uzakliga çevriliyor */
  Serial.print("Uzaklik ");
  Serial.print(distance);
  Serial.println(" cm.");
  if (distance < 200)
    return distance;
  return 0.0;
}
int Rain() {
  int sensorReading = analogRead(rain);
  int range = map(sensorReading, 0, 1024, 0, 3); // 0 - 1014 --> min-max
  switch (range) {
  case 0: // Sensor getting wet
    Serial.println("Yagmur yagiyor!");
    break;
  case 1: // Sensor getting wet
    Serial.println("Yagmur uyarisi!");
    break;
  case 2: // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
    Serial.println("Yagmur yagmiyor.");
    break;
  }
  delay(1); // delay between reads
  return range;
}
void Servo(int angle) {
  Servo1.write(angle);
}
boolean HasMovement() {
  return digitalRead(pirPin);
}
int IsAnyGas() {
  float lpg = mq2.readLPG();
  float co = mq2.readCO();
  float smk = mq2.readSmoke();
  Serial.println();
  Serial.print("LPG: ");
  Serial.println((float)lpg, 2);
  Serial.print("CO: ");
  Serial.println(co, 2);
  Serial.print("Smoke: ");
  Serial.println(smoke, 2);
  Serial.println();
  if(lpg > 10000 || co > 10000 || smk > 10000)
    return -1;
 if(lpg > 50 || co > 500 || smk > 200)
    return 1;
  else return -1;
}
