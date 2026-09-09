#ifndef MODULE_DTWIN_H
#define MODULE_DTWIN_H

/**
 * @file    dtwin.h
 * @brief   MOD-02 Digital Twin & Visualization -- public interface
 * @author  Samet ALKAN
 * @date    2026-03-29
 * @version 0.1
 *
 * Subscribes to telemetry_t (from MOD-04 / MOD-05) over a TCP WebSocket
 * and renders the robot state on a 3-D floor-map inside Unity or Gazebo.
 * Implements a priority-based pin-drop overlay (up to DTWIN_PIN_MAX pins)
 * and exports a WebSocket feed for a browser-based dashboard.
 *
 * Changelog:
 *   v0.1 (2026-03-29) - Initial draft: dtwin_init, dtwin_push_telemetry,
 *                        dtwin_drop_pin, dtwin_shutdown
 */

#include <stdint.h>
#include "../project_types.h"   /* telemetry_t, slam_state_t, nav_status_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants & Macros
 * ------------------------------------------------------------------------ */

/** Default TCP WebSocket server port for the Digital Twin feed. */
#define DTWIN_PORT_DEFAULT      8765

/** Maximum number of simultaneous priority pins on the map overlay. */
#define DTWIN_PIN_MAX           32

/** Telemetry publish rate from MOD-04/05 to MOD-02 (Hz). */
#define DTWIN_TELEMETRY_HZ      10

/** Dead-reckoning interpolation window when WebSocket feed lags (ms). */
#define DTWIN_INTERP_WINDOW_MS  200

/** Module version string -- bump on every interface change. */
#define DTWIN_VERSION           "0.1"

/* ---------------------------------------------------------------------------
 * Error / Return Code Enum
 * ------------------------------------------------------------------------ */

/**
 * @brief Return codes for all dtwin_* functions.
 *
 * DTWIN_OK        - Operation succeeded.
 * DTWIN_ERR_CONN  - WebSocket connection could not be established or was lost.
 * DTWIN_ERR_ENCODE- JSON serialisation of a telemetry frame failed.
 * DTWIN_ERR_MAP   - Floor-map file could not be loaded or parsed.
 * DTWIN_ERR_PIN   - Pin operation failed (e.g. DTWIN_PIN_MAX exceeded).
 */
typedef enum {
    DTWIN_OK          =  0,
    DTWIN_ERR_CONN    = -1,
    DTWIN_ERR_ENCODE  = -2,
    DTWIN_ERR_MAP     = -3,
    DTWIN_ERR_PIN     = -4
} dtwin_status_t;

/* ---------------------------------------------------------------------------
 * Data Types
 * ------------------------------------------------------------------------ */

/**
 * @brief A single pin drop on the map overlay.
 *
 * Pins are displayed in the Digital Twin renderer at the specified map
 * coordinates with a colour determined by their priority level.
 */
typedef struct {
    float   x_m;       /**< Map-frame X coordinate (metres). */
    float   y_m;       /**< Map-frame Y coordinate (metres). */
    uint8_t priority;  /**< 0 = low, 1 = medium, 2 = high, 3 = critical. */
} dtwin_pin_t;

/* ---------------------------------------------------------------------------
 * Public Function Declarations
 * ------------------------------------------------------------------------ */

/**
 * @brief Initialise the WebSocket server and load the floor-map asset.
 *
 * Must be called once before any other dtwin_* function.
 * Starts a background thread that listens for incoming WebSocket connections
 * from the Digital Twin renderer (Unity / Gazebo) and the browser dashboard.
 *
 * @param map_path  Path to the occupancy-grid file (.pgm/.yaml or equivalent).
 *                  Must not be NULL.
 * @param port      WebSocket listening port.  Pass DTWIN_PORT_DEFAULT (8765)
 *                  unless a different port is configured in the environment.
 * @return DTWIN_OK on success; DTWIN_ERR_MAP if the map file cannot be read;
 *         DTWIN_ERR_CONN if the server socket cannot be bound.
 */
dtwin_status_t dtwin_init(const char *map_path, uint16_t port);

/**
 * @brief Push a combined telemetry snapshot to all connected clients.
 *
 * Serialises @p t as a JSON object and broadcasts it over the WebSocket.
 * Intended to be called at DTWIN_TELEMETRY_HZ (10 Hz) by the main loop.
 * If no clients are connected the frame is silently dropped (not queued).
 *
 * @param t  Pointer to a caller-owned telemetry_t struct.  Must not be NULL.
 *           The struct is read but not modified; the caller retains ownership.
 * @return DTWIN_OK on success; DTWIN_ERR_CONN if the WebSocket channel is
 *         unavailable; DTWIN_ERR_ENCODE if JSON serialisation fails.
 */
dtwin_status_t dtwin_push_telemetry(const telemetry_t *t);

/**
 * @brief Drop a priority pin on the map at the given map-frame coordinates.
 *
 * Up to DTWIN_PIN_MAX pins may be active simultaneously.  Adding a pin when
 * the limit is reached returns DTWIN_ERR_PIN.
 *
 * @param x_m      Map-frame X coordinate of the pin (metres).
 * @param y_m      Map-frame Y coordinate of the pin (metres).
 * @param priority Priority level: 0 = low, 1 = medium, 2 = high, 3 = critical.
 *                 Values > 3 are clamped to 3.
 * @return DTWIN_OK on success; DTWIN_ERR_PIN if the pin limit is exceeded.
 */
dtwin_status_t dtwin_drop_pin(float x_m, float y_m, uint8_t priority);

/**
 * @brief Clear all active pins from the map overlay.
 *
 * Sends a "clear-pins" message to all connected clients and resets the
 * internal pin counter to zero.
 *
 * @return DTWIN_OK always (no-op if no clients are connected).
 */
dtwin_status_t dtwin_clear_pins(void);

/**
 * @brief Shut down the WebSocket server and release all resources.
 *
 * Sends a graceful close frame to connected clients, joins the background
 * listener thread, and frees the floor-map asset.  After this call the
 * module must be re-initialised with dtwin_init() before use.
 */
void dtwin_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_DTWIN_H */
