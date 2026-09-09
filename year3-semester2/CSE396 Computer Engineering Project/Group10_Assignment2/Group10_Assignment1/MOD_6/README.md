# MOD-06 — Sensor Integration

Sensor abstraction, noise filtering, and reactive obstacle detection for the LIDAR-Guided Hospital Navigation Robot.

## Authors

| Name                       | Student ID       | Role       |
|----------------------------|------------------|------------|
| Sayed Khalilullah Hashimi  | 200104004806     | Primary    |

**Course:** CSE 396 — Computer Engineering Project (Group 10)

---

## Module Purpose

MOD-06 abstracts raw environmental data from the LiDAR's forward arc ($\pm25^{\circ}$) and supplementary proximity sensors (Ultrasonic/IR). It applies noise filtering (e.g., rolling averages) to reject anomalies and continuously evaluates the immediate path. It provides clean `obstacle_alert_t` flags to MOD-05 (Navigation) at 20 Hz to trigger safe halting, and calculates lateral free space to recommend detour bearings.

---

## Dependencies

### Internal Modules

| Module   | Dependency                                  | Direction |
|----------|---------------------------------------------|-----------|
| MOD-05   | Consumes `obstacle_alert_t` (20 Hz)         | MOD-06 → MOD-05 |
| MOD-04   | Consumes `sensor_reading_t` (10 Hz)         | MOD-06 → MOD-04 |
| MOD-02   | Telemetry raw sensor snapshot (5 Hz)        | MOD-06 → MOD-02 |

### Shared Header

- `project_types.h` — central shared header at the repository root; provides `sensor_reading_t` and `obstacle_alert_t`.

### External Libraries

| Library              | Purpose                                    |
|----------------------|--------------------------------------------|
| RPi.GPIO             | Reading logic levels for Ultrasonic/IR     |
| Python 3.11          | Runtime (Raspberry Pi OS Bookworm 64-bit)  |

---

## Quick-Start Integration Example

The following snippet shows how MOD-05 (Navigation) integrates with the Sensor module:

```c
#include "sensor.h"
#include <stdio.h>

int main(void)
{
    /* 1. Initialise the sensor hardware */
    if (sensor_init() != SENSOR_OK) {
        fprintf(stderr, "Sensor init failed\n");
        return -1;
    }

    obstacle_alert_t current_alert;
    float detour_bearing = 0.0f;

    /* 2. Main control loop */
    while (running) {
        sensor_update(); /* Read hardware and filter noise */
        
        sensor_get_alert(&current_alert);
        
        if (current_alert.active) {
            printf("HALT: Obstacle detected at %.2f m! Bearing: %.1f deg\n", 
                   current_alert.min_distance_m, 
                   current_alert.bearing_deg);
                   
            /* Request detour direction */
            if (sensor_get_best_detour(&detour_bearing) == SENSOR_OK) {
                printf("Recommended detour bearing: %.1f\n", detour_bearing);
            }
        }
    }
    return 0;
}

