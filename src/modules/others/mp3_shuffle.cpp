// MP3 Shuffle Player
// Scans SD:/mp3 for .mp3 files and plays them back to back in random order.
// Built on top of the existing async audio backend in modules/others/audio.cpp

#include "mp3_shuffle.h"

#if defined(HAS_NS4168_SPKR)

#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "modules/others/audio.h"
#include <SD.h>
#include <algorithm>
#include <vector>

#define MP3_FOLDER "/mp3"

// ===== HELPERS =====

// Removes path + extension, keeping only the track name for display
static String mp3ExtractName(const String &filepath) {
    int lastSlash = filepath.lastIndexOf('/');
    int lastDot = filepath.lastIndexOf('.');
    String name = filepath.substring(lastSlash + 1);
    if (lastDot > lastSlash) { name = name.substring(0, lastDot - lastSlash - 1); }
    return name;
}

// Lists every *.mp3 file directly inside SD:/mp3 (non-recursive)
static std::vector<String> mp3ScanFolder() {
    std::vector<String> files;

    File root = SD.open(MP3_FOLDER);
    if (!root || !root.isDirectory()) {
        if (root) root.close();
        return files;
    }

    while (true) {
        bool isDir;
        String fullPath = root.getNextFileName(&isDir);
        if (fullPath == "") break;
        if (isDir) continue;

        String nameOnly = fullPath.substring(fullPath.lastIndexOf('/') + 1);

        // Skip hidden files and macOS AppleDouble metadata files
        // (e.g. ".DS_Store", "._track.mp3") that Finder/Explorer sometimes
        // write to SD cards - these are not valid audio and will always
        // fail to play, causing an instant-skip loop.
        if (nameOnly.startsWith(".")) continue;

        String lower = nameOnly;
        lower.toLowerCase();
        if (lower.endsWith(".mp3")) files.push_back(fullPath);
    }
    root.close();

    return files;
}

// Fisher-Yates shuffle of track indexes
static void mp3ShuffleOrder(std::vector<int> &order) {
    for (int i = (int)order.size() - 1; i > 0; i--) {
        int j = random(0, i + 1);
        std::swap(order[i], order[j]);
    }
}

// ===== UI =====

static void mp3DrawHeader(int current, int total) {
    tft.fillRect(0, 0, tftWidth, 20, bruceConfig.priColor);
    tft.setTextColor(bruceConfig.bgColor, bruceConfig.priColor);
    tft.setTextSize(1);

    String title = "MP3 SHUFFLE";
    tft.setCursor(6, 6);
    tft.print(title);

    if (total > 0) {
        String counter = String(current + 1) + "/" + String(total);
        int w = counter.length() * 6;
        tft.setCursor(tftWidth - w - 6, 6);
        tft.print(counter);
    }
}

static void mp3DrawTrackInfo(const String &trackName, const String &status) {
    int y = 30;
    tft.fillRect(0, y, tftWidth, 40, bruceConfig.bgColor);

    tft.setTextColor(TFT_WHITE, bruceConfig.bgColor);
    tft.setTextSize(2);

    String display = trackName;
    int maxChars = (tftWidth - 12) / 12; // textSize(2) ~ 12px per char
    if (display.length() > (size_t)maxChars) { display = display.substring(0, maxChars - 3) + "..."; }

    tft.setCursor(6, y + 4);
    tft.print(display);

    tft.setTextSize(1);
    tft.setTextColor(status == "[FAILED]" ? TFT_RED : TFT_LIGHTGREY, bruceConfig.bgColor);
    tft.setCursor(6, y + 26);
    tft.print(status);
}

static void mp3DrawFooter() {
    int y = tftHeight - 16;
    tft.fillRect(0, y, tftWidth, 16, bruceConfig.bgColor);
    tft.setTextColor(TFT_DARKGREY, bruceConfig.bgColor);
    tft.setTextSize(1);
    tft.setCursor(6, y + 4);
    tft.print("<Prev  OK:Pause  Next>  ESC:Stop");
}

// ===== MAIN ENTRY POINT =====

