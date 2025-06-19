// #include "alloc.h"
#include "engine.h"
#include "platform.h"
#include "render.h"
#include "utils.h"

texture_t RENDER_NO_TEXTURE;

struct {
  vec2i_t size;
  rgba_t *pixels;
} textures[RENDER_TEXTURES_MAX];

uint32_t textures_len = 0;

static rgba_t *screen_buffer;
static int32_t screen_pitch;
static int32_t screen_ppr;
static vec2i_t screen_size;

void render_backend_init(void) {}
void render_backend_cleanup(void) {}

void render_set_screen(vec2i_t size) { screen_size = size; }

void render_set_blend_mode(render_blend_mode_t mode) {
  // TODO
}

void render_set_post_effect(render_post_effect_t post) {
  // TODO
}

void render_frame_prepare(void) {
  screen_buffer = platform_get_screenbuffer(&screen_pitch);
  screen_ppr = screen_pitch / sizeof(rgba_t);
  memset(screen_buffer, 0, screen_size.y * screen_pitch);
}

void render_frame_end(void) {}

void render_draw_quad(quadverts_t *quad, texture_t texture_handle) {
  error_if(texture_handle.index >= textures_len, "Invalid texture %d",
           texture_handle.index);

  // FIXME: this only handles axis aligned quads; rotation/shearing is not
  // supported.

  vertex_t *v = quad->vertices;
  rgba_t color = v[0].color;

  int dx = v[0].pos.x;
  int dy = v[0].pos.y;
  int dw = v[2].pos.x - dx;
  int dh = v[2].pos.y - dy;

  vec2i_t src_size = textures[texture_handle.index].size;
  rgba_t *src_px = textures[texture_handle.index].pixels;

  vec2i_t uv_tl = vec2i_from_vec2(v[0].uv);
  uv_tl.x = clamp(uv_tl.x, 0, src_size.x);
  uv_tl.y = clamp(uv_tl.y, 0, src_size.y);

  vec2i_t uv_br = vec2i_from_vec2(v[2].uv);
  uv_br.x = clamp(uv_br.x, 0, src_size.x);
  uv_br.y = clamp(uv_br.y, 0, src_size.y);

  float sx = uv_tl.x;
  float sy = uv_tl.y;
  float sw = uv_br.x - sx;
  float sh = uv_br.y - sy;

  float sx_inc = sw / dw;
  float sy_inc = sh / dh;

  // Clip to screen
  if (dx < 0) {
    sx += sx_inc * -dx;
    dw += dx;
    dx = 0;
  }
  if (dx + dw >= screen_size.x) {
    dw = screen_size.x - dx;
  }
  if (dy < 0) {
    sy += sy_inc * -dy;
    dh += dy;
    dy = 0;
  }
  if (dy + dh >= screen_size.y) {
    dh = screen_size.y - dy;
  }

  // FIXME: There's probably an underflow in the source data when
  // sx_inc or sy_inc is negative?!
  int di = dy * screen_ppr + dx;
  for (int y = 0; y < dh; y++, di += screen_ppr - dw) {
    // fudge source index by 0.001 pixels to avoid rounding errors :/
    float si = floor(sy + y * sy_inc) * src_size.x + sx + 0.001;
    for (int x = 0; x < dw; x++, si += sx_inc, di++) {
      screen_buffer[di] =
          rgba_blend(screen_buffer[di], rgba_mix(src_px[(int)si], color));
    }
  }
}

texture_mark_t textures_mark(void) {
  return (texture_mark_t){.index = textures_len};
}

void textures_reset(texture_mark_t mark) {
  error_if(mark.index > textures_len, "Invalid texture reset mark %d >= %d",
           mark.index, textures_len);
  textures_len = mark.index;
}

texture_t texture_create(vec2i_t size, rgba_t *pixels) {
  error_if(textures_len >= RENDER_TEXTURES_MAX, "RENDER_TEXTURES_MAX reached");

  textures[textures_len].size = size;
  textures[textures_len].pixels = malloc(sizeof(rgba_t) * size.x * size.y);
  memcpy(textures[textures_len].pixels, pixels,
         sizeof(rgba_t) * size.x * size.y);

  texture_t texture_handle = {.index = textures_len};
  textures_len++;
  return texture_handle;
}

void texture_replace_pixels(texture_t texture_handle, vec2i_t size,
                            rgba_t *pixels) {
  error_if(texture_handle.index >= textures_len, "Invalid texture %d",
           texture_handle.index);

  vec2i_t dst_size = textures[texture_handle.index].size;
  rgba_t *dst_px = textures[texture_handle.index].pixels;
  error_if(dst_size.x < size.x || dst_size.y < size.y,
           "Cannot replace %dx%d pixels of %dx%d texture", size.x, size.y,
           dst_size.x, dst_size.y);

  int di = 0;
  int si = 0;
  for (int y = 0; y < size.y; y++, di += dst_size.x - size.x) {
    for (int x = 0; x < size.x; x++, si++, di++) {
      dst_px[di] = pixels[si];
    }
  }
}

rgba_t _colors[64] = {
  rgba(0, 0, 0, 255),
  rgba(255, 255, 255, 255),
  rgba(255, 0, 0, 255),
  rgba(0, 255, 0, 255),
  rgba(0, 0, 255, 255),
  rgba(255, 255, 0, 255),
  rgba(0, 255, 255, 255),
  rgba(255, 0, 255, 255)
};

