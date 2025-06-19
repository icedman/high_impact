#include "pd_api.h"
#include "render.h"

#define PATTERN_IMPLEMENTATION
#include "playdate_patterns.h"

static texture_t texture;
static int textures_len = 0;
static vec2i_t texture_sizes[RENDER_TEXTURES_MAX];
static LCDBitmap *textures[RENDER_TEXTURES_MAX];

extern PlaydateAPI *playdate;

texture_t RENDER_NO_TEXTURE;

// float rgbToLuminance(unsigned int red, unsigned int green, unsigned int blue)
// {
//   float luminance = 0.299f * red + 0.587f * green + 0.114f * blue;
//   float normalizedLuminance = luminance / 255.0f;
//   return normalizedLuminance;
// }

// LCDColor rgbToColor(unsigned int red, unsigned int green, unsigned int blue)
// {
//   float luminance = rgbToLuminance(red, green, blue);
//   int color_range = 5;
//   int index = roundf(luminance * color_range);
//   if (index >= color_range) {
//     index = color_range - 1;
//   }
//   return (LCDColor)&intensity[index];
// }

static vec2i_t screen_size;

void render_clear(int color) {
  vec2_t p1 = {0,0};
  vec2_t p2 = {screen_size.x, screen_size.y};
  render_draw_rect_fill(p1, p2, color);
}

void render_backend_init(void) { printf("render_backend_init\n"); }

void render_backend_cleanup(void) {}

void render_set_screen(vec2i_t size) { screen_size = size; }

void render_set_blend_mode(render_blend_mode_t mode) {}
void render_set_post_effect(render_post_effect_t post) {}

void render_frame_prepare(void) {}

void render_frame_end(void) {}

void render_draw_quad(quadverts_t *quad, texture_t texture_handle) {
  PlaydateAPI *pd = playdate;

  vertex_t *v = quad->vertices;
  // rgba_t color = v[0].color;

  // float l = rgbToLuminance(color.r, color.g, color.b);

  // int coords[8] = {
  //  v[0].pos.x, v[0].pos.y,
  //  v[1].pos.x, v[1].pos.y,
  //  v[2].pos.x, v[2].pos.y,
  //  v[3].pos.x, v[3].pos.y,
  // };
  // LCDColor clr = get_pattern(texture_handle.index % 4);
  // pd->graphics->fillPolygon(4, coords, clr, kPolygonFillNonZero);

  // for(int i=0; i<4; i++) {
  //   vertex_t v1 = v[i];
  //   vertex_t v2 = v[(i+1)%4];
  //   int x1 = v1.pos.x;
  //   int y1 = v1.pos.y;
  //   int x2 = v2.pos.x;
  //   int y2 = v2.pos.y;
  //   pd->graphics->drawLine(x1, y1, x2, y2, 1, get_pattern(0));
  // }

  LCDBitmap *texture = textures[texture_handle.index];
  if (!texture) {
    return;
  }
  vec2i_t texture_size = texture_sizes[texture_handle.index];
  int src_tiles = texture_size.x / texture_size.y;

  int dx = v[0].pos.x;
  int dy = v[0].pos.y;
  vec2_t dst_size = vec2(v[1].pos.x - v[0].pos.x, v[2].pos.y - v[1].pos.y);
  vec2_t src_size = vec2(v[1].uv.x - v[0].uv.x, v[2].uv.y - v[1].uv.y);

  // printf("%f %f\n", dx, dy, v[0].uv.x, v[0].uv.y);
  // printf("%d %d\n", texture_size.x, texture_size.y);

  double sz = vec2_len(dst_size) / vec2_len(src_size);

  int sx = v[0].uv.x * sz;
  int sy = v[0].uv.y * sz;

  // float ss = 1;
  // for(int i=0; i<4; i++) {
  //   vertex_t v1 = v[i];
  //   vertex_t v2 = v[(i+1)%4];
  //   int x1 = floor((v1.pos.x + (ss/2)) / ss) * ss;
  //   int y1 = floor((v1.pos.y + (ss/2)) / ss) * ss;
  //   int x2 = floor((v2.pos.x + (ss/2)) / ss) * ss;
  //   int y2 = floor((v2.pos.y + (ss/2)) / ss) * ss;
  //   pd->graphics->drawLine(x1, y1, x2, y2, 1, kColorWhite);
  // }

  int fX = v[0].uv.x > v[1].uv.x ? -1 : 1;
  if (fX == -1) {
    sx = (texture_size.x - v[0].uv.x) * sz;
  }

  pd->graphics->setClipRect(dx, dy, dst_size.x, dst_size.y);
  pd->graphics->drawScaledBitmap(texture, dx - sx, dy - sy, sz * fX, sz);
  pd->graphics->clearClipRect();
}

