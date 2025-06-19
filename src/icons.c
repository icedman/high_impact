#include "icons.h"
#include "image.h"
#include "render.h"

static image_t *sprite = NULL;
static image_t *sprite_black = NULL;

void icons_init() {
	sprite = image("assets/icons.qoi");
	sprite_black = image("assets/icons_black.qoi");
}

void icons_draw(vec2_t v1, vec2_t v2, int icon, int color) {
	vec2i_t img_size = image_size(sprite);
	vec2i_t tile_size = vec2i(24, 24);
	int tile = icon;
	vec2_t src_pos = vec2((tile * tile_size.x) % img_size.x,
	                    ((tile * tile_size.x) / img_size.x) * tile_size.y);
	vec2_t src_size = vec2(tile_size.x, tile_size.y);

	int tw = v2.x-v1.x;
	if (tw < 24) {
		v1.x -= (24 - tw)/2;
		v1.y -= (24 - tw)/2;
		tw = 24;
	}
	image_draw_ex(color ? sprite : sprite_black, src_pos, src_size,
                   v1, vec2(tw, tw), rgba_white());
}