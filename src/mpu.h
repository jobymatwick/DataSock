/* 
 * File:    mpu.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Functions to interface with the MPU 6050 6-axis IMU
 */

#pragma once

#include <Adafruit_MPU6050.h>

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
bool mpu_configure(mpu6050_accel_range_t accel, mpu6050_gyro_range_t gyro, mpu6050_bandwidth_t filter);

/*
 * Name:    mpu_sample
 *  accel:  accelerometer data {x, y, z} (m/s^2)
 *  gyro:   gyro data {x, y, z} (rad/s)
 *  temp:   temperature reading (degC)
 * Desc:    Get a single MPU 6050 sample
 */
bool mpu_sample(float accel[3], float gyro[3], float temp);

/*
 * Name:    mpu_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    MPU console command handler
 */
bool mpu_console(uint8_t argc, char* argv[]);
