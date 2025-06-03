#define BLYNK_TEMPLATE_ID "TMPL6rpSQB2A9"
#define BLYNK_TEMPLATE_NAME "duc"

#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include <BlynkSimpleEsp8266.h>
#include "DHTesp.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi credentials
char auth[] = "oT3tJ0WVxO9Emazx-kOCg8jiH72yfoSZ";
char ssid[] = "TOTOLINK_N350RT";
char pass[] = "123456780";

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT Sensor Configuration
DHTesp dht;
unsigned long timeShowOled = millis();
float temperature, humidity;

// Blynk Timer
BlynkTimer timer;
WidgetLED LEDCONNECT(V0);

// Virtual Pins
#define NHIETDO V1
#define DOAM V2
#define RELAY1_VPIN V3
#define RELAY2_VPIN V4

// Relay Configuration
#define RELAY1_PIN 14 // GPIO14 (D5)
#define RELAY2_PIN 12 // GPIO12 (D6)

// Button Configuration
#define BUTTON1_PIN 0 // GPIO0 (D3)
#define BUTTON2_PIN 2 // GPIO2 (D4)

bool relay1State = false; // Trạng thái relay 1
bool relay2State = false; // Trạng thái relay 2

void setup()
{
  Serial.begin(115200);
  delay(100);
  Blynk.begin(auth, ssid, pass);

  // Relay configuration
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH); // Tắt relay 1 ban đầu
  digitalWrite(RELAY2_PIN, HIGH); // Tắt relay 2 ban đầu

  // Button configuration
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // OLED configuration
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("OLED allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  delay(2000);

  // DHT configuration
  dht.setup(16, DHTesp::DHT11);

  // Timer update
  timer.setInterval(1000L, updateBlynk);
}

void loop() {
  Blynk.run();
  timer.run();

  // Cập nhật trạng thái relay dựa vào nút nhấn bên ngoài
  handleButtonPress();

  if (millis() - timeShowOled > dht.getMinimumSamplingPeriod()) {
    float t = dht.getTemperature();
    float h = dht.getHumidity();
    if (dht.getStatusString() == "OK") {
      temperature = t;
      humidity = h;
      showOled(temperature, humidity);
    }
    timeShowOled = millis();
  }
}

// Hàm xử lý nút nhấn bên ngoài
// Cập nhật nút nhấn bên ngoài (ngược trạng thái logic của relay)
void handleButtonPress() {
  // Nút nhấn 1
  if (digitalRead(BUTTON1_PIN) == LOW) {
    delay(50); // Chống dội phím
    if (digitalRead(BUTTON1_PIN) == LOW) {
      relay1State = !relay1State; // Đảo trạng thái relay 1
      digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW); // Chỉnh lại logic cho relay
      Blynk.virtualWrite(RELAY1_VPIN, relay1State);      // Cập nhật trạng thái lên Blynk
      while (digitalRead(BUTTON1_PIN) == LOW);           // Đợi nút được nhả
    }
  }
  // Nút nhấn 2
  if (digitalRead(BUTTON2_PIN) == LOW) {
    delay(50); // Chống dội phím
    if (digitalRead(BUTTON2_PIN) == LOW) {
      relay2State = !relay2State; // Đảo trạng thái relay 2
      digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW); // Chỉnh lại logic cho relay
      Blynk.virtualWrite(RELAY2_VPIN, relay2State);      // Cập nhật trạng thái lên Blynk
      while (digitalRead(BUTTON2_PIN) == LOW);           // Đợi nút được nhả
    }
  }
}

// Update Blynk
void updateBlynk() {
  if (LEDCONNECT.getValue()) LEDCONNECT.off();
  else LEDCONNECT.on();
  Blynk.virtualWrite(NHIETDO, temperature);
  Blynk.virtualWrite(DOAM, humidity);
}

// Display data on OLED
void showOled(float t, float h) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("T: ");
  display.print(t, 1);
  display.print((char)247);
  display.println("C");
  display.print("H: ");
  display.print(h, 0);
  display.println("%");
  display.display();
}

// Điều khiển relay qua Blynk
BLYNK_WRITE(RELAY1_VPIN) {
  relay1State = param.asInt(); // Lấy trạng thái từ Blynk
  digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW); // Chỉnh lại logic
}

BLYNK_WRITE(RELAY2_VPIN) {
  relay2State = param.asInt(); // Lấy trạng thái từ Blynk
  digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW); // Chỉnh lại logic
}
