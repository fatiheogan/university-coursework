# MOD-02 — Digital Twin & Visualization

> **Real-time 3-D mirror of the hospital navigation robot's state,
> rendered on a floor-map inside Unity / Gazebo and streamed to a
> browser dashboard over WebSocket.**

---

## Author

| Field | Value |
|---|---|
| **Name** | Samet ALKAN |
| **Module ID** | MOD-02 |
| **Primary role** | Digital Twin & Visualization |
| **Secondary support** | MOD-05 (NAV) |

---

## Dependencies

| Dependency | Type | Notes |
|---|---|---|
| `project_types.h` | Project header | Central shared header at the repository root; shared structs: `telemetry_t`, `slam_state_t`, `nav_status_t` |
| `MOD-04 (SLAM)` | Runtime module | Source of `telemetry_t` pose data via WebSocket |
| `MOD-05 (NAV)` | Runtime module | Source of navigation status data inside `telemetry_t` via WebSocket |
| `python-socketio` | External library | WebSocket server backend |
| Unity 2022 LTS **or** Gazebo | External tool | 3-D renderer / Digital Twin environment |
| `dtwin.h` | This module | Public C interface |

The module does **not** depend on MOD-01, MOD-03, or MOD-06 at runtime.

---

## Quick-Start Integration Example

### Sending data **to** MOD-02 (from MOD-04 / MOD-05)

```c
#include "dtwin.h"
#include "project_types.h"

int main(void) {
    /* 1. Initialise once at startup */
    dtwin_status_t rc = dtwin_init("maps/hospital_floor.pgm", DTWIN_PORT_DEFAULT);
    if (rc != DTWIN_OK) {
        /* handle error */
        return -1;
    }

    telemetry_t frame = {0};

    /* Main loop ? call at DTWIN_TELEMETRY_HZ (10 Hz) */
    while (running) {
        /* Fill from MOD-04 / MOD-05 outputs */
        frame.pose      = slam_get_current_pose();   /* slam_state_t  */
        frame.nav       = nav_get_current_status();  /* nav_status_t  */
        frame.battery_v = hw_get_battery_voltage();  /* float, volts  */
        frame.uptime_ms = get_uptime_ms();

        dtwin_push_telemetry(&frame);

        sleep_ms(100); /* 10 Hz */
    }

    dtwin_shutdown();
    return 0;
}
```

### Dropping a pin (e.g. when an obstacle is detected)

```c
/* Drop a critical-priority pin at the obstacle's map coordinates */
dtwin_drop_pin(obstacle_x_m, obstacle_y_m, 3 /* critical */);

/* Clear all pins when the path is clear */
dtwin_clear_pins();
```

### Connecting the browser dashboard

Open a browser and navigate to:

```
http://<raspberry-pi-ip>:8765
```

The page receives live JSON telemetry at 10 Hz without any additional configuration.

---

## API Summary

### Return Codes ? `dtwin_status_t`

| Code | Value | Meaning |
|---|---|---|
| `DTWIN_OK` | 0 | Success |
| `DTWIN_ERR_CONN` | -1 | WebSocket connection unavailable or lost |
| `DTWIN_ERR_ENCODE` | -2 | JSON serialisation failure |
| `DTWIN_ERR_MAP` | -3 | Floor-map file could not be loaded |
| `DTWIN_ERR_PIN` | -4 | Pin limit exceeded (`DTWIN_PIN_MAX = 32`) |

### Public Functions

| Function | Parameters | Returns | Description |
|---|---|---|---|
| `dtwin_init` | `const char *map_path`, `uint16_t port` | `dtwin_status_t` | Initialise WebSocket server and load floor-map. Call once at startup. |
| `dtwin_push_telemetry` | `const telemetry_t *t` | `dtwin_status_t` | Broadcast a telemetry snapshot to all connected clients (10 Hz). |
| `dtwin_drop_pin` | `float x_m`, `float y_m`, `uint8_t priority` | `dtwin_status_t` | Place a priority pin at map coordinates. Priority 0?3. |
| `dtwin_clear_pins` | *(none)* | `dtwin_status_t` | Remove all active pins from the overlay. |
| `dtwin_shutdown` | *(none)* | `void` | Gracefully close server and free all resources. |

### Key Constants

| Constant | Value | Description |
|---|---|---|
| `DTWIN_PORT_DEFAULT` | 8765 | Default WebSocket port |
| `DTWIN_PIN_MAX` | 32 | Max simultaneous map pins |
| `DTWIN_TELEMETRY_HZ` | 10 | Expected push rate (Hz) |
| `DTWIN_INTERP_WINDOW_MS` | 200 | Dead-reckoning interpolation window when feed lags |
| `DTWIN_VERSION` | `"0.1"` | Module version string |

---

## Incoming Data Flows

MOD-02 **consumes** the following structs (defined in the central `project_types.h`):

| Struct | Source | Rate | Transport |
|---|---|---|---|
| `telemetry_t` | MOD-04 / MOD-05 | 5?10 Hz | TCP WebSocket JSON |
| `nav_status_t` | MOD-05 | 10 Hz | TCP WebSocket JSON |
| `slam_state_t` *(embedded in telemetry)* | MOD-04 | 5 Hz | TCP WebSocket JSON |

MOD-02 does **not** send commands to any other module.

---

## Known Limitations & TODOs

- **RISK-07 (LOW):** If RPi WiFi is congested the 10 Hz telemetry feed may lag.
  MOD-02 must implement dead-reckoning interpolation
  (`DTWIN_INTERP_WINDOW_MS = 200 ms`) to compensate for missing frames.
  *Status: not yet implemented in v0.1.*

- **Floor-map format:** The current interface accepts a `.pgm`/`.yaml` pair
  (ROS-style occupancy map). If the SLAM team switches to a different format,
  `dtwin_init` will need to be updated accordingly.

- **Pin persistence:** Pins are cleared only via `dtwin_clear_pins()` or
  `dtwin_shutdown()`. A per-pin TTL / expiry mechanism may be added in a
  future version.

- **Audio event logging:** The report mentions MOD-01 publishing `audio_event_t`
  to MOD-02 for Digital Twin logging. This interface is not yet reflected in
  `dtwin.h`; a `dtwin_log_audio_event()` function will be added in v0.2.

- **Renderer choice:** Unity 2022 LTS is the primary target; Gazebo is a
  fallback. The WebSocket JSON schema is renderer-agnostic.

---

## Version History

| Version | Date | Changes |
|---|---|---|
| v0.1 | 2026-03-29 | Initial draft: `dtwin_init`, `dtwin_push_telemetry`, `dtwin_drop_pin`, `dtwin_clear_pins`, `dtwin_shutdown` |

---

*For interface change requests, contact **Samet ALKAN** (MOD-02 primary) and
notify **MOD-04** and **MOD-05** leads as they are direct data producers.*

