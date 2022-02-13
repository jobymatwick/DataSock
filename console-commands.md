# Console Command Listing
- [Console Command Listing](#console-command-listing)
  - [`mpu` - MPU 6050](#mpu---mpu-6050)
    - [`init` - Initialize or Reset](#init---initialize-or-reset)
    - [`sample` - Get single sensor sample](#sample---get-single-sensor-sample)
  - [`adc` - Analog-to-Digital Converter](#adc---analog-to-digital-converter)
    - [`init` - Set resolution](#init---set-resolution)
    - [`sample` - Get ADC reading(s)](#sample---get-adc-readings)
    - [`print` - Print the next `n` scheduled samples](#print---print-the-next-n-scheduled-samples)
  - [`clock` - Real Time Clock](#clock---real-time-clock)
    - [`get` - Get Local Time](#get---get-local-time)
    - [`set` - Set Local Time/Date](#set---set-local-timedate)
    - [`tz` - Set Timezone](#tz---set-timezone)
    - [`status` - Get Status](#status---get-status)
  - [`sd` - SD Card Storage](#sd---sd-card-storage)
    - [`init` - Connect to SD card](#init---connect-to-sd-card)
    - [`ls` - List files](#ls---list-files)
    - [`cat` - Print File Contents](#cat---print-file-contents)
    - [`default` - Create Default Config](#default---create-default-config)
    - [`load` - Load Settings file](#load---load-settings-file)
    - [`format` - Wipe the SD card](#format---wipe-the-sd-card)

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
Commands to interface with the Teensy's ADC.

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

### `print` - Print the next `n` scheduled samples
Prints the next `n` scheduled ADC samples as they occur. A value of -1 starts indefinitely printing; 0 stops this.
```
> adc print 10
[1016]
...
[8191]
```

## `clock` - Real Time Clock
Commands to interface with the real time clock within the Teensy. The RTC is configred with the compile time of the program by default and with a timezone of UTC-8 (Pacific). Unless a battery is connected to the Teensy VBat pins, the set time will be reset to default on power loss.

### `get` - Get Local Time
Get the local time as an ISO 8601 timestamp (`YYYY-MM-DDTHH:MM:SS+TZ`)
```
> clock get
Localtime: 2022-01-30T23:09:09-8:00
```

### `set` - Set Local Time/Date
Set the local time or time and date.
```
> clock set 10 50 00
RTC: 1643568600 UTC
Localtime: 2022-01-30T10:50:00-8:00
> clock set 10 50 00 25 2 2022
RTC: 1645815000 UTC
Localtime: 2022-02-25T10:50:00-8:00
```

### `tz` - Set Timezone
Set the timezone as an offset from UTC in hours
```
> clock tz -8
Timezone: -8
```

### `status` - Get Status
Check if the clock has been set since the last time the RTC lost power. 
```
> clock status
Clock has been set.
```

## `sd` - SD Card Storage
### `init` - Connect to SD card
Initiates communications with the SD card if present or return an error.
```
> sd init
SD initialized!
```

### `ls` - List files
Prints a list of all files on the SD card.
```
> sd ls
Last Modified    Size (Bytes) Filename
2022-02-12 12:41           52 config.txt
2022-02-12 13:58           31 other.txt
```

### `cat` - Print File Contents
Prints the contents of a specific file.
```
> sd cat config.txt
poll_rate=100
device_name=DataSock
value=default
```

### `default` - Create Default Config
Creates a new or replaces any existing config file with compiled defaults.
```
> sd default
Removing original config file
Default "config.txt" created.
```

### `load` - Load Settings file
Load and parse the settings file from the SD card.
```
> sd load
Loaded 3 settings.
```

### `format` - Wipe the SD card
Completely erase the SD card and then format as exFAT and create a default config file.
```
> sd format
Found 63.86 GB SD card
Starting erase... 100% complete.
All blocks erased to 0x00
Starting format...
Writing FAT .................................
Writing upcase table
Writing root
Format done
Default "config.txt" created.
```
