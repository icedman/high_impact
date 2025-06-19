#include "frame.h"
#include "image.h"
#include "render.h"

static image_t *sprite = NULL;

void frame_init() {
	sprite = image("assets/frames.qoi");
}

void frame_draw(vec2_t v1, vec2_t v2, int frame_style, int color) {
	vec2i_t img_size = image_size(sprite);
	vec2i_t tile_size = vec2i(32, 32);
	int tile = frame_style;
	vec2_t src_pos = vec2((tile * tile_size.x) % img_size.x,
	                    ((tile * tile_size.x) / img_size.x) * tile_size.y);
	vec2_t src_size = vec2(tile_size.x, tile_size.y);

	// image_draw_ex(sprite, src_pos, src_size,
    //                v1, vec2(tile_size.x, tile_size.y), rgba_white());

	vec2_t slice_size = vec2(8, 8);

	if (color == 1) {
		image_draw_ex(sprite, vec2(src_pos.x + 8 * 1, src_pos.y + 8 * 1), 
					vec2(16, 16),
                   vec2(v1.x + 8 * 1, v1.y + 8 * 1),
                   vec2(v2.x - v1.x - 16, v2.y - v1.y - 16), rgba_white());		
	}

	for(int i=v1.x; i<v2.x; i+=8) {
		// top
		image_draw_ex(sprite, vec2(src_pos.x + 8 * 1, src_pos.y), slice_size,
                   vec2(i, v1.y),
                   vec2(slice_size.x, slice_size.y), rgba_white());
		// bottom
		image_draw_ex(sprite, vec2(src_pos.x + 8 * 1, src_pos.y + 8 * 3), slice_size,
                   vec2(i, v2.y),
                   vec2(slice_size.x, slice_size.y), rgba_white());
	}

	for(int i=v1.y; i<v2.y; i+=8) {
		// left
		image_draw_ex(sprite, vec2(src_pos.x, src_pos.y + 8 * 1), slice_size,
	                   vec2(v1.x, i),
	                   vec2(slice_size.x, slice_size.y), rgba_white());

		// right
		image_draw_ex(sprite, vec2(src_pos.x + 8 * 3, src_pos.y + 8 * 1), slice_size,
	                   vec2(v2.x, i),
	                   vec2(slice_size.x, slice_size.y), rgba_white());
	}

	// corners
	image_draw_ex(sprite, src_pos, slice_size,
                   vec2(v1.x, v1.y),
                   vec2(slice_size.x, slice_size.y), rgba_white());

	image_draw_ex(sprite, vec2(src_pos.x + 8 * 3, src_pos.y), slice_size,
                   vec2(v2.x, v1.y),
                   vec2(slice_size.x, slice_size.y), rgba_white());

	image_draw_ex(sprite, vec2(src_pos.x, src_pos.y + 8 * 3), slice_size,
                   vec2(v1.x, v2.y),
                   vec2(slice_size.x, slice_size.y), rgba_white());

	image_draw_ex(sprite, vec2(src_pos.x + 8 * 3, src_pos.y + 8 * 3), slice_size,
                   vec2(v2.x, v2.y),
                   vec2(slice_size.x, slice_size.y), rgba_white());

}