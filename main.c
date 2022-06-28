extern void print(int x);
extern void putchar(char x);
extern unsigned char __heap_base;

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

void init(int width, int height) {

  /* center the player */
  pos_x = width/2;
  pos_y = height/2;

  /* initialize the renderer */
  {
    rendr.width = width;
    rendr.height = height;
    rendr.pixels = (void *) &__heap_base;

    /* make sure we have enough space for these pixels */
    __builtin_wasm_memory_grow(0, RENDR_PIXELS_LEN/BLOCK_SIZE);
  }
}

/* "static" functions are _internal_ and won't be exposed to JS */
static void draw_rect(int px, int py, int size, Pixel color) {
  for (int x = px-5; x < px+5; x++)
    for (int y = py-5; y < py+5; y++)
      if (x < rendr.width && y < rendr.height)
        rendr.pixels[y*rendr.width + x] = color;
}

void draw() {
  /* clear the screen */
  __builtin_memset(rendr.pixels, 255, RENDR_PIXELS_LEN);

  draw_rect(pos_x, pos_y, 10, (Pixel) {
    .r = 128,
    .g =  20,
    .b = 128,
    .a = 255
  });
}

void keydown(char k) {
  if (k == 'w') pos_y -= 5;
  if (k == 's') pos_y += 5;
  if (k == 'a') pos_x -= 5;
  if (k == 'd') pos_x += 5;
}
