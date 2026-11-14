/* SPDX-License-Identifier: Zlib */

#include "draw.h"

#include <glib/gi18n.h>
#include <girara-gtk/session.h>
#include <girara-gtk/settings.h>

#include "document.h"
#include "internal.h"
#include "page.h"
#include "page-widget.h"
#include "plugin.h"
#include "utils.h"
#include "zathura.h"

static void refresh_page(zathura_t* zathura, zathura_page_t* page) {
  ZathuraPageWidget* widget = zathura_page_get_widget(zathura, page);
  if (widget != NULL) {
    zathura_page_widget_refresh(widget);
  }
}

static void set_cursor(zathura_t* zathura) {
  if (zathura->ui.document_widget == NULL) {
    return;
  }
  const char* name = NULL;
  switch (zathura->global.draw_mode) {
  case ZATHURA_DRAW_PEN:
    name = "crosshair";
    break;
  case ZATHURA_DRAW_ERASER:
    name = "cell";
    break;
  }
  gtk_widget_set_cursor_from_name(GTK_WIDGET(zathura->ui.document_widget), name);
}

bool zathura_draw_off(zathura_t* zathura) {
  g_return_val_if_fail(zathura != NULL, false);
  if (zathura->global.draw_mode == ZATHURA_DRAW_OFF) {
    return false;
  }
  zathura->global.draw_mode = ZATHURA_DRAW_OFF;
  set_cursor(zathura);
  girara_notify(zathura->ui.session, GIRARA_INFO, _("Drawing off."));
  return true;
}

bool zathura_draw_toggle_mode(zathura_t* zathura, int mode) {
  g_return_val_if_fail(zathura != NULL, false);

  if (zathura->global.draw_mode == mode) {
    return zathura_draw_off(zathura);
  }

  if (zathura_has_document(zathura) == false) {
    girara_notify(zathura->ui.session, GIRARA_ERROR, _("No document opened."));
    return false;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(zathura_get_document(zathura));
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions == NULL || functions->page_add_ink == NULL) {
    girara_notify(zathura->ui.session, GIRARA_ERROR, _("This document type does not support drawing."));
    return false;
  }

  zathura->global.draw_mode = mode;
  set_cursor(zathura);
  if (mode == ZATHURA_DRAW_PEN) {
    girara_notify(zathura->ui.session, GIRARA_INFO, _("Pen: drag to draw, U undoes, Esc stops."));
  } else {
    girara_notify(zathura->ui.session, GIRARA_INFO, _("Eraser: click or drag over a stroke to remove it."));
  }
  return true;
}

bool zathura_draw_get_pen(zathura_t* zathura, GdkRGBA* color, double* width) {
  g_return_val_if_fail(zathura != NULL && color != NULL && width != NULL, false);

  *color = (GdkRGBA){.red = 0.9, .green = 0.1, .blue = 0.1, .alpha = 1};
  *width = 2;

  g_autofree char* name = NULL;
  girara_setting_get(zathura->ui.session, "pen-color", &name);
  if (name != NULL) {
    /* like :highlight, a hex color works without the # that would start a comment */
    g_autofree char* hex = name[0] != '#' ? g_strdup_printf("#%s", name) : NULL;
    if (gdk_rgba_parse(color, name) == FALSE && (hex == NULL || gdk_rgba_parse(color, hex) == FALSE)) {
      *color = (GdkRGBA){.red = 0.9, .green = 0.1, .blue = 0.1, .alpha = 1};
      return false;
    }
  }

  float w = 2;
  if (girara_setting_get(zathura->ui.session, "pen-width", &w) == true && w > 0) {
    *width = w;
  }
  return true;
}