void mp3ShufflePlayerUI() {
    if (!sdcardMounted) { setupSdCard(); }

    if (!sdcardMounted) {
        displayError("SD Card not found", true);
        return;
    }

    std::vector<String> tracks = mp3ScanFolder();
    if (tracks.empty()) {
        displayError("No .mp3 files in /mp3", true);
        return;
    }

    std::vector<int> order(tracks.size());
    for (size_t i = 0; i < order.size(); i++) order[i] = (int)i;
    mp3ShuffleOrder(order);

    int pos = 0;
    bool paused = false;
    bool exitPlayer = false;
    bool lastStartOk = false;
    int consecutiveFailures = 0;
    // If every single track in the folder fails to play in a row, stop
    // instead of hammering the SD/SPI bus in a tight, delay-less loop.
    const int MAX_CONSECUTIVE_FAILURES = (int)tracks.size() + 2;

    auto startTrack = [&](int p) {
        String path = tracks[order[p]];
        lastStartOk = playAudioFile(&SD, path, PLAYBACK_ASYNC);
        paused = false;
        tft.fillScreen(bruceConfig.bgColor);
        mp3DrawHeader(p, (int)order.size());
        mp3DrawTrackInfo(mp3ExtractName(path), lastStartOk ? "[PLAYING]" : "[FAILED]");
        mp3DrawFooter();

        // Give the SD/SPI bus a moment to settle between file switches,
        // whether the file played or failed - prevents runaway retry loops.
        // When a track fails, hold the FAILED status on screen a bit longer
        // so it's actually readable instead of flashing past.
        delay(lastStartOk ? 150 : 500);

        if (lastStartOk) {
            consecutiveFailures = 0;
        } else {
            consecutiveFailures++;
        }
    };

    startTrack(pos);

    while (!exitPlayer) {
        InputHandler();
        wakeUpScreen();

        // Button checks ALWAYS run first, regardless of playback state,
        // so the user can exit/skip even if every track fails to play.
        if (check(EscPress)) {
            stopAudioPlayback();
            delay(50); // <--- Dodane: Czas dla FreeRTOS na zwolnienie taska
            exitPlayer = true;
            break;
        }

        if (check(NextPress)) {
            stopAudioPlayback();
            delay(50); // <--- Dodane: Czas dla FreeRTOS na zwolnienie taska
            pos++;
            if (pos >= (int)order.size()) {
                mp3ShuffleOrder(order);
                pos = 0;
            }
            startTrack(pos);
            continue;
        }

        if (check(PrevPress)) {
            stopAudioPlayback();
            delay(50); // <--- Dodane: Czas dla FreeRTOS na zwolnienie taska
            pos = (pos - 1 + (int)order.size()) % (int)order.size();
            startTrack(pos);
            continue;
        }

        if (check(SelPress)) {
            pauseAudioPlayback();
            paused = !paused;
            mp3DrawTrackInfo(mp3ExtractName(tracks[order[pos]]), paused ? "[PAUSED]" : "[PLAYING]");
        }

        // CYKLICZNA ZMIANA GŁOŚNOŚCI PRZYCISKIEM (0 -> 25 -> 50 -> 75 -> 100 -> 0)
        if (check(UpPress)) {
            AudioPlaybackInfo info = getAudioPlaybackInfo();
            int vol = info.volume;

            // Dopasowanie do kolejnego progu lub wyzerowanie, jeśli jest na 100
            if (vol < 25) vol = 25;
            else if (vol < 50) vol = 50;
            else if (vol < 75) vol = 75;
            else if (vol < 100) vol = 100;
            else vol = 0; // Kiedy osiągnie 100, wraca na 0

            setAudioPlaybackVolume(vol);

            // Aktualizacja UI (tymczasowe pokazanie głośności)
            mp3DrawTrackInfo(mp3ExtractName(tracks[order[pos]]), "[VOL: " + String(vol) + "%]");

            // Debouncing - zapobiega przeskoczeniu kilku poziomów przy jednym wciśnięciu
            delay(150);
            continue;
        }

        if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
            stopAudioPlayback();
            displayError("Nie mozna odtworzyc plikow z /mp3", true);
            break;
        }

        // Track finished naturally (or failed to start) -> advance
        AudioPlaybackInfo info = getAudioPlaybackInfo();
        if (!paused && info.state == PLAYBACK_IDLE) {
            delay(50); // <--- Dodane: Czas dla FreeRTOS na zwolnienie taska
            pos++;
            if (pos >= (int)order.size()) {
                mp3ShuffleOrder(order);
                pos = 0;
            }
            startTrack(pos);
            continue;
        }

        delay(30);
    }

    tft.fillScreen(bruceConfig.bgColor);
}

#endif // HAS_NS4168_SPKR
