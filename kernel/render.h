// .h
// OS Graphics Rendering Functions
// by Kyle Furey

#ifndef HLOS_RENDER_H
#define HLOS_RENDER_H

#include "types.h"

/** Whether pixel rendering is enabled (otherwise uses text rendering). */
#ifndef PBA_ENABLED
#define PIXEL_RENDERING 0
#endif

/** Pixel Buffer Array index port. */
#define PBA_INDEX_PORT 0x3C8

/** Pixel Buffer Array color port. */
#define PBA_COLOR_PORT 0x3C9

/** Converts an X and Y value into a Pixel Buffer Array index. */
#define PBA_INDEX(x, y) ((x) + ((y) * PBA_WIDTH))

/** Converts a color to a Pixel Buffer Array color. */
#define COLOR_TO_PBA(r, g, b) ((((r) >> 5) << 5) | (((g) >> 5) << 2) | ((b) >> 6))

/** Converts a Pixel Buffer Array color into a color. */
#define PBA_TO_COLOR(i) ((color_t) { (((i) >> 5) * 255) / 7, (((i) >> 2) * 255) / 7, (((i) & 3) * 255) / 3, 255, })

/** Constructs a color. */
#define COLOR(...) (color_t) { __VA_ARGS__ }

/** Constructs a point. */
#define POINT(...) (point_t) { __VA_ARGS__ }

/** Pixel Buffer Array sizes. */
typedef enum PBA_size {
    PBA_WIDTH = 320,
    PBA_HEIGHT = 200,
    PBA_SIZE = PBA_WIDTH * PBA_HEIGHT,
    GLYPH_HEIGHT = 8,
    GLYPH_WIDTH = 8,
    GLYPH_SIZE = GLYPH_WIDTH * GLYPH_HEIGHT,
} PBA_size_t;

/** Represents a single pixel's color in the Pixel Buffer Array. */
typedef byte_t PBA_color_t;

/** Data for the state of the Pixel Buffer Array. */
typedef struct PBA {
    /** The address of the Pixel Buffer Array. */
    volatile PBA_color_t *const array;

    /** The next frame buffer of the Pixel Buffer Array. */
    PBA_color_t next[PBA_SIZE];
} PBA_t;

/** Represents a 32-bit color. */
typedef struct color {
    /** Red. */
    byte_t r;

    /** Green. */
    byte_t g;

    /** Blue. */
    byte_t b;

    /** Alpha. */
    byte_t a;
} color_t;

/** A 2D point. */
typedef struct point {
    /** X. */
    ushort_t x;

    /** Y. */
    ushort_t y;
} point_t;

/** The Pixel Buffer Array. */
extern PBA_t PBA;

/** Renders the frame buffer to the Pixel Buffer Array. */
void render();

/** Sets the color of the Pixel Buffer Array pixel at <point>. */
void pixel(point_t point, color_t color);

/** Sets the color of an array of pixels in the Pixel Buffer Array. */
void pixels(uint_t size, const point_t *points, color_t color);

/** Sets the color of each pixel in the Pixel Buffer Array. */
void fill(color_t color);

/** Fills a square of pixels from <start> to <end> with a color in the Pixel Buffer Array. */
void square(point_t start, point_t end, color_t color);

/**
 * Prints an 8 x 8 colored glyph in the Pixel Buffer Array starting at <point>.
 * <glyph> must be of size GLYPH_HEIGHT.
 */
void glyph(point_t point, const byte_t *glyph, color_t color);

#endif // HLOS_RENDER_H
