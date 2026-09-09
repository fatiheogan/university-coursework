# MOD-03 — Hardware & Motor Control

> **One-sentence purpose:** MOD-03 owns every low-level hardware interaction on the Raspberry Pi 4 — PWM motor drive, quadrature encoder odometry, MPU-6050 IMU, and battery monitoring — and exposes a clean, thread-safe API to the rest of the robot software stack.

---

## Author

| Field | Value |
|---|---|
| **Author** | Hicran KOÇ |
| **Module ID** | MOD-03 |
| **Header file** | `hw.h` |
| **Version** | v0.1 (2026-03-29) |

---

## Dependencies

### Other modules

| Module | Why it is needed |
|---|---|
| (none at init) | MOD-03 produces data; it does not call into other modules |

### Shared types

| Header | Purpose |
|---|---|
| `project_types.h` | Central shared header at the repository root; defines `odometry_data_t`, `imu_data_t`, `motion_cmd_t`, `motion_status_t` |

### External libraries

| Library | Purpose |
|---|---|
| `RPi.GPIO` ≥ 0.7 (or `pigpio`) | GPIO edge-detect interrupts for encoders; PWM for L298N |
| `smbus2` | I²C communication with MPU-6050 |
| Python 3.11 + `threading` | Encoder ISR runs in a background daemon thread |

---

## Hardware Connections

| Signal | RPi BCM Pin | Device |
|---|---|---|
| L298N IN1 | GPIO 5 | Left motor forward |
| L298N IN2 | GPIO 6 | Left motor reverse |
| L298N ENA | GPIO 12 (PWM0) | Left motor speed |
| L298N IN3 | GPIO 13 | Right motor forward |
| L298N IN4 | GPIO 19 | Right motor reverse |
| L298N ENB | GPIO 26 (PWM1) | Right motor speed |
| Encoder Left A/B | GPIO 17, 18 | Left wheel quadrature |
| Encoder Right A/B | GPIO 22, 23 | Right wheel quadrature |
| MPU-6050 SDA/SCL | GPIO 2, 3 | I²C @ 400 kHz, addr 0x68 |

---

## Quick-Start Integration Example

```c
#include "hw.h"
#include "project_types.h"

int main(void) {
    /* 1. Initialise all hardware subsystems once at startup */
    hw_status_t rc = hw_init();
    if (rc != HW_OK) {
        fprintf(stderr, "hw_init failed: %d\n", rc);
        return 1;
    }

    /* 2. Drive forward at half speed for one second */
    motion_cmd_t cmd = {
        .left_pwm      = 0.5f,
        .right_pwm     = 0.5f,
        .stop          = 0,
        .timestamp_ms  = get_time_ms()
    };
    hw_set_motion(&cmd);
    sleep_ms(1000);

    /* 3. Read odometry (call at 50 Hz in your control loop) */
    odometry_data_t odom;
    hw_get_odometry(&odom);
    printf("Left ticks: %d  Right ticks: %d\n",
           odom.left_ticks, odom.right_ticks);

    /* 4. Read IMU (call at 100 Hz in your sensor loop) */
    imu_data_t imu;
    hw_get_imu(&imu);
    printf("Yaw rate: %.2f deg/s\n", imu.gyro_z_dps);

    /* 5. Check battery voltage */
    float volts;
    hw_get_battery_voltage(&volts);
    if (volts < 10.5f) {
        fprintf(stderr, "WARNING: low battery (%.1f V)\n", volts);
    }

    /* 6. Safety stop before exit */
    hw_emergency_stop();
    return 0;
}
```

---

## API Summary

### Return codes (`hw_status_t`)

| Code | Value | Meaning |
|---|---|---|
| `HW_OK` | 0 | Success |
| `HW_ERR_GPIO` | −1 | GPIO init or write failure |
| `HW_ERR_I2C` | −2 | I²C bus error (MPU-6050 NACK or bus hang) |
| `HW_ERR_TIMEOUT` | −3 | Operation missed its deadline |

### Constants & macros

