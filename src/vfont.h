#ifndef _HP_FONT_H_
#define _HP_FONT_H_

#include <stdint.h>
#include "types.h"

#define FONT_UP 0xFE
#define FONT_LAST 0xFF

void vfont_init();
int *vfont_char_points(char c);
float vfont_draw(char *text, vec2_t pos, float size,
                      int align, int color);

#endif // _HP_FONT_H_
