// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"
#include "arduinoFFT.h"
#include "TinyGPS++.h"
#include "shakemap-sensor-validation-1_inferencing.h"

#define ENABLE_DEBUG_LOGGING
#define DEBUG_LOG_GPS_LOCATION

#define DATA_BUFFER_SIZE 166
#define DATA_SAMPLING_FREQ 333
#define GPS_SAMPLE_TIME_IN_MS 500

TaskHandle_t Task1;
hw_timer_t *acquireDataTimerConfig = NULL;
hw_timer_t *acquireGpsSignalTimerConfig = NULL;

int16_t buffer1[DATA_BUFFER_SIZE * 3];              // Assuming 3 values per entry (x, y, z)
int16_t buffer2[DATA_BUFFER_SIZE * 3];              // Assuming 3 values per entry (x, y, z)
float featuresBuffer[DATA_BUFFER_SIZE * 3];         // Feature data for AI analysis
volatile boolean buffer1acquisition = true;         // indicates the current buffer being filled by sensor data
volatile int16_t writeIndex = 0;                    // Current index of the data acquisition
volatile boolean acquireDataFlag = false;           // Flag to indicate that data should be acquired
volatile boolean acquireGpsSignalFlag = false;      // Flag to indicate that a GPS signal should be acquired

MPU6050 accelgyro;
TinyGPSPlus gps;

void IRAM_ATTR signalAcquireDataTimer() {
    acquireDataFlag = true;
}

void IRAM_ATTR signalAcquireGpsSignalTimer() {
    acquireGpsSignalFlag = true;
}

void acquireData() {
    if (writeIndex < DATA_BUFFER_SIZE) {
        int16_t *buffer = buffer1acquisition ? buffer1 : buffer2;
        buffer += (3 * writeIndex);
        accelgyro.getAcceleration(buffer, buffer + 1, buffer + 2);
        writeIndex++;
    }
}

void debugPrint(const char *msg) {
#ifdef ENABLE_DEBUG_LOGGING
    Serial.print(msg);
#endif
}

void debugPrintln(const char *msg) {
#ifdef ENABLE_DEBUG_LOGGING
    Serial.println(msg);
#endif
}

void logGpsData() {
    Serial.println("Acquired GPS signal:");
    Serial.print("Sats:");
    Serial.println(gps.satellites.value()); // Number of satellites in use (u32)
    if (gps.altitude.isValid()) {
        if (gps.altitude.isUpdated() || gps.satellites.isUpdated() || gps.course.isUpdated()) {
            Serial.print("Alt: ");
            Serial.print(gps.altitude.meters()); // Altitude in meters (double)
            Serial.println("m");
            Serial.print("Course:");
            Serial.print(gps.course.deg()); // Course in degrees (double)
        }
    } else {
        Serial.println("No valid misc data");
    }

    if (gps.location.isValid()) {
        if (gps.location.isUpdated()) {
            Serial.print("LAT: ");
            Serial.println(gps.location.lat()); // Raw latitude
            Serial.print("LNG: ");
            Serial.println(gps.location.lng()); // Raw longitude
        }
    } else {
        Serial.println("No valid location data");
    }

    if (gps.date.isValid() & gps.date.isValid()) {
        Serial.print(gps.date.day());
        Serial.print(".");
        Serial.print(gps.date.month());
        Serial.print(".");
        Serial.print(gps.date.year());
        Serial.print(" ");
        Serial.print(gps.time.hour() + 1);
        Serial.print(":");
        Serial.println(gps.time.minute());
    } else {
        Serial.println("No valid timestamp data");
    }

    if (gps.speed.isValid()) {
        Serial.print("Speed: ");
        Serial.print(gps.speed.kmph());
        Serial.println("km/h");
    } else {
        Serial.println("No valid speed data");
    }
}

void acquireGpsSignal() {
    debugPrintln("Acquiring GPS signal...");
    while (Serial1.available())
        gps.encode(Serial1.read());
    debugPrintln("GPS signal acquired!");

#ifdef DEBUG_LOG_GPS_LOCATION
    logGpsData();
#endif
}

void Task1code(void *parameter) {
    while (true) {
        if (acquireDataFlag) {
            acquireData();
            acquireDataFlag = false;
        }

        if (acquireGpsSignalFlag) {
            acquireGpsSignal();
            acquireGpsSignalFlag = false;
        }
    }
}

int getFeaturesData(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, featuresBuffer + offset, length * sizeof(float));
    return 0;
}

void doAIMagic(int16_t *buffer) {
    ei_impulse_result_t result = { 0 };

    signal_t features_signal;
    features_signal.total_length = DATA_BUFFER_SIZE * 3;
    features_signal.get_data = &getFeaturesData;
}

void setup() {
// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif

    Serial.begin(250000);

    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // init GPS
    Serial1.begin(9600, SERIAL_8N1, 34, 12);

    // Check if AI data buffer equals it's expected frame size
    while (DATA_BUFFER_SIZE * 3 != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        Serial.println("Data buffer size wrong!");
        delay(500);
    }

    // acquire data timer
    acquireDataTimerConfig = timerBegin(
        0,
        80,
        true
    );  // Timer runs at 80 MHz (prescaler 1), now at 1 MHz with prescaler 80
    timerAttachInterrupt(acquireDataTimerConfig, &signalAcquireDataTimer, true);
    timerAlarmWrite(acquireDataTimerConfig, 1000000 / DATA_SAMPLING_FREQ, true);
    timerAlarmEnable(acquireDataTimerConfig);

    // acquire GPS signal timer
    acquireGpsSignalTimerConfig = timerBegin(
        1,
        80,
        true
    );  // Timer runs at 80 MHz (prescaler 1), now at 1 MHz with prescaler 80
    timerAttachInterrupt(acquireGpsSignalTimerConfig, &signalAcquireGpsSignalTimer, true);
    timerAlarmWrite(acquireGpsSignalTimerConfig, 1000 * GPS_SAMPLE_TIME_IN_MS, true);
    timerAlarmEnable(acquireGpsSignalTimerConfig);

    xTaskCreatePinnedToCore(
            Task1code, /* Function to implement the task */
            "AcquireDataTask",   /* Name of the task */
            10000,     /* Stack size in words */
            NULL,      /* Task input parameter */
            0,         /* Priority of the task */
            &Task1,    /* Task handle. */
            0);        /* Core where the task should run */
}

// Runs FFT on acquired data when ready
void loop() {
    bool dataAquisitionBufferFull = writeIndex == DATA_BUFFER_SIZE;
    // data aquistion complete, switch buffers
    if (dataAquisitionBufferFull) {
        buffer1acquisition = !buffer1acquisition;
        writeIndex = 0;

        doAIMagic(buffer1acquisition ? buffer2 : buffer1);
    }
}
