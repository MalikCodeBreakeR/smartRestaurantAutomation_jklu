#include <esp_now.h>
#include <WiFi.h>
//OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
///////////////////////////////

#define NUM_TABLES 4

//oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C  // Default I2C address for SSD1306

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/////////////////////////////////

const int tbtn[NUM_TABLES] = {13, 12, 14, 27};
const int kbtn[NUM_TABLES] = {26, 25, 33, 32};

bool orderRequested[NUM_TABLES] = {false, false, false, false};

uint8_t receiverMAC[] = {0xC8, 0xC9, 0xA3, 0xFB, 0x19, 0x30};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Message Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void orderTable(const char *str);
void show(String str, int x, int y, int s, bool clear = false);

int ct = 0;
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  //oled
  Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  show("Inializing...", 10, 25, 2);
  /////////////////////////////////////
  for (int i = 0; i < NUM_TABLES; i++) {
    pinMode(tbtn[i], INPUT_PULLUP);
    pinMode(kbtn[i], INPUT_PULLUP);
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed!");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  show("Menu", 62, 25, 1);
}

void loop() {
  static int lastState = -1;  // Store the last state to avoid redundant updates

  for (int i = 0; i < NUM_TABLES; i++) {
    int currentState = 0;

    // Order requested
    if (digitalRead(tbtn[i]) == HIGH && digitalRead(kbtn[i]) == LOW && orderRequested[i] == false) {
      ct = 0;
      Serial.print("Order request from Table ");
      Serial.println(i + 1);

      show("Order request from Table " + String(i + 1), 10, 25, 1, true);

      orderRequested[i] = true;
      delay(200);
      currentState = 1;
    }
    // Confirm order
    else if (digitalRead(tbtn[i]) == HIGH && digitalRead(kbtn[i]) == HIGH && orderRequested[i] == true) {
      ct = 0;
      Serial.print("Order confirmed for Table ");
      Serial.println(i + 1);

      show("Order confirmed for Table " + String(i + 1), 10, 25, 1, true);

      char tableMsg[10];
      snprintf(tableMsg, sizeof(tableMsg), "table%d", i + 1);
      orderTable(tableMsg);
      delay(500);
      orderRequested[i] = false;
      currentState = 2;
    }
    // Sending robot back
    else if (digitalRead(tbtn[i]) == LOW && digitalRead(kbtn[i]) == HIGH && orderRequested[i] == false) {
      Serial.println("Sending robot back to home...");
      if (lastState != 3) {  // Update only if it's a new state
        show("Sending robot back to home...", 10, 25, 1, true);
        orderTable("home");
        lastState = 3;
      }
      delay(500);
      currentState = 3;
    }
    // Ready robot
    else if (digitalRead(tbtn[i]) == LOW && digitalRead(kbtn[i]) == LOW) {
      Serial.println("Robot ready");
      ct++;
      //  if (lastState != 4) {  // Update only if it's a new state
      if (ct == 1) {
        show("Menu:", 10, 5, 2, true);  // Clear before showing menu title
        show("1. Burger - $5", 10, 25, 1);
        show("2. Pizza  - $8", 10, 35, 1);
        show("3. Pasta  - $7", 10, 45, 1);
        show("4. Soda   - $2", 10, 55, 1);
        lastState = 4;
      }
      currentState = 4;
    }


    // Only update `lastState` if it has changed
    if (lastState != currentState) {
      lastState = currentState;
    }
  }
}


void orderTable(const char *str) {
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)str, strlen(str) + 1);
  Serial.println(result == ESP_OK ? "Order Sent Successfully" : "Error Sending Order");
}

//oled
void show(String str, int x, int y, int s, bool clear) {
  if (clear) {
    display.clearDisplay();  // Only clear when needed
  }
  display.setTextSize(s);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(str);
  display.display();
}

///////////////////////////////////////////
