#pragma once
#include "libretro.h"
#include <algorithm>
#include <array>

namespace Libretro
{
// The engine uses DirectInput-compatible physical key numbers.
inline unsigned ScanCode(unsigned key)
{
    static const char letters[] = "abcdefghijklmnopqrstuvwxyz";
    static const unsigned scans[] = {30,48,46,32,18,33,34,35,23,36,37,38,50,49,24,25,16,19,31,20,22,47,17,45,21,44};

    for (unsigned i = 0; i < 26; ++i)
        if (key == unsigned(letters[i]))
            return scans[i];

    if (key >= RETROK_1 && key <= RETROK_9)
        return 2 + key - RETROK_1;

    if (key >= RETROK_F1 && key <= RETROK_F10)
        return 59 + key - RETROK_F1;

    switch (key)
    {
    case RETROK_0:
        return 11;
    case RETROK_ESCAPE:
        return 1;
    case RETROK_MINUS:
        return 12;
    case RETROK_EQUALS:
        return 13;
    case RETROK_BACKSPACE:
        return 14;
    case RETROK_TAB:
        return 15;
    case RETROK_LEFTBRACKET:
        return 26;
    case RETROK_RIGHTBRACKET:
        return 27;
    case RETROK_RETURN:
        return 28;
    case RETROK_LCTRL:
        return 29;
    case RETROK_SEMICOLON:
        return 39;
    case RETROK_QUOTE:
        return 40;
    case RETROK_BACKQUOTE:
        return 41;
    case RETROK_LSHIFT:
        return 42;
    case RETROK_BACKSLASH:
        return 43;
    case RETROK_COMMA:
        return 51;
    case RETROK_PERIOD:
        return 52;
    case RETROK_SLASH:
        return 53;
    case RETROK_RSHIFT:
        return 54;
    case RETROK_LALT:
        return 56;
    case RETROK_SPACE:
        return 57;
    case RETROK_CAPSLOCK:
        return 58;
    case RETROK_F11:
        return 87;
    case RETROK_F12:
        return 88;
    case RETROK_RCTRL:
        return 157;
    case RETROK_RALT:
        return 184;
    case RETROK_HOME:
        return 199;
    case RETROK_UP:
        return 200;
    case RETROK_PAGEUP:
        return 201;
    case RETROK_LEFT:
        return 203;
    case RETROK_RIGHT:
        return 205;
    case RETROK_END:
        return 207;
    case RETROK_DOWN:
        return 208;
    case RETROK_PAGEDOWN:
        return 209;
    case RETROK_INSERT:
        return 210;
    case RETROK_DELETE:
        return 211;
    default:
        return 0;
    }
}
inline void PollKeys(retro_input_state_t input, unsigned (*binding)(unsigned),
    bool padEnabled, bool capture, std::array<bool, RETROK_LAST> &keys,
    std::array<bool, 256> &next, std::array<unsigned, 256> &characters)
{
    for (unsigned key = 0; key < keys.size(); ++key)
    {
        keys[key] = input(0, RETRO_DEVICE_KEYBOARD, 0, key) != 0;
        auto scan = ScanCode(key);
        if (scan)
        {
            next[scan] = next[scan] || keys[key];
            if (key < 128) characters[scan] = key;
        }
    }

    // Frontends may map the same physical keyboard to both devices (e.g.
    // RetroArch maps W to R and A to Y). The API provides no provenance for
    // joypad buttons, so give held keyboard keys priority over digital pad input.
    const bool keyboardActive = std::any_of(keys.begin(), keys.end(), [](bool down) { return down; });

    if (padEnabled && !keyboardActive) for (unsigned button = 0; button < 16; ++button)
    {
        auto scan = binding(button);
        if (capture && button == RETRO_DEVICE_ID_JOYPAD_A)
            scan = 28;

        if (capture && button == RETRO_DEVICE_ID_JOYPAD_B)
            scan = 1;

        if (!scan || scan >= next.size())
            continue;

        next[scan] = next[scan] || input(0, RETRO_DEVICE_JOYPAD, 0, button) != 0;
    }

}
}
