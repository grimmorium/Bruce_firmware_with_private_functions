#include "Mp3ShuffleMenu.h"

#include "core/display.h"
#include "modules/others/mp3_shuffle.h"

void Mp3ShuffleMenu::optionsMenu() {
#if defined(HAS_NS4168_SPKR)
    mp3ShufflePlayerUI();
#else
    displayError("Speaker not supported on this device", true);
#endif
}

void Mp3ShuffleMenu::drawIcon(float scale) {
    clearIconArea();

    // Simple eighth-note glyph + two crossing "shuffle" arrows above it.

    int noteHeadR = scale * 5;
    int stemH = scale * 24;
    int noteX = iconCenterX - scale * 8;
    int noteY = iconCenterY + scale * 10;

    // Note head (filled, slightly tilted look via ellipse-ish circle)
    tft.fillCircle(noteX, noteY, noteHeadR, bruceConfig.priColor);

    // Stem
    int stemX = noteX + noteHeadR - 2;
    tft.fillRect(stemX, noteY - stemH, scale * 2.5, stemH, bruceConfig.priColor);

    // Flag
    tft.fillTriangle(
        stemX + scale * 2.5,
        noteY - stemH,
        stemX + scale * 2.5 + scale * 10,
        noteY - stemH + scale * 6,
        stemX + scale * 2.5,
        noteY - stemH + scale * 14,
        bruceConfig.priColor
    );

    // Crossing "shuffle" arrows
    int sx = iconCenterX + scale * 4;
    int sy = iconCenterY - scale * 14;
    int sw = scale * 18;
    int sh = scale * 10;

    tft.drawWideLine(
        sx - sw / 2, sy, sx + sw / 2, sy + sh, scale * 2, bruceConfig.priColor, bruceConfig.bgColor
    );
    tft.drawWideLine(
        sx - sw / 2, sy + sh, sx + sw / 2, sy, scale * 2, bruceConfig.priColor, bruceConfig.bgColor
    );
}
