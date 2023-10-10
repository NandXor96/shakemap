#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "I2Cdev.h"

// #include "MPU6050.h"
#include "MPU6050_6Axis_MotionApps20.h"

#define BOARD2  // BOARD1 or BOARD2

#ifdef BOARD1
#define RECVR_MAC \
    { 0xE0, 0xE2, 0xE6, 0x70, 0x43, 0xB4 }  // replace with your board's address
// #define BLINK_ON_SEND
// #define BLINK_ON_SEND_SUCCESS
#define BLINK_ON_RECV
#else
#define RECVR_MAC \
    { 0x24, 0x0A, 0xC4, 0x60, 0xB4, 0x38 }  // replace with your board's address
// #define BLINK_ON_SEND
#define BLINK_ON_SEND_SUCCESS
// #define BLINK_ON_RECV
#endif

#define WIFI_CHAN 13  // 12-13 only legal in US in lower power mode, do not use 14
#define BAUD_RATE 115200
#define TX_PIN 1               // default UART0 is pin 1 (shared by USB)
#define RX_PIN 3               // default UART0 is pin 3 (shared by USB)
#define SER_PARAMS SERIAL_8N1  // SERIAL_8N1: start/stop bits, no parity

#define BUFFER_SIZE 250  // max of 250 bytes
// #define DEBUG            // for additional serial messages (may interfere with other messages)

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

MPU6050 mpu;
int16_t x_offset, y_offset, z_offset;
#define AQUISITION_RATE 333  // Hz

void setup(void) {
    delay(10000);  // initial delay to close the case

    pinMode(LED_BUILTIN, OUTPUT);

    // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(400000);  // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif

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

    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    mpu.setRate(1);  // 1khz / (1 + 1) = 500 Hz

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788);  // 1688 factory default for my test chip

    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();

    // initial calibration to get gravity vector
    mpu.getAcceleration(&x_offset, &y_offset, &z_offset);

    Serial.println("Reading events");
    delay(100);
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

uint32_t last_loop = micros();
void loop() {
    if (micros() - last_loop < 1E6 / AQUISITION_RATE) {
        return;
    }
    last_loop = micros();

    int16_t x, y, z;
    mpu.getAcceleration(&x, &y, &z);

    x -= x_offset;
    y -= y_offset;
    z -= z_offset;

    // Serial.print("Acceration - x: ");
    Serial.print(x);
    Serial.print(",");
    // Serial.print("y:")
    Serial.print(y);
    Serial.print(",");
    // Serial.print("z:")
    Serial.println(z);

    int charsWritten = snprintf((char *)buf_send, BUFFER_SIZE, "%d,%d,%d\n",
                                x,
                                y,
                                z);

    if (charsWritten >= BUFFER_SIZE || charsWritten < 0) {
        // Handle error (buffer too small or other error occurred)
        Serial.println("snprintf error");
        buf_size = 0;
    } else {
        buf_size = charsWritten;
        send_buffer_esp_now();
    }
}
