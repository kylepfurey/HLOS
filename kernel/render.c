// .c
// OS Graphics Rendering Functions
// by Kyle Furey

#include "hlos.h"

/** The Pixel Buffer Array. */
PBA_t PBA = {
    (volatile PBA_color_t * const) &__PBA_start,
    {0},
};

/** Renders the frame buffer to the Pixel Buffer Array. */
void render() {
    copy((void *) PBA.array, PBA.next, PBA_SIZE);
}

/** Sets the color of the Pixel Buffer Array pixel at <point>. */
void pixel(point_t point, color_t color) {
    if (point.x >= PBA_WIDTH || point.y >= PBA_HEIGHT) {
        return;
    }
    PBA.next[PBA_INDEX(point.x, point.y)] = COLOR_TO_PBA(color.r, color.g, color.b);
}

/** Sets the color of an array of pixels in the Pixel Buffer Array. */
void pixels(uint_t size, const point_t *points, color_t color) {
    PBA_color_t c = COLOR_TO_PBA(color.r, color.g, color.b);
    for (uint_t i = 0; i < size; ++i) {
        ushort_t x = points[i].x;
        ushort_t y = points[i].y;
        if (x >= PBA_WIDTH || y >= PBA_HEIGHT) {
            continue;
        }
        PBA.next[PBA_INDEX(x, y)] = c;
    }
}

/** Sets the color of each pixel in the Pixel Buffer Array. */
void fill(color_t color) {
    PBA_color_t c = COLOR_TO_PBA(color.r, color.g, color.b);
    set(PBA.next, c, PBA_SIZE);
}

/** Fills a square of pixels from <start> to <end> with a color in the Pixel Buffer Array. */
void square(point_t start, point_t end, color_t color) {
    PBA_color_t c = COLOR_TO_PBA(color.r, color.g, color.b);
    if (start.x > end.x) {
        swap(&start.x, &end.x, sizeof(start.x));
    }
    if (start.y > end.y) {
        swap(&start.y, &end.y, sizeof(start.y));
    }
    if (start.x >= PBA_WIDTH || start.y >= PBA_HEIGHT) {
        return;
    }
    end.x = (ushort_t) min(end.x, PBA_WIDTH - 1);
    end.y = (ushort_t) min(end.y, PBA_HEIGHT - 1);
    uint_t x = end.x - start.x + 1;
    for (uint_t y = start.y; y <= end.y; ++y) {
        set(PBA.next + (start.x + (y * PBA_WIDTH)), c, x);
    }
}

/**
 * Prints an 8 x 8 colored glyph in the Pixel Buffer Array starting at <point>.
 * <glyph> must be of size GLYPH_HEIGHT.
 */
void glyph(point_t point, const byte_t *glyph, color_t color) {
    if (point.x >= PBA_WIDTH || point.y >= PBA_HEIGHT) {
        return;
    }
    PBA_color_t c = COLOR_TO_PBA(color.r, color.g, color.b);
    for (uint_t byte = 0; byte < GLYPH_HEIGHT; ++byte) {
        if (point.y + byte >= PBA_HEIGHT) {
            break;
        }
        for (uint_t bit = 0; bit < GLYPH_WIDTH; ++bit) {
            if (point.x + bit >= PBA_WIDTH) {
                break;
            }
            if ((glyph[byte] & (1 << (7 - bit))) != 0) {
                PBA.next[PBA_INDEX(point.x + bit, point.y + byte)] = c;
            }
        }
    }
}
