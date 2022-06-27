extern void print(int x);
extern void putchar(char x);
#include <stdint.h>
#define BLOCK_SIZE (1 << 16)

static int pos_x = 0;
static int pos_y = 0;

static struct {
  int width, height, *pixels;
} rendr;
#define RENDR_PIXELS_LEN (rendr.width * rendr.height * 4)

void init(int width, int height) {
  __builtin_wasm_memory_grow(0, RENDR_PIXELS_LEN/BLOCK_SIZE);
}

void draw(uint8_t *ptr) {
  __builtin_memset(rendr.pixels, 255, RENDR_PIXELS_LEN);
}

void keydown(char k) {
  putchar(k);
}
