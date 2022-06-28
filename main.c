#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE (1 << 16)
extern void print(int i);
extern void putchar(char c);
extern unsigned char __heap_base;

char temp_str[1 << 11];

typedef struct { uint8_t r, g, b, a; } Color;
static struct {
  int width, height;
  Color *pixels;
} rendr;
#define RENDR_PIXELS_LEN (rendr.width * rendr.height * 4)

typedef struct Sprite Sprite;
struct Sprite {
  char type;
  uint8_t x, y, dx, dy;
  Sprite *next;
};
#define MAP_SIZE_X (100)
#define MAP_SIZE_Y (100)
typedef Sprite *SpriteStack;
static SpriteStack map[MAP_SIZE_X][MAP_SIZE_Y];

static Sprite *sprite_pool;

static Color char_color[255] = {
  ['0'] = {  0,   0,   0, 255},
  ['1'] = {128, 128, 128, 255},
  ['2'] = {255, 255, 255, 255},
  ['3'] = {255,   0,   0, 255},
  ['4'] = {  0, 255,   0, 255},
  ['5'] = {  0,   0, 255, 255},
  ['6'] = {255, 255,   0, 255},
  ['7'] = {  0, 255, 255, 255},
  ['8'] = {255,   0, 255, 255},
  ['.'] = {  0,   0,   0,   0}
};

static uint8_t char_solid[255] = {0};
void solid_set(char c) { char_solid[c] = 1; }

typedef struct { Color p[16][16]; } Bitmap;
static Bitmap char_bitmap[255];
void bitmap_set_from_str(char dest, char *str) {
  Bitmap ret = {0};

  if (str[1] == '\0') {
    for (int x = 0; x < 16; x++)
      for (int y = 0; y < 16; y++)
        ret.p[x][y] = char_color[*str];
    char_bitmap[dest] = ret;
    return;
  }

  int tx = 0, ty = 0;
  do {
    switch (*str) {
      case '\n': ty++, tx = 0; break;
      case  '.': tx++;         break;
      case '\0':               break;
      case  ' ':               break;
      default: {
        ret.p[tx][ty] = char_color[*str];
        tx++;
      } break;
    }
  } while (*str++);

  char_bitmap[dest] = ret;
  return;
}

typedef enum {
  PatternKind_Deep,
  PatternKind_Flat,
} PatternKind;
typedef struct { PatternKind kind; char str[16]; } Pattern;
static Pattern char_pattern[255] = {};
void pattern_set(char dest, PatternKind kind, char *str) {
  Pattern p = { kind };
  for (int i = 0; str[i] && i < 16; i++) p.str[i] = str[i];
  char_pattern[dest] = p;
}

typedef struct { char key, val; } CharPushAssoc;
static CharPushAssoc char_push_assoc[1 << 7];
static int char_push_assoc_count = 0;

void sprite_set_push(char key, char val) {
  char_push_assoc[char_push_assoc_count++] =
    (CharPushAssoc) { .key = key, .val = val };
}

uint8_t sprite_can_push(char key, char val) {
  for (
    CharPushAssoc *cpa = char_push_assoc;
    (cpa - char_push_assoc) < char_push_assoc_count;
    cpa++
  ) {
    if (cpa->key == key && cpa->val == val)
      return 1;
  }
  return 0;
}

/* returns true if this pattern matches this SpriteStack
   in the case of flat patterns, the given SpriteStack is the top left */
uint8_t spritestack_match(SpriteStack ss, char p) {
  char tmp[2] = "\0\0";

  Pattern *pat = char_pattern + (int) p;
  switch (pat->kind) {

    /* each char in the pattern needs to be found _somewhere_ in the stack,
       order doesn't matter. */
    case PatternKind_Deep: {
      char *str = pat->str;
      if (!*str) tmp[0] = p, str = tmp;

      for (; *str; str++ ) {
        SpriteStack s = ss;
        for (; s; s = s->next) {
          if (s->type == *str)
            goto NEXT;
        }
        return 0;
        NEXT:;
      }
      return 1;
    } break;

    /* all characters down and to the right need to match */
    case PatternKind_Flat: {
      // char *str = pat->str;
      // int tx = 0, ty = 0;
      // do {
      //   switch (*str) {
      //     case '\n': ty++, tx = 0; break;
      //     case  '.': tx++;         break;
      //     case '\0':               break;
      //     default: {
      //       tx++;
      //     } break;
      //   }
      // } while (*str++);
    } break;

  }
  return 0;
}

Sprite *map_index(uint8_t x, uint8_t y) { return map[x][y]; }

