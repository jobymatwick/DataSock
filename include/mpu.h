/* 
 * File:    mpu.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Functions to interface with the MPU 6050 6-axis IMU
 */

#pragma once

#include <Arduino.h>

typedef enum 
{
    GYRO_250_DEG_PER_S = 0,
    GYRO_500_DEG_PER_S,
    GYRO_1000_DEG_PER_S,
    GYRO_2000_DEG_PER_S
} mpu_gyro_range_t;

typedef enum 
{
    ACCEL_2_G = 0,
    ACCEL_4_G,
    ACCEL_8_G,
    ACCEL_16_G
} mpu_accel_range_t;

typedef enum 
{
    FILTER_260_HZ = 0,
    FILTER_184_HZ,
    FILTER_94_HZ,
    FILTER_44_HZ,
    FILTER_21_HZ,
    FILTER_10_HZ,
    FILTER_5_HZ
} mpu_filter_range_t;

typedef struct mpu_cal_t
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int8_t gx;
    int8_t gy;
    int8_t gz;
} mpu_cal_t;

const mpu_cal_t mpu_cal_configs[] =
{
    { 493,   1546, 930,  101, -1, -10 },
    { -5145, 855,  1022, 29,  50, 51  }
};

/*
 * Name:    mpu_init
 *  return: true if the IMU was detectected and configured correctly
 * Desc:    Connect to the MPU 6050 IMU over I2C and configure it's registers
 */
bool mpu_init();

/*
 * Name:    mpu_configure
 *  accel:  accelerometer range option
 *  gyro:   gyro range option
 *  filter: digital low-pass filter bandwidth option
 *  return: true if the IMU was configured correctly
 * Desc:    Configure the MPU 6050's register with provided values
 */
bool mpu_configure(mpu_accel_range_t accel, mpu_gyro_range_t gyro, mpu_filter_range_t filter);

/*
 * Name:    mpu_getAccelRange
 *  return: currently configured accelerometer range
 * Desc:    Get the currently configured accelerometer range
 */
uint8_t mpu_getAccelRange();

/*
 * Name:    mpu_getGyroRange
 *  return: currently configured gyro range
 * Desc:    Get the currently configured gyro range
 */
uint16_t mpu_getGyroRange();

/*
 * Name:    mpu_sampleRaw
 *  accel:  accelerometer data {x, y, z} (raw counts)
 *  gyro:   gyro data {x, y, z} (raw counts)
 *  temp:   temperature reading (raw counts)
 * Desc:    Get a single MPU 6050 sample and keep in raw integer format
 */
bool mpu_sampleRaw(int16_t accel[3], int16_t gyro[3], int16_t* temp);

/*
 * Name:    mpu_sampleFloat
 *  accel:  accelerometer data {x, y, z} (m/s^2)
 *  gyro:   gyro data {x, y, z} (rad/s)
 *  temp:   temperature reading (degC)
 * Desc:    Get a single MPU 6050 sample and convert to real units
 */
bool mpu_sampleFloat(float accel[3], float gyro[3], float* temp);

/*
 * Name:    mpu_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    MPU console command handler
 */
bool mpu_console(uint8_t argc, char* argv[]);