| Macro | Value | Meaning |
|---|---|---|
| `HW_ENCODER_CPR` | `20` | Counts per revolution per wheel |
| `HW_WHEEL_DIAMETER_M` | `0.065f` | Wheel diameter (metres) |
| `HW_WHEELBASE_M` | `0.17f` | Track width between wheel centres (metres) |
| `HW_PWM_FREQ_HZ` | `50` | L298N PWM carrier frequency (Hz) |
| `HW_MAX_SPEED_MPS` | `0.5f` | Speed safety cap — clamped before PWM write |
| `HW_IMU_I2C_ADDR` | `0x68` | MPU-6050 I²C address (AD0 = GND) |
| `HW_IMU_I2C_FREQ_HZ` | `400000` | I²C clock speed (fast-mode) |
| `HW_IMU_COMP_ALPHA` | `0.98f` | Complementary filter blending coefficient |
| `HW_VERSION` | `"0.1"` | Interface version string |

### Public functions

| Function | Parameters | Returns | Notes |
|---|---|---|---|
| `hw_init()` | `void` | `hw_status_t` | Call once at startup; sets up GPIO, PWM, encoder ISR, I²C, MPU-6050 registers |
| `hw_set_motion(cmd)` | `const motion_cmd_t *cmd` | `hw_status_t` | Apply left/right PWM targets; clamps to ±`HW_MAX_SPEED_MPS`; stops if `cmd->stop != 0` |
| `hw_emergency_stop()` | `void` | `hw_status_t` | Cuts both motors immediately; safe to call from any thread |
| `hw_get_odometry(out)` | `odometry_data_t *out` | `hw_status_t` | Atomically snapshots encoder delta counters and computed wheel speeds; call at 50 Hz |
| `hw_get_imu(out)` | `imu_data_t *out` | `hw_status_t` | Synchronous I²C burst read from MPU-6050; call at 100 Hz |
| `hw_get_battery_voltage(v_out)` | `float *v_out` | `hw_status_t` | ADC read + calibration factor → battery voltage in volts |

### Data types produced

| Type | Destination | Rate | Key fields |
|---|---|---|---|
| `odometry_data_t` | MOD-04 | 50 Hz | `left_ticks`, `right_ticks`, `left_speed_mps`, `right_speed_mps`, `timestamp_ms` |
| `imu_data_t` | MOD-04 | 100 Hz | `gyro_z_dps`, `accel_x_mss`, `accel_y_mss`, `temp_c`, `timestamp_ms` |
| `motion_status_t` | MOD-05 | 50 Hz | `actual_left_mps`, `actual_right_mps`, `stall_flag`, `timestamp_ms` |

### Data types consumed

| Type | Source | Rate | Notes |
|---|---|---|---|
| `motion_cmd_t` | MOD-05 | 50 Hz | `left_pwm` and `right_pwm` are normalised −1.0 … +1.0; `stop = 1` triggers emergency stop |

---

## Known Limitations & TODOs

- **RISK-04 — GPIO interrupt jitter on non-RTOS Linux:** RPi OS scheduler may delay encoder edge-detect callbacks at high speeds, degrading odometry accuracy. Mitigation: limit `HW_MAX_SPEED_MPS` to 0.5 m/s; evaluate `pigpio` daemon as a higher-priority alternative before Week 7 integration.

- **RISK-06 — LiPo capacity:** A 2200 mAh pack powering two motors + RPi + LiDAR may last fewer than 45 minutes under full load. A low-battery threshold check (`< 10.5 V`) is provided via `hw_get_battery_voltage()`; a safe-shutdown hook should be wired to the main control loop before integration week.

- **TODO:** `hw_get_battery_voltage()` requires a hardware voltage divider on an ADC-capable pin; exact resistor values and calibration factor are TBD pending W5 hardware setup.

- **TODO:** `motion_status_t` (actual speeds + stall flag) publication to MOD-05 is described in the interface contract but the push mechanism (shared queue) is not yet implemented in this header. Will be added in v0.2.

- **TODO:** `pigpio` daemon integration (if selected over RPi.GPIO) will change the GPIO init call; the public API will remain the same, but `hw_init()` internals will differ.

---

## Version History

| Version | Date | Changes |
|---|---|---|
| v0.1 | 2026-03-29 | Initial draft — `hw_init`, `hw_set_motion`, `hw_emergency_stop`, `hw_get_odometry`, `hw_get_imu`, `hw_get_battery_voltage` |

