/* SPDX-License-Identifier: Zlib */

/* Drive a real zathura session headlessly (e.g. with GDK_BACKEND=broadway):
 *
 *   drive_highlight <plugindir> <file.pdf> <step>...
 *
 * A step starting with ':' runs that command, any other step is fed as keys,
 * like the feedkeys shortcut does. "@x,y,x,y,..." drags the mouse over page 1
 * through those points (page coordinates, like the pen does with the left
 * button). After each step the state is printed.
 */

#include <stdio.h>
#include <girara/log.h>
#include <girara-gtk/session.h>
#include <girara-gtk/settings.h>

#include "zathura.h"
#include "document.h"
#include "page.h"
#include "shortcuts.h"
#include "page-widget.h"
#include "utils.h"

bool girara_sc_feedkeys(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);
bool girara_command_run(girara_session_t* session, const char* input);

static void pump(void) {
  for (int idle = 0; idle < 20;) {
    if (g_main_context_iteration(NULL, FALSE) == FALSE) {
      ++idle;
      g_usleep(5000);
    } else {
      idle = 0;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s plugindir file.pdf step...\n", argv[0]);
    return 2;
  }
  girara_set_log_level(GIRARA_WARNING);
  gtk_init();

  g_autofree char* tmp = g_dir_make_tmp("zathura-drive-XXXXXX", NULL);
  zathura_t* zathura   = zathura_create();
  zathura_set_config_dir(zathura, tmp);
  zathura_set_data_dir(zathura, tmp);
  zathura_set_cache_dir(zathura, tmp);
  zathura_set_plugin_dir(zathura, argv[1]);
  if (zathura_init(zathura) == false) {
    fprintf(stderr, "init failed\n");
    return 1;
  }
  pump();

  if (document_open(zathura, argv[2], NULL, NULL, 0, NULL) == false) {
    fprintf(stderr, "open failed\n");
    return 1;
  }
  pump();

  for (int i = 3; i < argc; ++i) {
    bool ret = true;
    if (argv[i][0] == '@') {
      /* the pen/eraser input the mouse handlers feed, in widget coordinates */
      ZathuraPageWidget* widget = zathura_page_get_widget_by_number(zathura, 0);
      const double scale        = zathura_document_get_scale(zathura_get_document(zathura));
      g_auto(GStrv) parts       = g_strsplit(argv[i] + 1, ",", -1);
      for (guint p = 0; parts[p] != NULL && parts[p + 1] != NULL; p += 2) {
        const double x = g_ascii_strtod(parts[p], NULL) * scale;
        const double y = g_ascii_strtod(parts[p + 1], NULL) * scale;
        if (p == 0) {
          zathura_page_widget_ink_press(widget, x, y);
        } else {
          zathura_page_widget_ink_motion(widget, x, y);
        }
      }
      zathura_page_widget_ink_release(widget);
    } else if (argv[i][0] == ':') {
      ret = girara_command_run(zathura->ui.session, argv[i] + 1);
    } else {
      girara_argument_t arg = {.n = 0, .data = argv[i]};
      girara_sc_feedkeys(zathura->ui.session, &arg, NULL, 0);
    }
    pump();
    printf("step %-14.14s ret=%d draw=%d mode=%s visual=%d unsaved=%d open=%d\n", argv[i], ret,
           zathura->global.draw_mode,
           girara_mode_get(zathura->ui.session) == zathura->modes.visual ? "visual" : "other",
           zathura->visual != NULL, zathura->global.unsaved_highlights, zathura_has_document(zathura));
  }

  /* what zathura itself draws for page 1 now, before anything is saved */
  const char* png = g_getenv("DRIVE_PNG");
  if (png != NULL && zathura_has_document(zathura) == true) {
    zathura_page_t* page     = zathura_document_get_page(zathura_get_document(zathura), 0);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, zathura_page_get_width(page),
                                                          zathura_page_get_height(page));
    cairo_t* cairo           = cairo_create(surface);
    printf("render %d\n", zathura_page_render(page, cairo, false));
    cairo_surface_write_to_png(surface, png);
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);
  }

  zathura_free(zathura);
  return 0;
}
