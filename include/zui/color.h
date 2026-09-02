#ifndef ZUI_COLOR_H
#define ZUI_COLOR_H

typedef struct ZuiColor {
    float r, g, b, a;
} ZuiColor;

#define ZUI_COLOR(r, g, b, a) ((ZuiColor){(r), (g), (b), (a)})
#define ZUI_COLOR_RGB(r, g, b) ZUI_COLOR((r), (g), (b), 1.0f)
#define ZUI_COLOR_HEX(hex) ZUI_COLOR( \
    (((hex) >> 16) & 0xFF) / 255.0f, \
    (((hex) >> 8) & 0xFF) / 255.0f, \
    ((hex) & 0xFF) / 255.0f, 1.0f)

#endif
