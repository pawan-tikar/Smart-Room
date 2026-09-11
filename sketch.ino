#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define LDR_PIN 34
#define SERVO_PIN 18
#define LED_PIN 19
#define BUTTON_PIN 27

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
Servo windowServo;

volatile bool buttonPressed = false;
bool autoMode = true;

unsigned long previousMillis = 0;
const unsigned long displayInterval = 1000;

void IRAM_ATTR buttonISR() {
  buttonPressed = true;
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  dht.begin();
  windowServo.attach(SERVO_PIN);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    buttonISR,
    FALLING
  );

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Room");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1500);

  lcd.clear();
}

void loop() {

  // Toggle AUTO/MANUAL mode
  if (buttonPressed) {
    static unsigned long lastPress = 0;

    if (millis() - lastPress > 300) {
      autoMode = !autoMode;
      lastPress = millis();
    }

    buttonPressed = false;
  }

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int light = analogRead(LDR_PIN);

  if (isnan(temp) || isnan(hum)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT ERROR");
    delay(1000);
    return;
  }

  if (autoMode) {

    // Servo Control
    if (temp < 25) {
      windowServo.write(0);
    }
    else if (temp < 30) {
      windowServo.write(90);
    }
    else {
      windowServo.write(180);
    }

    // LED Control
    int brightness = map(light, 4095, 0, 0, 255);
    brightness = constrain(brightness, 0, 255);

    analogWrite(LED_PIN, brightness);
  }

  // LCD Update every second
  if (millis() - previousMillis >= displayInterval) {

    previousMillis = millis();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp, 1);
    lcd.print(" H:");
    lcd.print((int)hum);

    lcd.setCursor(0, 1);

    if (autoMode)
      lcd.print("AUTO ");
    else
      lcd.print("MAN ");

    lcd.print("L:");
    lcd.print(light);

    Serial.print("Temp: ");
    Serial.print(temp);

    Serial.print(" Hum: ");
    Serial.print(hum);

    Serial.print(" Light: ");
    Serial.print(light);

    Serial.print(" Mode: ");

    if (autoMode)
      Serial.println("AUTO");
    else
      Serial.println("MANUAL");
  }
}