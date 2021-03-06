#include <stdlib.h>
#include <stdio.h>

//[KCTF][TYLE_H]===============================================================

typedef struct Tyle_t {
    unsigned char creature;
    unsigned char ground;
} Tyle;

void Tyle_construct(Tyle *cake) {
    cake->creature = 0;
    cake->ground = '.';
}

//=============================================================================
//[KCTF][MAP_H]================================================================

typedef struct Map_t {
    size_t w;
    size_t h;
    Tyle *buffer;
} Map;

Tyle *Map_get(const Map *cake, const size_t x, const size_t y) {
    return &(cake->buffer[x * cake->w + y]);
}

void Map_construct(Map *cake, const size_t w, const size_t h) {
    cake->w = w;
    cake->h = h;
    cake->buffer = calloc(w * h, sizeof(Tyle));
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Tyle_construct(&cake->buffer[y * w + x]);
        }
    }
}

void Map_print(const Map *cake) {
    for (int y = 0; y < cake->h; ++y) {
        for (int x = 0; x < cake->w; ++x) {
            printf("%c", cake->buffer[y * cake->w + x].ground);
        }
        printf("\n");
    }
}

//=============================================================================
//[KCTF][WORLD_H]==============================================================

typedef struct Wortld_t {
    Map map;
} World;

void World_construct(World *cake, const size_t w, const size_t h) {
    Map_construct(&cake->map, w, h);
}

void World_print(World *cake) {
    Map_print(&cake->map);
}

//=============================================================================

int f(int x) {
    return (x + 5) ? x + 5 : (printf("*"), x + 5);
}

int main() {
    printf("%d\n", f(10));

    return 0;
}

