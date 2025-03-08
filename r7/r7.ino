#include <esp_now.h>
#include <WiFi.h>

//car
#define in1 13
#define in2 12
#define in3 14
#define in4 27
///////////////////////////////

//ir sensors
int irr1 = 23;
int irl1 = 21;
//////////////////////
// Sender ESP32 MAC Address (Replace with actual sender MAC)
uint8_t senderMAC[] = {0x5C, 0x01, 0x3B, 0x4D, 0xCA, 0xF0}; // Replace with actual MAC

// Global variable to track the last message received
char lastReceivedMessage[20] = "";
char cmd[999] = "";
// Callback function when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  // Ensure incomingData is properly null-terminated
  char receivedMessage[len + 1];
  memcpy(receivedMessage, incomingData, len);
  receivedMessage[len] = '\0'; // Null-terminate

  // Check if it's a duplicate message to avoid re-processing
  if (strcmp(receivedMessage, lastReceivedMessage) == 0) {
    return; // Ignore if it's the same message
  }

  // Store the message for reference
  strcpy(lastReceivedMessage, receivedMessage);
  strcpy(cmd, receivedMessage);

  // Send acknowledgment back to sender
  char feedback[50];  // Buffer for feedback message
  snprintf(feedback, sizeof(feedback), "Order received from %s", receivedMessage);

  esp_err_t result = esp_now_send(senderMAC, (uint8_t *)feedback, strlen(feedback) + 1);

  if (result == ESP_OK) {
    Serial.println("Order Confirmed");
  } else {
    Serial.println("Order Cancelled");
  }

  // Check received message and respond accordingly
  Serial.print("Received msg : ");
  Serial.println(receivedMessage);

  // 🔹 Reset received message after processing
  memset(lastReceivedMessage, 0, sizeof(lastReceivedMessage));
}

// Movement functions (same as your original code)
void forward()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void backward()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void right()
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void left()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}



void Stop()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
void setup() {
  Serial.begin(115200);
  // Set pin modes
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(irr1,INPUT);
  pinMode(irl1,INPUT);
  WiFi.mode(WIFI_STA); // Set device as a Wi-Fi Station

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv); // Register receive callback

  // Add Sender as a Peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo)); // Initialize struct with zeros
  memcpy(peerInfo.peer_addr, senderMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}                                                                ;
void loop() {
  int irr_1, irl_1;
  irr_1 = digitalRead(irr1);
  irl_1 = digitalRead(irl1);
  Serial.print(irr_1);
  Serial.print(", ");
  Serial.println(irl_1);
  // The receiver will continuously listen for messages and send acknowledgment
  if (strcmp(cmd, "table1") == 0) {
    Serial.println("Placing on table 1");
    if (irl_1 == 0 && irr_1 == 0)
    {
      forward();
    }
    else if (irr_1 == 1 && irl_1 == 0)
    {                                                                                                                                                                                
      right();
    }
    else if (irr_1 == 0 && irl_1 == 1)
    {
      left();
    }
    if (irr_1 == 1 && irl_1 == 1)
    {
      Stop();
    }
  }
  else
  {
    Stop();
  }
}
