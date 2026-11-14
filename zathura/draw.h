/* SPDX-License-Identifier: Zlib */

#ifndef ZATHURA_DRAW_H
#define ZATHURA_DRAW_H

#include <stdbool.h>
#include <stddef.h>
#include <gdk/gdk.h>

#include "types.h"

/* Freehand drawing on the page: the strokes become ink annotations that are
 * saved into the document with :write. */

enum {
  ZATHURA_DRAW_OFF = 0,
  ZATHURA_DRAW_PEN,
  ZATHURA_DRAW_ERASER,
};

/** Switch to the pen or the eraser, or back off when that mode is already on */
bool zathura_draw_toggle_mode(zathura_t* zathura, int mode);

/** Leave pen and eraser mode, returns true if one of them was on */
bool zathura_draw_off(zathura_t* zathura);

/** Color (alpha is the opacity) and width in points of the pen */
bool zathura_draw_get_pen(zathura_t* zathura, GdkRGBA* color, double* width);

/** Add a stroke of n_points x, y pairs in page coordinates to a page */
bool zathura_draw_stroke(zathura_t* zathura, unsigned int page_index, const double* points, size_t n_points);

/** Remove the strokes near x, y (page coordinates) */
bool zathura_draw_erase(zathura_t* zathura, unsigned int page_index, double x, double y, double radius);

/** Remove the last stroke drawn in this session */
bool zathura_draw_undo(zathura_t* zathura);

/** Forget the strokes of this session, e.g. when the document is closed */
void zathura_draw_reset(zathura_t* zathura);

#endif
