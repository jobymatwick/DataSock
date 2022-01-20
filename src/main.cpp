#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define LED_PIN 13
static const char* accel_labels[] = { "2", "4", "8", "16" };
static const char* gyro_labels[] = { "250", "500", "1000", "2000" };
static const char* filter_labels[] = { "260", "184", "94", "44", "21", "10", "5" };

Adafruit_MPU6050 mpu;
bool sensor_connected = false;

void setup()
{
    // initialize the digital pin as an output.
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);
    Serial.println("Starting up!");

    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050!");
    }
    else
    {
        sensor_connected = true;
        Serial.println("Connected to MPU6050!");

        mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        Serial.print("Accelerometer range set to: +/- ");
        Serial.print(accel_labels[mpu.getAccelerometerRange()]);
        Serial.println(" G");

        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        Serial.print("Gyro range set to: +/- ");
        Serial.print(gyro_labels[mpu.getGyroRange()]);
        Serial.println(" deg/s");

        mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        Serial.print("Filter bandwidth set to: ");
        Serial.print(filter_labels[mpu.getFilterBandwidth()]);
        Serial.println(" Hz");
    }
}

void loop()
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    if (sensor_connected)
    {
        /* Get new sensor events with the readings */
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        /* Print out the values */
        Serial.print("Acceleration X: ");
        Serial.print(a.acceleration.x);
        Serial.print(", Y: ");
        Serial.print(a.acceleration.y);
        Serial.print(", Z: ");
        Serial.print(a.acceleration.z);
        Serial.println(" m/s^2");

        Serial.print("Rotation X: ");
        Serial.print(g.gyro.x);
        Serial.print(", Y: ");
        Serial.print(g.gyro.y);
        Serial.print(", Z: ");
        Serial.print(g.gyro.z);
        Serial.println(" rad/s");

        Serial.print("Temperature: ");
        Serial.print(temp.temperature);
        Serial.println(" degC");
    }
    else
    {
        Serial.println("Sensor not connected...");
    }

    delay(500);
}
