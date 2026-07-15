#ifndef __MP3_SHUFFLE_MENU_H__
#define __MP3_SHUFFLE_MENU_H__

#include "MenuItemInterface.h"

class Mp3ShuffleMenu : public MenuItemInterface {

public:
    Mp3ShuffleMenu() : MenuItemInterface("MP3 Shuffle") {}

    void optionsMenu(void);
    void drawIcon(float scale);

    // No theme/GIF support for this item yet - always falls back to the
    // vector icon drawn in drawIcon().
    bool hasTheme() { return false; }
    String themePath() { return ""; }
};

#endif
