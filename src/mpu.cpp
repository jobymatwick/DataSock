/* 
 * File:    mpu.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Functions to interface with the MPU 6050 6-axis IMU
 */

#include "mpu.h"

#include <string.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define ACCEL_RANGE MPU6050_RANGE_8_G
#define GYRO_RANGE  MPU6050_RANGE_500_DEG
#define FILTER_BW   MPU6050_BAND_21_HZ

Adafruit_MPU6050 mpu;
bool             connected = false;

bool mpu_init()
{
    // Reset if already connected
    if (connected)
    {
        mpu.reset();
    }

    if (!mpu.begin())
    {
        connected = false;
        return connected;
    }

    connected = true;
    connected = mpu_configure(ACCEL_RANGE, GYRO_RANGE, FILTER_BW);

    return connected;
}

bool mpu_configure(mpu6050_accel_range_t accel, mpu6050_gyro_range_t gyro, mpu6050_bandwidth_t filter)
{
    if (!connected)
        return false;

    mpu.setAccelerometerRange(accel);
    mpu.setGyroRange(gyro);
    mpu.setFilterBandwidth(filter);

    // Confirm settings applied correctly
    if (mpu.getAccelerometerRange() != accel ||
        mpu.getGyroRange() != gyro ||
        mpu.getFilterBandwidth() != filter)
        connected = false;

    return connected;
}

bool mpu_sample(float accel[3], float gyro[3], float* temp)
{
    sensors_event_t a, g, t;

    if (!connected)
        return false;

    if (!mpu.getEvent(&a, &g, &t))
    {
        connected = false;
        return connected;
    }

    accel[0] = a.acceleration.x;
    accel[1] = a.acceleration.y;
    accel[2] = a.acceleration.z;

    gyro[0] = g.gyro.x;
    gyro[1] = g.gyro.y;
    gyro[2] = g.gyro.z;

    *temp = t.temperature;

    return connected;
}

bool mpu_console(char* args)
{
    if (!strcmp("get", args))
    {
        float a[3], g[3], t;
        if (!mpu_sample(a, g, &t))
        {
            Serial.println("Error getting sensor data!");
            return false;
        }

        Serial.printf("A: [%6.2f, %6.2f, %6.2f], ", a[0], a[1], a[2]);
        Serial.printf("G: [%6.2f, %6.2f, %6.2f], ", g[0], g[1], g[2]);
        Serial.printf("T: %5.2f\r\n", t);

        return true;
    }

    if (!strcmp("init", args))
    {
        if (!mpu_init())
        {
            Serial.println("Error initializing sensor!");
            return false;
        }

        Serial.println("Sensor initialized!");
        return true;
    }

    return false;
}
