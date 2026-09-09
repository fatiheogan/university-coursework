# MOD-01 — Voice Communication

Voice Communication converts spoken Turkish destination requests into structured navigation goals and reads navigation status updates back to the user as synthesized speech.

## Author

- **Name:** Muhammet Gokhan OZBEK
- **Student ID:** 210104004038

## Purpose

MOD-01 owns the robot's voice-facing interface. It captures microphone input, runs offline speech-to-text, maps recognized commands to `nav_goal_t`, forwards those goals to MOD-05, and speaks back `nav_status_t` state changes using Turkish TTS. It also acts as the human-facing confirmation layer for noisy hospital navigation scenarios identified in the report.

## Dependencies

- Shared header: central `project_types.h` at the repository root
- Downstream module contract: MOD-05 Navigation (`nav_goal_t` producer, `nav_status_t` consumer)
- Audio input hardware: Youmi Mini USB 2.0 microphone
- Audio output hardware: speaker + PAM8406 amplifier
- External libraries / tools:
  - Offline STT: Whisper tiny or Vosk
  - Offline TTS: Piper TTS (`tr_TR-dfki-medium`)
  - ALSA / PyAudio or equivalent Linux audio backend

## Public Headers

- `voice.h` - Public API for STT, TTS, goal retrieval, and NAV status callbacks
- `project_types.h` - Central shared inter-module type definitions

## Quick-Start Integration Example

```c
#include <stdio.h>
#include "voice.h"

static void on_nav_status(const nav_status_t *st)
{
    if (st != NULL) {
        (void)voice_speak(st->message);
    }
}

int main(void)
{
    nav_goal_t goal;

    if (voice_init() != VOICE_OK) {
        return -1;
    }

    voice_set_status_cb(on_nav_status);

    if (voice_start_listen() != VOICE_OK) {
        return -1;
    }

    if (voice_get_goal(&goal) == VOICE_OK) {
        printf("Destination: %s (waypoint %u)\n",
               goal.destination,
               (unsigned)goal.waypoint_id);
        /* Forward goal to MOD-05 via the team integration layer. */
    }

    voice_stop_listen();
    return 0;
}
```

## API Summary

| Function | Parameters | Returns |
| --- | --- | --- |
| `voice_init(void)` | None | `voice_status_t` - `VOICE_OK` on success, error otherwise |
| `voice_start_listen(void)` | None | `voice_status_t` - starts continuous STT loop |
| `voice_stop_listen(void)` | None | `voice_status_t` - stops listening and releases audio input |
| `voice_speak(const char *text)` | `text`: NUL-terminated UTF-8 Turkish string | `voice_status_t` - `VOICE_OK` or TTS-related error |
| `voice_get_goal(nav_goal_t *out)` | `out`: caller-owned output goal struct | `voice_status_t` - `VOICE_OK` or `VOICE_ERR_TIMEOUT` |
| `voice_set_status_cb(voice_status_cb_t cb)` | `cb`: callback pointer or `NULL` | `void` |

## Constants and Callback Contract

| Symbol | Meaning |
| --- | --- |
| `VOICE_MAX_CMD_LEN` | Maximum recognized command length in characters |
| `VOICE_SPEAK_TIMEOUT_MS` | Maximum blocking TTS synthesis/playback wait |
| `VOICE_VERSION` | Current public header version string |
| `voice_status_cb_t` | Callback signature for incoming `nav_status_t` updates |

## Inter-Module Contract

- MOD-01 -> MOD-05: `nav_goal_t`
  - Trigger: when a valid spoken destination command is parsed
  - Notes: contains human-readable `destination`, agreed `waypoint_id`, and `timestamp_ms`
- MOD-05 -> MOD-01: `nav_status_t`
  - Trigger: on every NAV state transition
  - Notes: used for spoken feedback such as moving, waiting, rerouting, and arrived

## Known Limitations and TODOs

- `waypoint_id` mapping depends on the final floor-map lookup table agreed with MOD-05.
- Turkish speech recognition accuracy may drop in noisy hospital corridors.
- The header does not define the team-wide transport layer; queue / threading integration remains implementation-side.
- Voice confirmation flow for ambiguous commands is planned but not yet represented as a separate public API.

## Version History

- `v0.1` (2026-03-29) - Initial MOD-01 header and README draft based on Assignment 2 report and instructor requirements

