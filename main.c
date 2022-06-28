extern void print(int x);
extern void putchar(char x);
extern double sin(double x);
extern double cos(double x);
extern unsigned char __heap_base;

#define WASM_EXPORT __attribute__((visibility("default")))

#include <stdint.h>
#define BLOCK_SIZE (1 << 16)

/* player stuff */
static struct {
  double x, y, rot;
} player;

typedef struct { uint8_t r, g, b, a; } Pixel;
static struct {
  int width, height;
  Pixel *pixels;
} rendr;
#define RENDR_PIXELS_LEN (rendr.width * rendr.height * 4)

int WASM_EXPORT init(int width, int height) {

  /* center the player */
  player.x = width/2;
  player.y = height/2;

  /* initialize the renderer */
  {
    rendr.width = width;
    rendr.height = height;
    rendr.pixels = (void *) &__heap_base + BLOCK_SIZE;

    /* make sure we have enough space for these pixels */
    __builtin_wasm_memory_grow(0, 1+RENDR_PIXELS_LEN/BLOCK_SIZE);
  }

  return (int)rendr.pixels;
}

static void fill_pixel(int x, int y, Pixel color) {
  if (x > 0 && x < rendr.width && y > 0 && y < rendr.height)
    rendr.pixels[y*rendr.width + x] = color;
}

/* "static" functions are _internal_ and won't be exposed to JS */
typedef struct {
  int px, py, size_x, size_y;
  double angle;
  Pixel color;
} DrawRect;

static void draw_rect(DrawRect dr) {
  double cosangle = cos(dr.angle);
  double sinangle = sin(dr.angle);
  for (int x = 0; x < dr.size_x*2; x++)
    for (int y = 0; y < dr.size_y*2; y++) {
      double fx = cosangle * (x - dr.size_x) - (y - dr.size_y) * sinangle;
      double fy = sinangle * (x - dr.size_x) + (y - dr.size_y) * cosangle;
      fill_pixel(dr.px + (int)fx - dr.size_x,
                 dr.py + (int)fy - dr.size_y,
                 dr.color);
    }
}

void WASM_EXPORT draw(double dt) {
  /* draw the background */
  __builtin_memset(rendr.pixels, 255, RENDR_PIXELS_LEN);

  /* draw the player */
  draw_rect((DrawRect) {
    .px = player.x,
    .py = player.y,
    .size_x = 10,
    .size_y = 10,
    .angle = player.rot,
    .color.r = 128,
    .color.g =  20,
    .color.b = 128,
    .color.a = 255
  });

  int ox = cos(player.rot)*7;
  int oy = sin(player.rot)*7;
  draw_rect((DrawRect) {
    .px = player.x     + ox,
    .py = player.y - 5 + oy,
    .size_x = 10,
    .size_y =  5,
    .angle = player.rot,
    .color.r = 128,
    .color.g = 128,
    .color.b = 128,
    .color.a = 255
  });
}

void WASM_EXPORT keydown(char k) {
  if (k == 'w') player.x += cos(player.rot)*2, player.y += sin(player.rot)*2;
  if (k == 's') player.x -= cos(player.rot)*2, player.y -= sin(player.rot)*2;

  if (k == 'a') player.rot -= 0.2;
  if (k == 'd') player.rot += 0.2;
}
