/* SPDX-License-Identifier: Zlib */

#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include <cairo.h>

#include "types.h"
#include "page.h"
#include "document.h"
#include "links.h"
#include "zathura-version.h"

typedef struct zathura_plugin_functions_s zathura_plugin_functions_t;

/**
 * Opens a document
 */
typedef zathura_error_t (*zathura_plugin_document_open_t)(zathura_document_t* document);

/**
 * Frees the document
 */
typedef zathura_error_t (*zathura_plugin_document_free_t)(zathura_document_t* document, void* data);

/**
 * Generates the document index
 */
typedef girara_tree_node_t* (*zathura_plugin_document_index_generate_t)(zathura_document_t* document, void* data,
                                                                        zathura_error_t* error);

/**
 * Save the document
 */
typedef zathura_error_t (*zathura_plugin_document_save_as_t)(zathura_document_t* document, void* data,
                                                             const char* path);

/**
 * Get list of attachments
 */
typedef girara_list_t* (*zathura_plugin_document_attachments_get_t)(zathura_document_t* document, void* data,
                                                                    zathura_error_t* error);

/**
 * Save attachment to a file
 */
typedef zathura_error_t (*zathura_plugin_document_attachment_save_t)(zathura_document_t* document, void* data,
                                                                     const char* attachment, const char* file);

/**
 * Get document information
 */
typedef girara_list_t* (*zathura_plugin_document_get_information_t)(zathura_document_t* document, void* data,
                                                                    zathura_error_t* error);

/**
 * Gets the page object
 */
typedef zathura_error_t (*zathura_plugin_page_init_t)(zathura_page_t* page);

/**
 * Free page
 */
typedef zathura_error_t (*zathura_plugin_page_clear_t)(zathura_page_t* page, void* data);

/**
 * Search text
 */
typedef girara_list_t* (*zathura_plugin_page_search_text_t)(zathura_page_t* page, void* data, const char* text,
                                                            zathura_error_t* error);

/**
 * Get links on a page
 */
typedef girara_list_t* (*zathura_plugin_page_links_get_t)(zathura_page_t* page, void* data, zathura_error_t* error);

/**
 * Get form fields
 */
typedef girara_list_t* (*zathura_plugin_page_form_fields_get_t)(zathura_page_t* page, void* data,
                                                                zathura_error_t* error);

/**
 * Get list of images
 */
typedef girara_list_t* (*zathura_plugin_page_images_get_t)(zathura_page_t* page, void* data, zathura_error_t* error);

/**
 * Get the image
 */
typedef cairo_surface_t* (*zathura_plugin_page_image_get_cairo_t)(zathura_page_t* page, void* data,
                                                                  zathura_image_t* image, zathura_error_t* error);

/**
 * Get text for selection
 */
typedef char* (*zathura_plugin_page_get_text_t)(zathura_page_t* page, void* data, zathura_rectangle_t rectangle,
                                                zathura_error_t* error);

/**
 * Get rectangles from selection
 */
typedef girara_list_t* (*zathura_plugin_page_get_selection_t)(zathura_page_t* page, void* data,
                                                              zathura_rectangle_t rectangle, zathura_error_t* error);

/**
 * Renders the page to a cairo surface.
 */
typedef zathura_error_t (*zathura_plugin_page_render_cairo_t)(zathura_page_t* page, void* data, cairo_t* cairo,
                                                              bool printing);

/**
 * Get page label.
 */
typedef zathura_error_t (*zathura_plugin_page_get_label_t)(zathura_page_t* page, void* data, char** label);

/**
 * Get the text layout of a page: the page text and the bounding box of each
 * of its characters. Used for selecting text with the keyboard.
 */
typedef zathura_error_t (*zathura_plugin_page_get_text_layout_t)(zathura_page_t* page, void* data,
                                                                 zathura_text_layout_t* layout);

/**
 * Get signatures
 */
typedef girara_list_t* (*zathura_plugin_page_get_signatures)(zathura_page_t* page, void* data, zathura_error_t* error);

/**
 * Add a highlight annotation covering the given rectangles (one per line of
 * text, in page coordinates) to the page. The annotation is part of the
 * document and is written by document_save_as.
 */
typedef zathura_error_t (*zathura_plugin_page_add_highlight_t)(zathura_page_t* page, void* data,
                                                               girara_list_t* rectangles, double red, double green,
                                                               double blue);

/**
 * Remove the highlight annotations of the page that intersect any of the given
 * rectangles. The number of removed annotations is stored in removed.
 */
typedef zathura_error_t (*zathura_plugin_page_remove_highlights_t)(zathura_page_t* page, void* data,
                                                                   girara_list_t* rectangles, unsigned int* removed);

/**
 * Add a freehand ink annotation with a single stroke to the page. The points
 * are n_points pairs of x, y in page coordinates; width is the line width in
 * points and opacity is between 0 and 1.
 */
