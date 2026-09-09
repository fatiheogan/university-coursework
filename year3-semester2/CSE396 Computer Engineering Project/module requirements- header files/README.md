# MODULE_SLAM — SLAM: Simultaneous Localization and Mapping

## Purpose

Processes RPLidar A2 scan data combined with wheel-odometry and IMU deltas to build and continuously update a 2-D occupancy-grid map of the hospital environment, and to maintain an accurate real-time pose estimate (x, y, θ) for the robot.

---

## Authors

| Name | Student ID | Role |
|------|-----------|------|
| Fatih Emre OĞAN | — | Primary (module lead) |
| Mervan Deniz YILDIRIM | — | Primary (module lead) |
| Ertuğrul ERGÜL | — | Secondary (contributing support) |
| Hicran KOÇ | — | Secondary (contributing support) |

---

## Dependencies

| Dependency | Type | Provider |
|-----------|------|---------|
| `sensor.h` / M6 Sensor Integration | Internal module | `slam_scan_t` data and raw LiDAR frames |
| `hardware.h` / M3 Hardware & Motor | Internal module | Odometry deltas `dx`, `dy`, `dtheta` from encoders + IMU |
| `kiss-icp` (KISS-ICP library) | External library | Point-cloud scan matching; Python/C++ via `kiss_icp_ros2` |
| `numpy` | External Python library | Numerical operations in the Python binding layer |
| `stdint.h`, `stdbool.h` | System headers | C standard types |

---

## Quick-Start Integration Example

```c
#include "slam.h"
#include "sensor.h"    /* for slam_scan_t */

/* --- Initialise SLAM at startup --- */
slam_pose_t start = { .x = 0.0f, .y = 0.0f, .theta = 0.0f, .timestamp = 0 };
if (slam_init(&start) != SLAM_OK) {
    /* handle error */
}

/* --- Register pose callback (optional) --- */
void on_pose_update(const slam_pose_t *pose) {
    printf("Pose: x=%.2f  y=%.2f  θ=%.3f rad\n",
           pose->x, pose->y, pose->theta);
}
slam_register_pose_callback(on_pose_update);

/* --- Main control loop (~10 Hz) --- */
slam_scan_t   scan;
float dx, dy, dtheta;

while (running) {
    sensor_get_scan(&scan);          /* M6 provides the scan  */
    odometry_get_delta(&dx, &dy, &dtheta); /* M3 provides deltas   */

    slam_status_t st = slam_update(&scan, dx, dy, dtheta);
    if (st == SLAM_ERR_ICP_FAIL) {
        /* ICP below threshold — pose NOT corrected, odometry used only */
        log_warn("ICP match low quality");
    }

    /* Provide pose to NAV module (M5) */
    slam_pose_t current_pose;
    slam_get_pose(&current_pose);
    nav_set_pose(&current_pose);
}
```

---

## API Summary

### Data Types

| Type | Description |
|------|------------|
| `slam_status_t` | Return code enum for all SLAM functions |
| `slam_pose_t` | Robot 2-D pose: x (m), y (m), theta (rad), timestamp |
| `slam_scan_point_t` | Single LiDAR point: angle (rad), range (m) |
| `slam_scan_t` | Full scan frame: array of `slam_scan_point_t`, count, timestamp |
| `slam_map_t` | 2-D occupancy grid: cells[400][400], origin, resolution |
| `slam_diagnostics_t` | Diagnostics: ICP score, drift estimate, map_ready flag |

### Functions

| Function | Parameters | Return | Description |
|----------|-----------|--------|-------------|
| `slam_init` | `const slam_pose_t *initial_pose` | `slam_status_t` | Initialise module; call once at startup |
| `slam_update` | `const slam_scan_t *scan, float dx, float dy, float dtheta` | `slam_status_t` | Feed scan + odometry delta, run ICP, update map & pose |
| `slam_get_pose` | `slam_pose_t *out` | `slam_status_t` | Copy latest pose into caller buffer |
| `slam_get_map` | `const slam_map_t **out` | `slam_status_t` | Get pointer to current occupancy grid |
| `slam_register_pose_callback` | `slam_pose_cb_t cb` | `slam_status_t` | Register callback called on each pose update |
| `slam_register_map_callback` | `slam_map_cb_t cb` | `slam_status_t` | Register callback called on each map update |
| `slam_get_diagnostics` | `slam_diagnostics_t *out` | `slam_status_t` | Read ICP score, drift, update count |
| `slam_reset` | `void` | `slam_status_t` | Clear map and reset pose to origin |

### Error Codes

| Code | Value | Meaning |
|------|-------|---------|
| `SLAM_OK` | 0 | Success |
| `SLAM_ERR_NOT_INIT` | -1 | `slam_init()` not called |
| `SLAM_ERR_BAD_PARAM` | -2 | NULL pointer or invalid value |
| `SLAM_ERR_NO_SCAN` | -3 | No LiDAR data available |
| `SLAM_ERR_ICP_FAIL` | -4 | ICP match score below threshold |
| `SLAM_ERR_MAP_FULL` | -5 | Occupancy grid capacity exceeded |
| `SLAM_ERR_TIMEOUT` | -6 | Sensor read timed out |

---

## Known Limitations & TODOs

- **Single-floor only:** Map is a single 400×400 cell 2-D grid; multi-floor or 3-D environments are not supported.
- **Static map assumption:** Map is built incrementally but does not currently handle dynamic obstacles (e.g., doors opening/closing). Planned fix: occupancy decay factor.
- **ICP cold-start:** First 3–5 scan frames may produce low ICP scores until a sufficient local map is built; odometry is used exclusively during this period.
- **Thread safety:** `slam_get_map()` returns an internal pointer that may be overwritten by the next `slam_update()`. Consumers (M5, M2) must copy the map or synchronise access.
- **TODO:** Add `slam_save_map()` / `slam_load_map()` for pre-built map injection.
- **TODO:** Tune `SLAM_ICP_MATCH_THRESHOLD` during field testing in the hospital environment.
- **TODO:** Evaluate KISS-ICP versus custom ICP implementation for latency on Raspberry Pi 4.

---

## Inter-Module Communication

| Direction | Partner Module | Data | Notes |
|-----------|---------------|------|-------|
| M6 → M4 | Sensor Integration | `slam_scan_t` | 10 Hz, raw LiDAR frames via shared memory |
| M3 → M4 | Hardware & Motor | `float dx, dy, dtheta` | Per control cycle (~50 ms), odometry deltas |
| M4 → M5 | NAV – Path Planning | `slam_pose_t` | On each update via callback or `slam_get_pose()` |
| M4 → M5 | NAV – Path Planning | `slam_map_t*` | On map update via callback or `slam_get_map()` |
| M4 → M2 | Digital Twin | `slam_pose_t`, `slam_map_t*` | For live 3-D visualization |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| v0.1 | 2025-03-24 | Initial draft: slam_init, slam_update, slam_get_pose, slam_get_map, slam_reset, core data types |
