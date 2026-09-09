# MOD-04 — SLAM (Mapping & Localization)

LiDAR-based simultaneous localization and mapping for the LIDAR-Guided Hospital Navigation Robot.

## Authors

| Name                    | Student ID             | Role        |
|-------------------------|------------------------|-------------|
| Fatih Emre OGAN         | [230104004090] | Co-primary  |
| Mervan Deniz YILDIRIM   | [230104004173] | Co-primary  |

**Course:** CSE 396 — Computer Engineering Project (Group 10)

---

## Module Purpose

MOD-04 runs the KISS-ICP LiDAR odometry pipeline, fusing RPLidar A2 360° scans with wheel-encoder odometry and MPU-6050 IMU priors received from MOD-03. It maintains a 2-D occupancy grid (5 cm/cell) and publishes a real-time pose estimate (`slam_state_t`) to MOD-05 at 20 Hz. Drift correction is handled at predefined corridor-junction checkpoints through scan-to-reference matching with a minimum confidence threshold of 0.80.

---

## Dependencies

### Internal Modules

| Module   | Dependency                                  | Direction |
|----------|---------------------------------------------|-----------|
| MOD-03   | `odometry_data_t` (50 Hz), `imu_data_t` (100 Hz) | MOD-03 → MOD-04 |
| MOD-05   | Consumes `slam_state_t` (20 Hz), `occ_map_t` (on update) | MOD-04 → MOD-05 |
| MOD-05   | `checkpoint_req_t` / `checkpoint_ack_t` handshake | MOD-05 ↔ MOD-04 |
| MOD-02   | `telemetry_t` pose snapshot (5 Hz)          | MOD-04 → MOD-02 |
| MOD-06   | `sensor_reading_t` auxiliary sensor data (10 Hz) | MOD-06 → MOD-04 |

### Shared Header

- `project_types.h` — central shared header at the repository root; contains all shared structs and enums used across modules.

### External Libraries

| Library              | Purpose                                    |
|----------------------|--------------------------------------------|
| rplidar-roboticia    | Python wrapper for RPLidar A2 SDK          |
| KISS-ICP             | LiDAR odometry / scan matching pipeline    |
| NumPy                | Occupancy grid and matrix operations       |
| smbus2               | I²C communication (IMU data via MOD-03)    |
| Python 3.11          | Runtime (Raspberry Pi OS Bookworm 64-bit)  |

---

## Quick-Start Integration Example

The following snippet shows how a consumer (e.g. MOD-05) initialises and reads from the SLAM module:

```c
#include "slam.h"

int main(void)
{
    /* 1. Initialise with a pre-built map (or NULL for empty grid) */
    if (slam_init("/home/pi/maps/hospital_floor.pgm") != SLAM_OK) {
        fprintf(stderr, "SLAM init failed\n");
        return -1;
    }

    /* 2. Main control loop — feed odometry + IMU every tick */
    odometry_data_t odom;
    imu_data_t      imu;
    slam_state_t    pose;

    while (running) {
        hw_get_odometry(&odom);          /* from MOD-03 */
        hw_get_imu(&imu);               /* from MOD-03 */

        slam_status_t st = slam_update(&odom, &imu);
        if (st == SLAM_WARN_LOW_CONF) {
            /* Consider requesting a checkpoint snap */
        }

        /* 3. Read the latest pose */
        slam_get_pose(&pose);
        printf("x=%.2f y=%.2f θ=%.3f conf=%.2f\n",
               pose.x_m, pose.y_m, pose.theta_rad, pose.confidence);
    }

    /* 4. Clean up */
    slam_shutdown();
    return 0;
}
```

### Checkpoint Drift Correction Example

```c
/* MOD-05 requests a pose snap at junction checkpoint #3 */
checkpoint_req_t req = { .type = CP_REQ_SNAP, .checkpoint_id = 3 };
checkpoint_ack_t ack;

slam_process_checkpoint(&req, &ack);

if (ack.result == CP_ACK_OK) {
    /* Apply corrected pose */
    slam_reset_pose(ack.snapped_pose.x_m,
                    ack.snapped_pose.y_m,
                    ack.snapped_pose.theta_rad);
}
```

