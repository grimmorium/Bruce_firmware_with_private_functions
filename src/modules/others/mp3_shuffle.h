#ifndef __MP3_SHUFFLE_H__
#define __MP3_SHUFFLE_H__

#include <Arduino.h>

/**
 * @brief Scans SD:/mp3 and plays every .mp3 file found in random (shuffled)
 *        order, looping forever until the user presses ESC.
 *
 * Controls:
 *   NEXT   -> skip to next track
 *   PREV   -> go back to previous track
 *   SEL/OK -> pause / resume
 *   ESC    -> stop playback and exit
 *
 * @note Requires a device with HAS_NS4168_SPKR (e.g. Cardputer / Cardputer ADV)
 */
void mp3ShufflePlayerUI();

#endif // __MP3_SHUFFLE_H__