typedef struct { uint8_t x, y, open; } MatchState;
uint8_t map_match_x(MatchState *ms) { return ms->x; }
uint8_t map_match_y(MatchState *ms) { return ms->y; }
uint8_t map_match(MatchState *ms, char p) {
  if (ms->open) ms->x++;
  ms->open = 1;

  Sprite *s;
  for (; ms->y < MAP_SIZE_Y; ms->y++) {
    for (; ms->x < MAP_SIZE_X; ms->x++)
      if ((s = map[ms->x][ms->y]))
        if (spritestack_match(s, p))
          return 1;
    ms->x = 0;
  }
  return 0;
}


/* does this swap entire tiles or just the relevant sprites? */
/*
static int map_swap(char a, char b) {
  int i = 0;
  MatchState ms = {0};
  while (map_match(&ms, a)) {
    i++;
    Sprite s = (Sprite) { .x = ms.x, .y = ms.y, .type = b };
    if (map[ms.x][ms.y]) *map[ms.x][ms.y] = s;
    else *(map[ms.x][ms.y] = sprite_pool++) = s;
  }
  return i;
}*/

static Sprite *sprite_add(int x, int y, char type) {
  Sprite s = (Sprite) { .x = x, .y = y, .type = type };
  s.next = map[x][y];
  *(map[x][y] = sprite_pool++) = s;
  return map[x][y];
}

/* removes sprite from its tile */
static void sprite_pluck(Sprite *s) {
  Sprite *n = map[s->x][s->y];

  if (n == s){
    map[s->x][s->y] = s->next;
    return;
  }
  for (; n->next; n = n->next)
    if (n->next == s) {
      n->next = s->next;
      return;
    }
}

uint8_t sprite_move(Sprite *s, int dx, int dy) {
  if (char_solid[(int)s->type]) {
    /* no moving into a solid! */
    Sprite *n = map[s->x+dx][s->y+dy];
    for (; n; n = n->next)
      if (char_solid[(int)n->type]) {
        /* unless you can push them out of the way ig */
        if (sprite_can_push(s->type, n->type)) {
          if (!sprite_move(n, dx, dy))
            return 0;
        }
        else
          return 0;
      }

    /* ok, what'd we push? */
    n = map[s->x+dx][s->y+dy];
    for (; n; n = n->next)
      if (sprite_can_push(s->type, n->type))
        sprite_move(n, dx, dy);
  }

  sprite_pluck(s);
  s->x += dx;
  s->y += dy;
  s->dx = dx;
  s->dy = dy;
  s->next = map[s->x][s->y];
  map[s->x][s->y] = s;
  return 1;
}

void init(int width, int height) {
  rendr.width = width;
  rendr.height = height;

  /* deal with the two dynamically allocated things
    (pixels, sprites) */
  {
    void *mem = &__heap_base;
    size_t needed = RENDR_PIXELS_LEN + sizeof(Sprite) * (1 << 11);
    __builtin_wasm_memory_grow(0, 1+needed/BLOCK_SIZE);

    /* dump memory into renderer */
    rendr.pixels = mem;
    mem += RENDR_PIXELS_LEN;

    /* rest for sprite pool */
    sprite_pool = mem;
  }
}

void map_set(char *str) {
  __builtin_memset(map, 0, sizeof(map));

  int tx = 0, ty = 0;
  do {
    switch (*str) {
      case '\n': ty++, tx = 0; break;
      case  '.': tx++;         break;
      case '\0':               break;
      case  ' ':               break;
      default: {
        map[tx][ty] = sprite_pool++;
        *map[tx][ty] = (Sprite) { .x = tx, .y = ty, .type = *str };
        tx++;
      } break;
    }
  } while (*str++);
}

static void render_tile(uint8_t tx, uint8_t ty) {
  char type = map[tx][ty]->type;
  for (int x = 0; x < 16; x++)
    for (int y = 0; y < 16; y++) {
      Color color = char_bitmap[(int)type].p[x][y];
      if (color.a)
        rendr.pixels[(y + ty*16)*rendr.height + (x + tx*16)] = color; 
    }
}

void draw(void) {
  for (int x = 0; x < rendr.width; x++)
    for (int y = 0; y < rendr.height; y++)
      rendr.pixels[x*rendr.width + y] = (Color) { 30, 30, 85, 255 }; 

  for (int x = 0; x < MAP_SIZE_X; x++)
    for (int y = 0; y < MAP_SIZE_Y; y++)
      if (map[x][y] && map[x][y]->type)
        render_tile(x, y);
}
