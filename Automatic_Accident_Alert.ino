#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

Adafruit_MPU6050 mpu;

// SIM800 TX -> Arduino D8, SIM800 RX -> Arduino D7
#define SIM800_TX_PIN 8
#define SIM800_RX_PIN 7

// GPS TX -> Arduino D4, GPS RX -> Arduino D3
#define GPS_TX_PIN 4
#define GPS_RX_PIN 3

// Buzzer & Button Pins
#define BUZZER_PIN 6
#define BUTTON_PIN 10

SoftwareSerial serialSIM800(SIM800_TX_PIN, SIM800_RX_PIN);
SoftwareSerial gpsSerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPSPlus gps;

// Accident Detection Thresholds
float accelThreshold = 15.0;  // Adjust for impact detection
float gyroThreshold = 10.0;  // Adjust for rotation detection

bool accidentDetected = false;
unsigned long lastAccidentTime = 0;
const unsigned long cooldownTime = 5000;  // 5 seconds cooldown

double latitude, longitude;

void setup() {
  Serial.begin(9600);
  serialSIM800.begin(9600);
  gpsSerial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button uses internal pull-up

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  }

  Serial.println("MPU6050 initialized!");
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Checking SIM800...");
  serialSIM800.println("AT");
  delay(1000);
  printResponse();

  serialSIM800.println("AT+CMGF=1");  // Set SMS mode to text
  delay(1000);
  printResponse();

  Serial.println("Waiting for GPS signal...");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Read Accelerometer (Impact Detection)
  float accelMagnitude = abs(accel.acceleration.x) + abs(accel.acceleration.y) + abs(accel.acceleration.z);

  // Read Gyroscope (Rotation Speed)
  float gyroMagnitude = abs(gyro.gyro.x) + abs(gyro.gyro.y) + abs(gyro.gyro.z);

  Serial.print("Accel: "); Serial.print(accelMagnitude);
  Serial.print(" | Gyro: "); Serial.print(gyroMagnitude);
  Serial.println();

  // 🚨 Detect Accident: If impact OR rotation is above the threshold
  if ((accelMagnitude > accelThreshold || gyroMagnitude > gyroThreshold) && !accidentDetected) {
    Serial.println("🚨 Accident Detected! 🚨");
    digitalWrite(BUZZER_PIN, HIGH);
     unsigned long startTime = millis();
     while (millis() - startTime < 5000) {      
      if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("✅ Alert Canceled by User");
        digitalWrite(BUZZER_PIN, LOW);
        return;
      }
    }
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("⏳ No Response. Proceeding with Alert...");
    accidentDetected = true;
    getGps();
    sendLocationSMS(latitude, longitude);
    lastAccidentTime = millis();
  }

  // Cooldown to avoid multiple alerts
  if (accidentDetected && millis() - lastAccidentTime > cooldownTime) {
    accidentDetected = false;
  }

  delay(200);
}


// 📡 Get GPS Data
void getGps() {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (gpsSerial.available()) {
      if (gps.encode(gpsSerial.read())) {
        newData = true;
        break;
      }
    }
  }

  if (newData) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  } else {
    Serial.println("No GPS data available");
    latitude = 0;
    longitude = 0;
  }

  Serial.print("Latitude: ");
  Serial.println(latitude, 6);
  Serial.print("Longitude: ");
  Serial.println(longitude, 6);
}

// 📩 Send SMS with GPS Location
void sendLocationSMS(double lat, double lon) {
  Serial.println("Sending SMS...");

  char latStr[12], lonStr[12];
  dtostrf(lat, 9, 6, latStr);
  dtostrf(lon, 9, 6, lonStr);

  char smsContent[100];
  snprintf(smsContent, sizeof(smsContent), "Accident Detected! Location: https://www.google.com/maps?q=%s,%s", latStr, lonStr);

  serialSIM800.println("AT+CMGS=\"YOUR_PHONE_NUMBER\"");
  delay(3000);
  printResponse();

  serialSIM800.println(smsContent);
  delay(1000);
  printResponse();

  serialSIM800.write(26);  // Ctrl+Z to send SMS
  delay(1000);
  printResponse();

  Serial.println("📨 SMS Sent!");
}

// 🛠 Print SIM800 Response for Debugging
void printResponse() {
  while (serialSIM800.available()) {
    Serial.write(serialSIM800.read());
  }
}
