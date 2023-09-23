// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"
#include "arduinoFFT.h"
#include "TinyGPS++.h"
#include "shakemap-sensor-validation-1_inferencing.h"

#define DATA_BUFFER_SIZE 1024
#define GPS_SAMPLE_TIME_IN_MS 500

TaskHandle_t Task1;
hw_timer_t *acquireDataTimerConfig = NULL;
hw_timer_t *acquireGpsSignalTimerConfig = NULL;

int16_t buffer1[DATA_BUFFER_SIZE * 3];            // Assuming 3 values per entry (x, y, z)
int16_t buffer2[DATA_BUFFER_SIZE * 3];            // Assuming 3 values per entry (x, y, z)
volatile boolean buffer1acquisition = true;  // indicates the current buffer being filled by sensor data
volatile int16_t writeIndex = 0;             // Current index of the data acquisition
volatile boolean acquireDataFlag = false;    // Flag to indicate that data should be acquired
volatile boolean acquireGpsSignalFlag = false;   // Flag to indicate that a GPS signal should be acquired

MPU6050 accelgyro;
TinyGPSPlus gps;

// FFT
arduinoFFT FFT = arduinoFFT();         /* Create FFT object */
const uint16_t samples = DATA_BUFFER_SIZE;  // This value MUST ALWAYS be a power of 2
const double samplingFrequency = 512;  // Hz

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void IRAM_ATTR signalAcquireDataTimer() {
    acquireDataFlag = true;
}

void IRAM_ATTR signalAcquireGpsSignalTimer() {
    acquireGpsSignalFlag = true;
}

void acquireData() {
    if (writeIndex < DATA_BUFFER_SIZE) {
        // get pointer to the current address in the correct buffer
        int16_t *buffer = buffer1acquisition ? buffer1 : buffer2;
        // get pointer to the current address in the other buffer
        buffer += (3 * writeIndex);
        int16_t ax, ay, az;
        accelgyro.getAcceleration(buffer, buffer + 1, buffer + 2);
        writeIndex++;
    }
}

void acquireGpsSignal() {
    Serial.println("Acquiring GPS signal...");
    while (Serial1.available())
        gps.encode(Serial1.read());
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

void Task1code(void *parameter) {
    while (true) {
        if (acquireDataFlag) {
            acquireData();
            acquireDataFlag = false;
        }

        if (acquireGpsSignalFlag) {
            acquireGpsSignal();
            acquireGpsSignalFlag = false;
            Serial.print("Frame size: ");
            Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
            Serial.print("Label count: ");
            Serial.println(EI_CLASSIFIER_LABEL_COUNT);
        }
    }
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType) {
    for (uint16_t i = 0; i < bufferSize; i++) {
        double abscissa;
        /* Print abscissa value */
        switch (scaleType) {
            case SCL_INDEX:
                abscissa = (i * 1.0);
                break;
            case SCL_TIME:
                abscissa = ((i * 1.0) / samplingFrequency);
                break;
            case SCL_FREQUENCY:
                abscissa = ((i * 1.0 * samplingFrequency) / samples);
                break;
        }
        Serial.print(abscissa, 6);
        if (scaleType == SCL_FREQUENCY)
            Serial.print("Hz");
        Serial.print(" ");
        Serial.println(vData[i], 4);
    }
    Serial.println();
}

void fft(int16_t *buffer) {
    for (uint16_t i = 0; i < samples; i++) {
        // double norm = sqrt(pow(buffer[3 * samples], 2) + pow(buffer[3 * samples + 1], 2) + pow(buffer[3 * samples + 2], 2));

        double value = 0;
        value += buffer[3 * i + 2];
        vReal[i] = value;
        vImag[i] = 0.0;  // Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
    }

    FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency); /* Create FFT object */
    /* Print the results of the simulated sampling according to time */
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data */
    // Serial.println("Weighed data:");
    // PrintVector(vReal, samples, SCL_TIME);
    FFT.Compute(FFT_FORWARD); /* Compute FFT */
    // Serial.println("Computed Real values:");
    // PrintVector(vReal, samples, SCL_INDEX);
    // Serial.println("Computed Imaginary values:");
    // PrintVector(vImag, samples, SCL_INDEX);
    FFT.ComplexToMagnitude(); /* Compute magnitudes */
    Serial.println("Computed magnitudes:");
    PrintVector(vReal, 20, SCL_FREQUENCY);

    double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
    Serial.print("Major peak at frequency: ");
    Serial.println(x, 6);
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

    // acquire data timer
    acquireDataTimerConfig = timerBegin(0, 80,
                                        true);  // Timer runs at 80 MHz (prescaler 1), now at 1 MHz with prescaler 80
    timerAttachInterrupt(acquireDataTimerConfig, &signalAcquireDataTimer, true);
    timerAlarmWrite(acquireDataTimerConfig, 1000000 / samplingFrequency, true);
    timerAlarmEnable(acquireDataTimerConfig);

    // acquire GPS signal timer
    acquireGpsSignalTimerConfig = timerBegin(1, 80,
                                             true);  // Timer runs at 80 MHz (prescaler 1), now at 1 MHz with prescaler 80
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

uint32_t last_loop_time = 0;

// Runs FFT on acquired data when ready
void loop() {
    bool dataAquisitionBufferFull = writeIndex == DATA_BUFFER_SIZE;
    if (dataAquisitionBufferFull) {
        // data aquistion complete, switch buffers
        buffer1acquisition = !buffer1acquisition;
        writeIndex = 0;

        // run fft once
        // Serial.print("FFT on buffer");
        // Serial.println(buffer1acquisition ? "2" : "1");
        int16_t *fftBuffer = buffer1acquisition ? buffer2 : buffer1;
        fft(fftBuffer);

        // Serial.print("Active Aquisition Buffer:");
        // Serial.print(buffer1acquisition ? "1" : "2");
        // Serial.println();
    }

    // Serial.print("Active Buffer:");
    // Serial.print(buffer1acquisition ? "1" : "2");
    // Serial.print(",WriteIndex:");
    // Serial.print(writeIndex);

    // // print latest data
    // if (writeIndex > 0) {
    //     int16_t *activeBuffer = buffer1acquisition ? buffer1 : buffer2;
    //     Serial.print(",ax:");
    //     Serial.print(activeBuffer[(writeIndex * 3) - 3]);
    //     Serial.print(",ay:");
    //     Serial.print(activeBuffer[(writeIndex * 3) - 2]);
    //     Serial.print(",az:");
    //     Serial.print(activeBuffer[(writeIndex * 3) - 1]);
    //     Serial.print(",norm:");
    //     Serial.print(sqrt(pow(activeBuffer[(writeIndex * 3) - 3], 2) + pow(activeBuffer[(writeIndex * 3) - 2], 2) + pow(activeBuffer[(writeIndex * 3) - 1], 2)));
    // }

    // // Calculate loop time and frequency
    // uint32_t loop_time = micros() - last_loop_time;
    // last_loop_time = micros();
    // float loop_freq = 1000000.0 / loop_time;
    // Serial.print(",loopFreq:");
    // Serial.print(loop_freq);

    // Serial.println("");
    Serial.flush();
}
