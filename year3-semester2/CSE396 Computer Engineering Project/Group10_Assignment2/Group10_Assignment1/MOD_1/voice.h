#ifndef MODULE_VOICE_H
#define MODULE_VOICE_H

/**
 * @file    voice.h
 * @brief   MOD-01 Voice Communication public interface
 * @author  Muhammet Gokhan OZBEK 210104004038
 * @date    2026-03-29
 * @version 0.1
 *
 * Brief:
 *   Captures Turkish voice commands, converts them into nav_goal_t requests
 *   for MOD-05, and speaks nav_status_t updates received back from navigation.
 *
 * Changelog:
 *   v0.1 (2026-03-29) - Initial interface draft for STT, TTS, and NAV dispatch
 */

#include <stdint.h>
#include "../project_types.h"

/* -- Constants ----------------------------------------------- */
#define VOICE_MAX_CMD_LEN        64U
#define VOICE_SPEAK_TIMEOUT_MS   5000U
#define VOICE_VERSION            "0.1"

/* -- Callback Types ------------------------------------------ */
typedef void (*voice_status_cb_t)(const nav_status_t *st);

/* -- Return Codes -------------------------------------------- */
typedef enum {
    VOICE_OK          = 0,
    VOICE_ERR_MIC     = -1,
    VOICE_ERR_STT     = -2,
    VOICE_ERR_TTS     = -3,
    VOICE_ERR_TIMEOUT = -4
} voice_status_t;

/* -- Public Functions ---------------------------------------- */
/**
 * @brief  Initialise microphone capture and TTS engine resources.
 * @return VOICE_OK on success, error code otherwise.
 */
voice_status_t voice_init(void);

/**
 * @brief  Start the continuous speech-to-text listening loop.
 * @return VOICE_OK on success, error code otherwise.
 */
voice_status_t voice_start_listen(void);

/**
 * @brief  Stop the listening loop and release audio input resources.
 * @return VOICE_OK on success, error code otherwise.
 */
voice_status_t voice_stop_listen(void);

/**
 * @brief  Synthesize and play Turkish speech through the speaker.
 * @param  text  Pointer to a NUL-terminated UTF-8 output string.
 * @return VOICE_OK on success, error code otherwise.
 */
voice_status_t voice_speak(const char *text);

/**
 * @brief  Block until a spoken command is parsed into a navigation goal.
 * @param  out  Pointer to a caller-owned nav_goal_t that will be filled.
 * @return VOICE_OK on success, or VOICE_ERR_TIMEOUT if no goal is produced.
 */
voice_status_t voice_get_goal(nav_goal_t *out);

/**
 * @brief  Register or unregister the callback for NAV status updates.
 * @param  cb  Callback function pointer, or NULL to unregister.
 */
void voice_set_status_cb(voice_status_cb_t cb);

#endif /* MODULE_VOICE_H */
