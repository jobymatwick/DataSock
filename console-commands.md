# Console Command Listing
- [Console Command Listing](#console-command-listing)
  - [`mpu` - MPU 6050](#mpu---mpu-6050)
    - [`init` - Initialize or Reset](#init---initialize-or-reset)
    - [`sample` - Get single sensor sample](#sample---get-single-sensor-sample)
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
> mpu get
A: [ -4.01,  -6.60,  -7.05], G: [ -0.07,   0.01,  -0.02], T: 28.86
```
