#ifndef FRAME_H
#define FRAME_H

#include "image.h"
#include "types.h"

void frame_init();
void frame_draw(vec2_t v1, vec2_t v2, int frame_style, int color);

#endif