---

## API Summary

| Function                  | Parameters                                          | Return            | Description                                      |
|---------------------------|-----------------------------------------------------|-------------------|--------------------------------------------------|
| `slam_init`               | `const char *map_path`                              | `slam_status_t`   | Initialise SLAM engine; load pre-built map       |
| `slam_update`             | `const odometry_data_t *odom, const imu_data_t *imu`| `slam_status_t`  | Feed one tick of odometry + IMU into pipeline    |
| `slam_get_pose`           | `slam_state_t *out`                                 | `slam_status_t`   | Copy current pose estimate (thread-safe)         |
| `slam_get_map`            | `uint8_t **grid_out, uint16_t *width, uint16_t *height` | `slam_status_t` | Get read-only pointer to occupancy grid         |
| `slam_reset_pose`         | `float x_m, float y_m, float theta_rad`             | `slam_status_t`   | Override internal pose (post-checkpoint)         |
| `slam_process_checkpoint` | `const checkpoint_req_t *req, checkpoint_ack_t *ack` | `slam_status_t`  | Run scan-to-reference match at a checkpoint      |
| `slam_shutdown`           | `void`                                              | `void`            | Release LiDAR, grid, and internal threads        |

### Constants

| Constant                   | Value   | Description                                |
|----------------------------|---------|--------------------------------------------|
| `SLAM_GRID_RESOLUTION_M`  | `0.05f` | Occupancy grid cell size (metres)          |
| `SLAM_CHECKPOINT_THRESH`  | `0.80f` | Min ICP confidence for checkpoint snap     |
| `SLAM_LIDAR_MAX_RANGE_M`  | `8.0f`  | RPLidar A2 usable range cap (metres)       |
| `SLAM_UPDATE_HZ`          | `20`    | Pose publish rate to MOD-05 (Hz)           |
| `SLAM_VERSION`            | `"0.1"` | Module version string                      |

### Error Codes

| Code                  | Value | Meaning                            |
|-----------------------|-------|------------------------------------|
| `SLAM_OK`             | `0`   | Success                            |
| `SLAM_ERR_INIT`       | `-1`  | Initialisation failure             |
| `SLAM_ERR_LIDAR`      | `-2`  | LiDAR communication / scan error   |
| `SLAM_WARN_LOW_CONF`  | `1`   | ICP confidence below threshold     |

---

## Known Limitations & TODOs

1. **SLAM drift on long corridors (>20 m)** — KISS-ICP may accumulate pose error beyond odometry correction capacity. Checkpoint density for the 300-400 m² hospital floor needs to be determined during Week 5 initial mapping. *(RISK-01)*

2. **RPLidar A2 USB instability under load** — USB bus contention with the microphone may cause dropped scan frames. A powered USB hub and dedicated USB port assignment is planned. *(RISK-02)*

3. **Waypoint-ID mapping not finalised** — The occupancy grid coordinate-to-waypoint mapping depends on the completed floor map. Interim solution: hard-coded JSON lookup file. *(RISK-05)*

4. **Occupancy grid memory footprint** — A 400 m² floor at 5 cm resolution yields ~160 000 cells. Current implementation stores one byte per cell; if memory becomes tight on the 2 GB RPi 4, grid compression or tiling may be needed.

5. **Checkpoint reference scans** — Reference scans for corridor junctions must be captured during initial mapping (Week 5) and stored alongside the map file. Format and storage path TBD.

---

## Version History

| Version | Date       | Changes                                              |
|---------|------------|------------------------------------------------------|
| v0.1    | 2026-03-29 | Initial draft — `slam_init`, `slam_update`, `slam_get_pose`, `slam_get_map`, `slam_reset_pose`, `slam_process_checkpoint`, `slam_shutdown` defined. |

