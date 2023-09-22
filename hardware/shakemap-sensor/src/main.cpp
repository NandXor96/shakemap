// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

TaskHandle_t Task1;
hw_timer_t *Timer0_Cfg = NULL;

#define BUFFER_SIZE 1000
int16_t buffer1[BUFFER_SIZE * 3];            // Assuming 3 values per entry (x, y, z)
int16_t buffer2[BUFFER_SIZE * 3];            // Assuming 3 values per entry (x, y, z)
volatile boolean buffer1acquisition = true;  // indicates the current buffer being filled by sensor data
volatile int16_t writeIndex = 0;             // Current index of the data acquisition
volatile boolean aquireDataFlag = false;     // Flag to indicate that data should be acquired

MPU6050 accelgyro;

// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

void IRAM_ATTR Timer0_ISR() {
    aquireDataFlag = true;
}

void aquireData() {
    if (writeIndex < BUFFER_SIZE) {
        // get pointer to the current address in the correct buffer
        int16_t *buffer = buffer1acquisition ? buffer1 : buffer2;
        // get pointer to the current address in the other buffer
        buffer += (3 * writeIndex);
        int16_t ax, ay, az;
        accelgyro.getAcceleration(buffer, buffer + 1, buffer + 2);
        writeIndex++;
    }
}

void Task1code(void *parameter) {
    while (true) {
        if (aquireDataFlag) {
            aquireData();
            aquireDataFlag = false;
        }
    }
}

void setup() {
// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif

    Serial.begin(115200);

    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    Timer0_Cfg = timerBegin(0, 80, true);  // Timer runs at 80 MHz (prescaler 1), now at 1 MHz with prescaler 80
    timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
    timerAlarmWrite(Timer0_Cfg, 50000, true);  // 50 ms
    timerAlarmEnable(Timer0_Cfg);

    xTaskCreatePinnedToCore(
        Task1code, /* Function to implement the task */
        "Task1",   /* Name of the task */
        10000,     /* Stack size in words */
        NULL,      /* Task input parameter */
        0,         /* Priority of the task */
        &Task1,    /* Task handle. */
        0);        /* Core where the task should run */
}

uint32_t last_loop_time = 0;
void loop() {
    Serial.print("Active Buffer:");
    Serial.print(buffer1acquisition ? "1" : "2");
    Serial.print(",WriteIndex:");
    Serial.print(writeIndex);

    // print latest data
    if (writeIndex > 0) {
        int16_t *activeBuffer = buffer1acquisition ? buffer1 : buffer2;
        Serial.print(",ax:");
        Serial.print(activeBuffer[(writeIndex * 3) - 3]);
        Serial.print(",ay:");
        Serial.print(activeBuffer[(writeIndex * 3) - 2]);
        Serial.print(",az:");
        Serial.print(activeBuffer[(writeIndex * 3) - 1]);
    }

    // Calculate loop time and frequency
    uint32_t loop_time = micros() - last_loop_time;
    last_loop_time = micros();
    float loop_freq = 1000000.0 / loop_time;
    Serial.print(",loopFreq:");
    Serial.print(loop_freq);

    Serial.println("");
    Serial.flush();
}
