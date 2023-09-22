// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"
#include "arduinoFFT.h"

TaskHandle_t Task1;
hw_timer_t *Timer0_Cfg = NULL;

#define BUFFER_SIZE 1024
int16_t buffer1[BUFFER_SIZE * 3];            // Assuming 3 values per entry (x, y, z)
int16_t buffer2[BUFFER_SIZE * 3];            // Assuming 3 values per entry (x, y, z)
volatile boolean buffer1acquisition = true;  // indicates the current buffer being filled by sensor data
volatile int16_t writeIndex = 0;             // Current index of the data acquisition
volatile boolean aquireDataFlag = false;     // Flag to indicate that data should be acquired

MPU6050 accelgyro;

// FFT
arduinoFFT FFT = arduinoFFT();         /* Create FFT object */
const uint16_t samples = BUFFER_SIZE;  // This value MUST ALWAYS be a power of 2
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

    Timer0_Cfg = timerBegin(0, 80, true);  // Timer runs at 80 MHz (prescaler 1), now at 1 MHz with prescaler 80
    timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
    timerAlarmWrite(Timer0_Cfg, 1000000 / samplingFrequency, true);
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
    bool dataAquisitionBufferFull = writeIndex == BUFFER_SIZE;
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
