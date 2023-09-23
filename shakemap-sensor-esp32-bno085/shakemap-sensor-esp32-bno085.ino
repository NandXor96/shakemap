// Basic demo for readings from Adafruit BNO08x
#include <Adafruit_BNO08x.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>


#define BOARD2  // BOARD1 or BOARD2

#ifdef BOARD1
#define RECVR_MAC \
  { 0xE0, 0xE2, 0xE6, 0x70, 0x43, 0xB4 }  // replace with your board's address
//#define BLINK_ON_SEND
//#define BLINK_ON_SEND_SUCCESS
#define BLINK_ON_RECV
#else
#define RECVR_MAC \
  { 0x24, 0x0A, 0xC4, 0x60, 0xB4, 0x38 }  // replace with your board's address
//#define BLINK_ON_SEND
#define BLINK_ON_SEND_SUCCESS
//#define BLINK_ON_RECV
#endif

#define WIFI_CHAN 13  // 12-13 only legal in US in lower power mode, do not use 14
#define BAUD_RATE 115200
#define TX_PIN 1               // default UART0 is pin 1 (shared by USB)
#define RX_PIN 3               // default UART0 is pin 3 (shared by USB)
#define SER_PARAMS SERIAL_8N1  // SERIAL_8N1: start/stop bits, no parity

#define BUFFER_SIZE 250  // max of 250 bytes
//#define DEBUG // for additional serial messages (may interfere with other messages)

#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // some boards don't have an LED or have it connected elsewhere
#endif

const uint8_t broadcastAddress[] = RECVR_MAC;
// wait for double the time between bytes at this serial baud rate before sending a packet
// this *should* allow for complete packet forming when using packetized serial comms
const uint32_t timeout_micros = (int)(1.0 / BAUD_RATE * 1E6) * 20;

uint8_t buf_recv[BUFFER_SIZE];
uint8_t buf_send[BUFFER_SIZE];
uint8_t buf_size = 0;
uint32_t send_timeout = 0;

esp_now_peer_info_t peerInfo;  // scope workaround for arduino-esp32 v2.0.1

#if defined(DEBUG) || defined(BLINK_ON_SEND_SUCCESS)
uint8_t led_status = 0;
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#ifdef DEBUG
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Send success");
  } else {
    Serial.println("Send failed");
  }
#endif

#ifdef BLINK_ON_SEND_SUCCESS
  if (status == ESP_NOW_SEND_SUCCESS) {
    led_status = ~led_status;
    // this function happens too fast to register a blink
    // instead, we latch on/off as data is successfully sent
    digitalWrite(LED_BUILTIN, led_status);
    return;
  }
  // turn off the LED if send fails
  led_status = 0;
  digitalWrite(LED_BUILTIN, led_status);
#endif
}
#endif

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
#ifdef BLINK_ON_RECV
  digitalWrite(LED_BUILTIN, HIGH);
#endif
  memcpy(&buf_recv, incomingData, sizeof(buf_recv));
  Serial.write(buf_recv, len);
#ifdef BLINK_ON_RECV
  digitalWrite(LED_BUILTIN, LOW);
#endif
#ifdef DEBUG
  Serial.print("\n Bytes received: ");
  Serial.println(len);
#endif
}

// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9

// For SPI mode, we also need a RESET
//#define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1


Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

void setup(void) {
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(BAUD_RATE, SER_PARAMS, RX_PIN, TX_PIN);
  Serial.println(send_timeout);
  WiFi.mode(WIFI_STA);

#ifdef DEBUG
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
#endif

  if (esp_wifi_set_channel(WIFI_CHAN, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
#ifdef DEBUG
    Serial.println("Error changing WiFi channel");
#endif
    return;
  }

  if (esp_now_init() != ESP_OK) {
#ifdef DEBUG
    Serial.println("Error initializing ESP-NOW");
#endif
    return;
  }

#if defined(DEBUG) || defined(BLINK_ON_SEND_SUCCESS)
  esp_now_register_send_cb(OnDataSent);
#endif

  // esp_now_peer_info_t peerInfo;  // scope workaround for arduino-esp32 v2.0.1
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = WIFI_CHAN;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
#ifdef DEBUG
    Serial.println("Failed to add peer");
#endif
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);


  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
    //if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte UART buffer!
    //if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
  Serial.println("BNO08x Found!");

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");

  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
    Serial.println("Could not enable linear acceleration");
  }
}


void send_buffer_esp_now() {
#ifdef BLINK_ON_SEND
  digitalWrite(LED_BUILTIN, HIGH);
#endif
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&buf_send, buf_size);
  buf_size = 0;
#ifdef DEBUG
  if (result == ESP_OK) {
    Serial.println("Sent!");
  } else {
    Serial.println("Send error");
  }
#endif
#ifdef BLINK_ON_SEND
  digitalWrite(LED_BUILTIN, LOW);
#endif
}

void loop() {
  delay(10);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  switch (sensorValue.sensorId) {

    case SH2_LINEAR_ACCELERATION:
      // Serial.print("Linear Acceration - x: ");
      // Serial.print("x:");
      Serial.print(sensorValue.un.linearAcceleration.x);
      Serial.print(",");
      // Serial.print("y:")
      Serial.print(sensorValue.un.linearAcceleration.y);
      Serial.print(",");
      // Serial.print("z:")
      Serial.println(sensorValue.un.linearAcceleration.z);



      int charsWritten = snprintf((char*)buf_send, BUFFER_SIZE, "%f,%f,%f\n",
                                  sensorValue.un.linearAcceleration.x,
                                  sensorValue.un.linearAcceleration.y,
                                  sensorValue.un.linearAcceleration.z);

      if (charsWritten >= BUFFER_SIZE || charsWritten < 0) {
        // Handle error (buffer too small or other error occurred)
        Serial.println("snprintf error");
        buf_size = 0;
      } else {
        buf_size = charsWritten;
        send_buffer_esp_now();
      }


      break;
  }
}