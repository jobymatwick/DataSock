# Console Command Listing
- [Console Command Listing](#console-command-listing)
  - [`mpu` - MPU 6050](#mpu---mpu-6050)
    - [`init` - Initialize or Reset](#init---initialize-or-reset)
    - [`sample` - Get single sensor sample](#sample---get-single-sensor-sample)
  - [`adc` - Analog-to-Digital Converter](#adc---analog-to-digital-converter)
    - [`init` - Set resolution](#init---set-resolution)
    - [`sample` - Get ADC reading(s)](#sample---get-adc-readings)
## `mpu` - MPU 6050
Commands to interface with the MPU 6050 6-axis IMU over I2C.

### `init` - Initialize or Reset
Intialize and configure the MPU 6050. If the sensor is already connected, it is reset an then reconfigured.
```
> mpu init
Sensor initialized!
```

### `sample` - Get single sensor sample
Get a single sensor reading and print it to the console.
```
> mpu sample
A: [ -4.01,  -6.60,  -7.05], G: [ -0.07,   0.01,  -0.02], T: 28.86
```

## `adc` - Analog-to-Digital Converter
Commands to interface with the MPU 6050 6-axis IMU over I2C.

### `init` - Set resolution
Set the ADC resolution to the maximum usable for single-ended sampling (13 bits).
```
> adc init
ADC initialized!
```

### `sample` - Get ADC reading(s)
Get the reading(s) from one or more ADC channels.
```
> adc sample 1 2 3 4
[1016, 8191, 3912, 8169]
> adc sample 1
[1316]
```
