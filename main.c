extern void print(int x);
extern void putchar(char x);
extern double sin(double x);
extern double cos(double x);
extern unsigned char __heap_base;

#define WASM_EXPORT __attribute__((visibility("default")))

#include <stdint.h>
#define BLOCK_SIZE (1 << 16)

/* player stuff */
static int pos_x = 0;
static int pos_y = 0;

typedef struct { uint8_t r, g, b, a; } Pixel;
static struct {
  int width, height;
  Pixel *pixels;
} rendr;
#define RENDR_PIXELS_LEN (rendr.width * rendr.height * 4)

int WASM_EXPORT init(int width, int height) {

  /* center the player */
  pos_x = width/2;
  pos_y = height/2;

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
  int px, py, size;
  double angle;
  Pixel color;
} DrawRect;

static void draw_rect(DrawRect dr) {
  double cosangle = cos(dr.angle);
  double sinangle = sin(dr.angle);
  for (int x = 0; x < dr.size*2; x++)
    for (int y = 0; y < dr.size*2; y++) {
      double fx = cosangle * (x - dr.size) - (y - dr.size) * sinangle;
      double fy = sinangle * (x - dr.size) + (y - dr.size) * cosangle;
      fill_pixel(dr.px + (int)fx - dr.size,
                 dr.py + (int)fy - dr.size,
                 dr.color);
    }
}

void WASM_EXPORT draw(double dt) {
  /* draw the background */
  __builtin_memset(rendr.pixels, 255, RENDR_PIXELS_LEN);

  /* draw the player */
  draw_rect((DrawRect) {
    .px = pos_x,
    .py = pos_y,
    .size = 10,
    .angle = dt * 0.001,
    .color.r = 128,
    .color.g =  20,
    .color.b = 128,
    .color.a = 255
  });
}

void WASM_EXPORT keydown(char k) {
  if (k == 'w') pos_y -= 5;
  if (k == 's') pos_y += 5;
  if (k == 'a') pos_x -= 5;
  if (k == 'd') pos_x += 5;
}