texture_mark_t textures_mark(void) {
  return (texture_mark_t){.index = textures_len};
}
void textures_reset(texture_mark_t mark) {}

texture_t texture_create(vec2i_t size, rgba_t *pixels) {
  texture.index = textures_len;
  texture_sizes[textures_len] = size;
  textures[textures_len] = (void *)pixels;
  textures_len++;
  if (textures_len >= RENDER_TEXTURES_MAX) {
    printf("-----------------------\n", textures_len);
    printf(":: textures overflow! %d\n", textures_len);
    printf("-----------------------\n", textures_len);
  }
  return texture;
}
void texture_replace_pixels(texture_t texture_handle, vec2i_t size,
                            rgba_t *pixels) {}

void render_draw_rect(vec2_t v1, vec2_t v2, int color) {
  PlaydateAPI *pd = playdate;
  int x1 = (int)v1.x;
  int y1 = (int)v1.y;
  int x2 = (int)v2.x;
  int y2 = (int)v2.y;
  int width = 2;

  LCDColor kColor = kColorWhite;
  if (color == 0) {
    kColor = kColorBlack;
  }

  // pd->graphics->fillRect(x1, y1, x2 - x1, y2 - y1, kBgColor);
  pd->graphics->drawRect(x1, y1, x2 - x1, y2 - y1, kColor);
}

void render_draw_rect_fill(vec2_t v1, vec2_t v2, int color) {
  PlaydateAPI *pd = playdate;
  int x1 = (int)v1.x;
  int y1 = (int)v1.y;
  int x2 = (int)v2.x;
  int y2 = (int)v2.y;
  int width = 2;

  LCDColor kColor = kColorWhite;
  if (color == 0) {
    kColor = kColorBlack;
  }

  if (color > 1) {
    kColor = get_pattern(color);
  }

  pd->graphics->fillRect(x1, y1, x2 - x1, y2 - y1, kColor);
}

void render_draw_triangle(vec2_t v1, vec2_t v2, vec2_t v3, int color) {
  PlaydateAPI *pd = playdate;
  int width = 1;

  LCDColor kColor = kColorWhite;
  if (color == 0) {
    kColor = kColorBlack;
  }

  pd->graphics->drawLine(v1.x, v1.y, v2.x, v2.y, width, kColor);
  pd->graphics->drawLine(v2.x, v2.y, v3.x, v3.y, width, kColor);
  pd->graphics->drawLine(v3.x, v3.y, v1.x, v1.y, width, kColor);
}

void render_draw_triangle_fill(vec2_t v1, vec2_t v2, vec2_t v3, int color) {
  PlaydateAPI *pd = playdate;
  int width = 1;

  LCDColor kColor = kColorWhite;
  if (color == 0) {
    kColor = kColorBlack;
  }

  if (color > 1) {
    kColor = get_pattern(color);
  }

  pd->graphics->fillTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, kColor);
}

void render_draw_line(vec2_t v1, vec2_t v2, int color) {
  PlaydateAPI *pd = playdate;
  int x1 = (int)v1.x;
  int y1 = (int)v1.y;
  int x2 = (int)v2.x;
  int y2 = (int)v2.y;
  int width = 1;

  LCDColor kColor = kColorWhite;
  if (color == 0) {
    kColor = kColorBlack;
  }

  pd->graphics->drawLine(x1, y1, x2, y2, width, kColor);
}

void render_draw_polygon(vec2_t *points, int count, int color) {
  for(int i=0; i<count; i++) {
    int ii = (i + 1) % count;
    vec2_t v1 = points[i];
    vec2_t v2 = points[ii];
    render_draw_line(v1, v2, color);
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

void render_draw_polygon_fill(vec2_t *points, int count, int color) {
  render_draw_polygon(points, count, color);
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

void render_set_colors(float *colors, int count) {
  
}