typedef zathura_error_t (*zathura_plugin_page_add_ink_t)(zathura_page_t* page, void* data, const double* points,
                                                         size_t n_points, double width, double red, double green,
                                                         double blue, double opacity);

/**
 * Remove the ink annotations of the page whose strokes pass within radius of
 * the point x, y (page coordinates). If radius is negative, the most recently
 * added ink annotation of the page is removed instead. The number of removed
 * annotations is stored in removed.
 */
typedef zathura_error_t (*zathura_plugin_page_remove_ink_t)(zathura_page_t* page, void* data, double x, double y,
                                                            double radius, unsigned int* removed);

struct zathura_plugin_functions_s {
  /**
   * Opens a document
   */
  zathura_plugin_document_open_t document_open;

  /**
   * Frees the document
   */
  zathura_plugin_document_free_t document_free;

  /**
   * Generates the document index
   */
  zathura_plugin_document_index_generate_t document_index_generate;

  /**
   * Save the document
   */
  zathura_plugin_document_save_as_t document_save_as;

  /**
   * Get list of attachments
   */
  zathura_plugin_document_attachments_get_t document_attachments_get;

  /**
   * Save attachment to a file
   */
  zathura_plugin_document_attachment_save_t document_attachment_save;

  /**
   * Get document information
   */
  zathura_plugin_document_get_information_t document_get_information;

  /**
   * Gets the page object
   */
  zathura_plugin_page_init_t page_init;

  /**
   * Free page
   */
  zathura_plugin_page_clear_t page_clear;

  /**
   * Search text
   */
  zathura_plugin_page_search_text_t page_search_text;

  /**
   * Get links on a page
   */
  zathura_plugin_page_links_get_t page_links_get;

  /**
   * Get form fields
   */
  zathura_plugin_page_form_fields_get_t page_form_fields_get;

  /**
   * Get list of images
   */
  zathura_plugin_page_images_get_t page_images_get;

  /**
   * Get the image
   */
  zathura_plugin_page_image_get_cairo_t page_image_get_cairo;

  /**
   * Get text for selection
   */
  zathura_plugin_page_get_text_t page_get_text;

  /**
   * Get text for selection
   */
  zathura_plugin_page_get_selection_t page_get_selection;

  /**
   * Renders the page to a cairo surface.
   */
  zathura_plugin_page_render_cairo_t page_render_cairo;

  /**
   * Get page label.
   */
  zathura_plugin_page_get_label_t page_get_label;

  /**
   * Get signatures.
   */
  zathura_plugin_page_get_signatures page_get_signatures;

  /**
   * Get the text layout of a page
   */
  zathura_plugin_page_get_text_layout_t page_get_text_layout;

  /**
   * Add a highlight annotation
   */
  zathura_plugin_page_add_highlight_t page_add_highlight;

  /**
   * Remove highlight annotations
   */
  zathura_plugin_page_remove_highlights_t page_remove_highlights;

  /**
   * Add a freehand ink annotation
   */
  zathura_plugin_page_add_ink_t page_add_ink;

  /**
   * Remove ink annotations
   */
  zathura_plugin_page_remove_ink_t page_remove_ink;
};

typedef struct zathura_plugin_definition_s {
  const char* name;
  const char* version;
  zathura_plugin_functions_t functions;
  const size_t mime_types_size;
  const char** mime_types;
} zathura_plugin_definition_t;

#define JOIN(x, y) JOIN2(x, y)
#define JOIN2(x, y) x##_##y

#define ZATHURA_PLUGIN_DEFINITION_SYMBOL JOIN(zathura_plugin, JOIN(ZATHURA_API_VERSION, ZATHURA_ABI_VERSION))

/**
 * Register a plugin.
 *
 * @param plugin_name the name of the plugin
 * @param version the plugin's version
 * @param plugin_functions function to register the plugin's document functions
 * @param mimetypes a char array of mime types supported by the plugin
 */
#define ZATHURA_PLUGIN_REGISTER_WITH_FUNCTIONS(plugin_name, plugin_version, plugin_functions, mimetypes)               \
  static const char* zathura_plugin_mime_types[] = mimetypes;                                                          \
                                                                                                                       \
  ZATHURA_PLUGIN_API const zathura_plugin_definition_t ZATHURA_PLUGIN_DEFINITION_SYMBOL = {                            \
      .name            = plugin_name,                                                                                  \
      .version         = plugin_version,                                                                               \
      .functions       = plugin_functions,                                                                             \
      .mime_types_size = sizeof(zathura_plugin_mime_types) / sizeof(zathura_plugin_mime_types[0]),                     \
      .mime_types      = zathura_plugin_mime_types,                                                                    \
  };

#define ZATHURA_PLUGIN_MIMETYPES(...) __VA_ARGS__
#define ZATHURA_PLUGIN_FUNCTIONS(...) __VA_ARGS__

#endif // PLUGIN_API_H