void render_set_colors(float *colors, int count) {
  int idx = 0;
  for(int i=0; i<(3*count); i+=3) {
    int r = 255 * colors[i];
    int g = 255 * colors[i+1];
    int b = 255 * colors[i+2];
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    _colors[idx++] = rgba(r, g, b, 255);
  }
}

void render_draw_rect_fill(vec2_t v1, vec2_t v2, int color) {
  int dx = v1.x;
  int dy = v1.y;
  int dw = v2.x - v1.x;
  int dh = v2.y - v1.y;

  float sz = 2;
  dx *= sz;
  dy *= sz;
  dw *= sz;
  dh *= sz;

  // Clip to screen
  if (dx < 0) {
    dw += dx;
    dx = 0;
  }
  if (dx + dw >= screen_size.x) {
    dw = screen_size.x - dx;
  }
  if (dy < 0) {
    dh += dy;
    dy = 0;
  }
  if (dy + dh >= screen_size.y) {
    dh = screen_size.y - dy;
  }

  rgba_t clr = _colors[color];

  int di = dy * screen_ppr + dx;
  for (int y = 0; y < dh; y++, di += screen_ppr - dw) {
    for (int x = 0; x < dw; x++, di++) {
      screen_buffer[di] = rgba_blend(screen_buffer[di], clr);
    }
  }
}

void render_draw_triangle(vec2_t v1, vec2_t v2, vec2_t v3, int color) {
  render_draw_line(v1, v2, color);
  render_draw_line(v2, v3, color);
  render_draw_line(v3, v1, color);
}

void render_draw_triangle_fill(vec2_t v1, vec2_t v2, vec2_t v3, int color) {
  vec2_t d1 = vec2_sub(v3, v1);
  vec2_t d2 = vec2_sub(v2, v1);
  float l1 = vec2_len(d1);
  float l2 = vec2_len(d2);

  float d = l1 < l2 ? l1 : l2;
  float pr = 2;
  d1.x /= (d*pr);
  d1.y /= (d*pr);
  d2.x /= (d*pr);
  d2.y /= (d*pr);

  vec2_t p1 = v1;
  vec2_t p2 = v1;
  for(float i=0; i<(d*pr); i++) {
    render_draw_line(p1, p2, color);
    p1 = vec2_add(p1, d1);
    p2 = vec2_add(p2, d2);
  }
}

void render_draw_line(vec2_t v1, vec2_t v2, int color) {
  float sz = 2;

  v1.x *= sz;
  v1.y *= sz;
  v2.x *= sz;
  v2.y *= sz;

  // clip!
  if (v1.x < 0 || v1.y < 0 || v2.x < 0 || v2.y < 0) {
    return;
  }

  vec2_t dir = vec2_sub(v2, v1);
  float l = vec2_len(dir);
  dir.x /= l;
  dir.y /= l;

  rgba_t clr = _colors[color];

  int px = -1;
  int py = -1;

  vec2_t p = v1;
  for(int i=0; i<l; i++) {
    p = vec2_add(p, dir);
    if (p.x == px && p.y == py) continue;
    px = (int)p.x;
    py = (int)p.y;
    if (px < 0) break;
    if (py < 0) break;
    if (px >= screen_size.x) break;
    if (py >= screen_size.y) break;
    int di = (py * screen_size.x) + px;
    screen_buffer[di] = rgba_blend(screen_buffer[di], clr);
  }
}

void render_draw_rect(vec2_t v1, vec2_t v2, int color) {
  vec2_t v3 = vec2(v2.x, v1.y);
  vec2_t v4 = vec2(v1.x, v2.y);

  render_draw_line(v1, v3, color);
  render_draw_line(v3, v2, color);
  render_draw_line(v4, v2, color);
  render_draw_line(v1, v4, color);
}

void render_draw_polygon(vec2_t *points, int count, int color) {
  for(int i=0; i<count; i++) {
    int ii = (i + 1) % count;
    vec2_t v1 = points[i];
    vec2_t v2 = points[ii];
    render_draw_line(v1, v2, color);
  }
}

void render_draw_polygon_fill(vec2_t *points, int count, int color) {
  // render_draw_polygon(points, count, color);

  triangle_t *tris = NULL;
  int num_tris = triangulate_polygon(points, count, &tris);
  for(int i=0; i<num_tris; i++) {
    vec2_t v1 = points[tris[i].a];
    vec2_t v2 = points[tris[i].b];
    vec2_t v3 = points[tris[i].c];
    render_draw_triangle_fill(v1, v2, v3, color);
  }
}

void render_draw_circle(vec2_t pos, int radius, int color) {
  vec2_t points[32];
  int sides = 16;
  for(int i=0; i<sides; i++) {
    float angle = ((360 / sides) * i) * 3.14 / 180;
    points[i] = vec2_add(pos, vec2_mulf(vec2_from_angle(angle), radius));
  }
  render_draw_polygon(points, sides, color);
}

void render_draw_circle_fill(vec2_t pos, int radius, int color) {
  vec2_t points[32];
  int sides = 16;
  for(int i=0; i<sides; i++) {
    float angle = ((360 / sides) * i) * 3.14 / 180;
    points[i] = vec2_add(pos, vec2_mulf(vec2_from_angle(angle), radius));
  }
  render_draw_polygon_fill(points, sides, color);
}

void render_clear(int color) {
  render_draw_rect_fill(vec2(0, 0), vec2(screen_size.x, screen_size.y), color);
}