bool zathura_draw_stroke(zathura_t* zathura, unsigned int page_index, const double* points, size_t n_points) {
  g_return_val_if_fail(zathura != NULL, false);
  if (zathura_has_document(zathura) == false || points == NULL || n_points == 0) {
    return false;
  }

  zathura_page_t* page = zathura_document_get_page(zathura_get_document(zathura), page_index);
  if (page == NULL) {
    return false;
  }

  GdkRGBA color;
  double width;
  if (zathura_draw_get_pen(zathura, &color, &width) == false) {
    g_autofree char* name = NULL;
    girara_setting_get(zathura->ui.session, "pen-color", &name);
    girara_notify(zathura->ui.session, GIRARA_ERROR, _("Invalid pen color: %s"), name != NULL ? name : "");
    return false;
  }

  /* a click without moving is a dot */
  double dot[4];
  if (n_points == 1) {
    dot[0] = dot[2] = points[0];
    dot[1] = dot[3] = points[1];
    points          = dot;
    n_points        = 2;
  }

  const zathura_error_t error =
      zathura_page_add_ink(page, points, n_points, width, color.red, color.green, color.blue, color.alpha);
  if (error == ZATHURA_ERROR_NOT_IMPLEMENTED) {
    girara_notify(zathura->ui.session, GIRARA_ERROR, _("This document type does not support drawing."));
    return false;
  } else if (error != ZATHURA_ERROR_OK) {
    girara_notify(zathura->ui.session, GIRARA_ERROR, _("Failed to add the stroke."));
    return false;
  }

  if (zathura->global.ink_undo == NULL) {
    zathura->global.ink_undo = g_array_new(FALSE, FALSE, sizeof(unsigned int));
  }
  g_array_append_val(zathura->global.ink_undo, page_index);
  zathura->global.unsaved_highlights = true;
  refresh_page(zathura, page);
  return true;
}

bool zathura_draw_erase(zathura_t* zathura, unsigned int page_index, double x, double y, double radius) {
  g_return_val_if_fail(zathura != NULL, false);
  if (zathura_has_document(zathura) == false) {
    return false;
  }

  zathura_page_t* page = zathura_document_get_page(zathura_get_document(zathura), page_index);
  if (page == NULL) {
    return false;
  }

  unsigned int removed        = 0;
  const zathura_error_t error = zathura_page_remove_ink(page, x, y, radius, &removed);
  if (error != ZATHURA_ERROR_OK || removed == 0) {
    return false;
  }

  /* undo removes the newest stroke of a page, which may now be another one: forget that page */
  GArray* undo = zathura->global.ink_undo;
  for (guint idx = undo != NULL ? undo->len : 0; idx > 0; --idx) {
    if (g_array_index(undo, unsigned int, idx - 1) == page_index) {
      g_array_remove_index(undo, idx - 1);
    }
  }

  zathura->global.unsaved_highlights = true;
  refresh_page(zathura, page);
  return true;
}

bool zathura_draw_undo(zathura_t* zathura) {
  g_return_val_if_fail(zathura != NULL, false);

  GArray* undo = zathura->global.ink_undo;
  if (zathura_has_document(zathura) == false || undo == NULL || undo->len == 0) {
    girara_notify(zathura->ui.session, GIRARA_INFO, _("Nothing to undo."));
    return false;
  }

  const unsigned int page_index = g_array_index(undo, unsigned int, undo->len - 1);
  g_array_remove_index(undo, undo->len - 1);

  zathura_page_t* page = zathura_document_get_page(zathura_get_document(zathura), page_index);
  unsigned int removed = 0;
  if (page == NULL || zathura_page_remove_ink(page, 0, 0, -1, &removed) != ZATHURA_ERROR_OK || removed == 0) {
    girara_notify(zathura->ui.session, GIRARA_ERROR, _("Failed to undo the stroke."));
    return false;
  }

  zathura->global.unsaved_highlights = true;
  refresh_page(zathura, page);
  return true;
}

void zathura_draw_reset(zathura_t* zathura) {
  g_return_if_fail(zathura != NULL);
  if (zathura->global.ink_undo != NULL) {
    g_array_free(zathura->global.ink_undo, TRUE);
    zathura->global.ink_undo = NULL;
  }
  if (zathura->global.draw_mode != ZATHURA_DRAW_OFF) {
    zathura->global.draw_mode = ZATHURA_DRAW_OFF;
    set_cursor(zathura);
  }
}
