//Include the library files
#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6hjmkKv9I"
#define BLYNK_TEMPLATE_NAME "AUPLINGS"
#define BLYNK_AUTH_TOKEN "I2pl7s5wr5pRZn2ECvI_p2drscVUL9Sn"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>



//char auth[] = "I2pl7s5wr5pRZn2ECvI_p2drscVUL9Sn"; // Enter your Auth token
char ssid[] = "OPPO A12";//Enter your WIFI name
char pass[] = "10101010";//Enter your WIFI password


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
BlynkTimer timer;
bool Relay = false;
bool pumpOn = false;
bool buttonPressed = false;
unsigned long buttonPressStartTime = 0;
const unsigned long buttonPressDuration = 900000; // 15 minutes in milliseconds

String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Define component pins
#define sensor1 A0
#define sensor2 D1
#define sensor3 D2
#define sensor4 D5
#define sensor5 D6
#define sensor6 D7
#define waterPump D3
#define buttonPin D0

void setup() {
  Serial.begin(115200);
  pinMode(waterPump, OUTPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT);
  pinMode(sensor5, INPUT);
  pinMode(sensor6, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Use an internal pull-up resistor for the button

  digitalWrite(waterPump, HIGH);

  //Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
 // Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  // Call the function
  timer.setInterval(100L, soilMoistureSensor);
}

void loop() {
  Blynk.run(); // Run the Blynk library
  timer.run(); // Run the Blynk timer

  // Check if it's time to activate the pump based on humidity and time of day
  if (isTimeToRunPump() && getHumidity(sensor1) < 30) {
    digitalWrite(waterPump, LOW); // Turn on the pump
    pumpOn = true;
    Serial.println("Pump is turned on.");
  } else if (pumpOn && (!isTimeToRunPump() || getHumidity(sensor1) >= 30)) {
    digitalWrite(waterPump, HIGH); // Turn off the pump
    pumpOn = false;
    Serial.println("Pump is turned off.");
  }


  // Check if the button is pressed and limit its operation to 15 minutes
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressStartTime = millis();
    digitalWrite(waterPump, LOW); // Turn on the pump
    Serial.println("Button is pressed, pump is turned on.");
  }

  if (buttonPressed && millis() - buttonPressStartTime >= buttonPressDuration) {
    buttonPressed = false;
    digitalWrite(waterPump, HIGH); // Turn off the pump after 15 minutes
    Serial.println("Button operation time is over, pump is turned off.");
  }
}

// Get the button value
BLYNK_WRITE(buttonPin) {
  Relay = param.asInt();
}

//Get the button value
BLYNK_WRITE(V1) {
  Relay = param.asInt();

  if (Relay == 1) {
    digitalWrite(waterPump, LOW);
  } else {
    digitalWrite(waterPump, HIGH);
  }
}

// Get the soil moisture values
void soilMoistureSensor() {
  int value1 = getHumidity(sensor1);
  Blynk.virtualWrite(V0, value1);

  int value2 = getHumidity(sensor2);
  Blynk.virtualWrite(V2, value2);

  int value3 = getHumidity(sensor3);
  Blynk.virtualWrite(V3, value3);

  int value4 = getHumidity(sensor4);
  Blynk.virtualWrite(V4, value4);

  int value5 = getHumidity(sensor5);
  Blynk.virtualWrite(V5, value5);

  int value6 = getHumidity(sensor6);
  Blynk.virtualWrite(V6, value6);
}

// Get humidity value from the analog sensor
int getHumidity(int sensorPin) {
  int value = analogRead(sensorPin);
  value = map(value, 0, 1024, 0, 100);
  return (value - 100) * -1;
}

// Check if it's time to run the pump based on your specified time frame
bool isTimeToRunPump() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  return ((currentHour >= 18 || currentHour < 6) && isWeekday());
}

// Check if it's a weekday (Monday to Friday)
bool isWeekday() {
  int currentDay = timeClient.getDay();
  return (currentDay >= 1 && currentDay <= 7);
}