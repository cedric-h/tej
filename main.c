extern void print(int x);
#include <stdint.h>
#define BLOCK_SIZE (1 << 16)

void draw(uint8_t *ptr, int width, int height) {
  __builtin_wasm_memory_grow(0, (width * height * 4)/BLOCK_SIZE);

  int i = 0;
  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      ptr[i++] = 188; /* red */
      ptr[i++] = x^(1+y) * 255; /* green */
      ptr[i++] = 188; /* blue */
      ptr[i++] = 255;
    }
}

int add(int a, int b) {
  return a + b;
}
