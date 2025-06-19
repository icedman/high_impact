#ifndef _HP_FONT_H_
#define _HP_FONT_H_

#include <stdint.h>

#define FONT_UP 0xFE
#define FONT_LAST 0xFF

void vfont_init();
int *vfont_char_points(char c);

#endif // _HP_FONT_H_
