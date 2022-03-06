/* 
 * File:    mpu.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Functions to interface with the MPU 6050 6-axis IMU
 */

#include "mpu.h"

#include <string.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

#include "storage.h"

#define G_M_PER_S 9.8066
#define DEG_PER_RAD 0.0174533

static const uint16_t accel_ranges[] = { 2, 4, 8, 16 }; 
static const uint16_t gyro_ranges[] = { 250, 500, 1000, 2000 }; 
static const uint16_t filter_ranges[] = { 260, 184, 194, 44, 21, 10, 5 }; 

MPU6050 _mpu;
bool _connected = false;
mpu_accel_range_t _accel_setting = ACCEL_4_G;
mpu_gyro_range_t _gyro_setting = GYRO_500_DEG_PER_S;
mpu_filter_range_t _filter_setting = FILTER_21_HZ;

bool mpu_init()
{
    Wire.begin();

    if (_connected) _mpu.reset();
    _mpu.initialize();

    if (!_mpu.testConnection())
    {
        Serial.println("MPU connection failed.");
        _connected = false;
        return _connected;
    }

    _connected = true;

    _mpu.setXAccelOffset(mpu_cal_configs[(int) storage_configGetNum(CONFIG_MPU_ID)].ax);
    _mpu.setYAccelOffset(mpu_cal_configs[(int) storage_configGetNum(CONFIG_MPU_ID)].ay);
    _mpu.setZAccelOffset(mpu_cal_configs[(int) storage_configGetNum(CONFIG_MPU_ID)].az);
    _mpu.setXGyroOffset(mpu_cal_configs[(int) storage_configGetNum(CONFIG_MPU_ID)].gx);
    _mpu.setXGyroOffset(mpu_cal_configs[(int) storage_configGetNum(CONFIG_MPU_ID)].gy);
    _mpu.setXGyroOffset(mpu_cal_configs[(int) storage_configGetNum(CONFIG_MPU_ID)].gz);

    _connected = mpu_configure(_accel_setting, _gyro_setting, _filter_setting);

    return _connected;
}

bool mpu_configure(mpu_accel_range_t accel, mpu_gyro_range_t gyro, mpu_filter_range_t filter)
{
    if (!_connected)
        return false;

    _mpu.setFullScaleGyroRange(gyro);
    _mpu.setFullScaleAccelRange(accel);
    _mpu.setDLPFMode(filter);

    // Confirm settings applied correctly
    if (_mpu.getFullScaleGyroRange() != accel ||
        _mpu.getFullScaleAccelRange() != gyro ||
        _mpu.getDLPFMode() != filter)
    {
        Serial.println("Settings not applied.");
        _connected = false;
        return _connected;
    }

    _gyro_setting = gyro;
    _accel_setting = accel;
    _filter_setting = filter;

    return _connected;
}

uint8_t mpu_getAccelRange()
{
    return accel_ranges[_accel_setting];
}

uint16_t mpu_getGyroRange()
{
    return gyro_ranges[_gyro_setting];
}

bool mpu_sampleRaw(int16_t accel[3], int16_t gyro[3], int16_t* temp)
{
    if (!_connected || !_mpu.testConnection())
    {
        _connected = false;
        return _connected;
    }

    _mpu.getMotion6(&accel[0], &accel[1], &accel[2], &gyro[0], &gyro[1], &gyro[2]);
    *temp = _mpu.getTemperature();

    return _connected;
}

bool mpu_sampleFloat(float accel[3], float gyro[3], float* temp)
{
    int16_t a[3], g[3], t;
    if (!mpu_sampleRaw(a, g, &t))
    {
        // Zero everthing
        memset(accel, 0, sizeof(*accel) * 3);
        memset(gyro, 0, sizeof(*gyro) * 3);
        memset(temp, 0, sizeof(*temp) * 1);

        return _connected;
    }

    // Convert accelerometer register values to ms^2 & gyro register values to deg/s
    float a_factor = G_M_PER_S / (UINT16_MAX / (2 * mpu_getAccelRange()));
    float g_factor = DEG_PER_RAD / (UINT16_MAX / (2 * mpu_getGyroRange()));
    for (uint8_t i = 0; i < 3; i++)
    {
        accel[i] = a[i] * a_factor;
        gyro[i] = g[i] * g_factor;
    }

    // Covert temperature register values to degC
    *temp = (t / 340.0) + 36.53;

    return _connected;
}

bool mpu_console(uint8_t argc, char* argv[])
{
    if (!strcmp("init", argv[1]))
    {
        if (!mpu_init())
        {
            Serial.println("Error initializing sensor!");
            return false;
        }

        Serial.println("Sensor initialized!");
        return true;
    }

    if (!strcmp("sample", argv[1]))
    {
        float a[3], g[3], t;
        if (!mpu_sampleFloat(a, g, &t))
        {
            Serial.println("Error getting sensor data!");
            return false;
        }

        Serial.printf("A: [%6.2f, %6.2f, %6.2f], ", a[0], a[1], a[2]);
        Serial.printf("G: [%6.2f, %6.2f, %6.2f], ", g[0], g[1], g[2]);
        Serial.printf("T: %5.2f\r\n", t);

        return true;
    }

    return false;
}
