/*
g3data : A program for grabbing data from scanned graphs
Copyright (C) 2011 Paul Novak

    This file is part of g3data.

    g3data is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    g3data is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Authors email : pnovak@alumni.caltech.edu
*/

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <math.h>
#include "g3data-window.h"
#include "g3data-image.h"
#include "drawing.h"
#include "points.h"
#include "main.h"

#define ZOOMPIXSIZE 200
#define ZOOMFACTOR 4
#define MAXPOINTS 256

extern gboolean use_error;

static GtkWidget *g3data_window_control_points_add (G3dataWindow *window, struct g3data_options *options);
static GtkWidget *g3data_window_status_area_add (G3dataWindow *window);
static GtkWidget *g3data_window_remove_buttons_add (G3dataWindow *window);
static GtkWidget *g3data_window_zoom_area_add (G3dataWindow *window);
static GtkWidget *g3data_window_log_buttons_add (G3dataWindow *window, struct g3data_options *options);
static GtkWidget *g3data_window_sort_buttons_add (G3dataWindow *window);
static GtkWidget *g3data_window_error_buttons_add (void);
static gint g3data_image_insert (G3dataWindow *window,
                                 const gchar *filename,
                                 GtkWidget *container,
                                 struct g3data_options *options);
static void SetButtonSensitivity (G3dataWindow *window);

/* Callbacks */
static gboolean image_area_draw (GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean zoom_area_draw (GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data);
static void control_point_button_toggled (GtkWidget *widget, gpointer data);
static void control_point_entry_read (GtkWidget *entry, gpointer func_data);
static void button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data);
static void remove_last (GtkWidget *widget, gpointer data);
static void remove_all (GtkWidget *widget, gpointer data);
static void log_button_callback (GtkWidget *widget, gpointer data);
static void key_press_event(GtkWidget *widget, GdkEventKey *event);

static const gchar control_point_header_text[] = "<b>Axis points</b>";
static const gchar status_area_header[] = "<b>Processing information</b>";
static const gchar zoom_area_header[] = "<b>Zoom area</b>";
static const gchar log_header[] = "<b>Logarithmic scales</b>";
static const gchar sort_header[] = "<b>Point ordering</b>";
static const gchar error_header[] = "<b>Value errors</b>";

static const gchar control_point_button_text[4][40] = {
        "Set point X<sub>1</sub> on X axis (_1)",
        "Set point X<sub>2</sub> on X axis (_2)",
        "Set point Y<sub>1</sub> on Y axis (_3)",
        "Set point Y<sub>2</sub> on Y axis (_4)"};

static const gchar control_point_tooltip[4][28] = {
        "Set first point on x axis",
        "Set second point on x axis",
        "Set first point on y axis",
        "Set second point on y axis"};

static const gchar control_point_label_text[4][32] = {
        "X<sub>1</sub> value : ",
        "X<sub>2</sub> value : ",
        "Y<sub>1</sub> value : ",
        "Y<sub>2</sub> value : "};

static const gchar control_point_entry_tooltip[4][37] = {
        "Value of the first point on x axis",
        "Value of the second point on x axis",
        "Value of the first point on y axis",
        "Value of the second point on y axis"};

static const gchar x_string[] = " X : ";
static const gchar y_string[] = " Y : ";
static const gchar pm_string[] = " ± ";
static const gchar nump_string[] = "Number of points : ";

static const gchar remove_last_button_text[] = "_Remove last point";
static const gchar remove_all_button_text[] = "Remove _all points";
static const gchar remove_last_tooltip[] = "Remove last point";
static const gchar remove_all_tooltip[] = "Remove all points";

static const gchar x_log_text[] = "_X axis is logarithmic";
static const gchar y_log_text[] = "_Y axis is logarithmic";
static const gchar x_log_tooltip[] = "X axis is logarithmic";
static const gchar y_log_tooltip[] = "Y axis is logarithmic";

static const gchar sort_button_text[3][20] = {
        "No ordering",
        "Based on X value",
        "Based on Y value"};

static const gchar error_button_text[] = "Include _errors";
static const gchar error_tooltip[] = "Export errors of the x and y values";

static gchar g3data_window_title[] = "%s - g3data";


void g3data_window_insert_image (G3dataWindow *window,
                                 const gchar *filename,
                                 struct g3data_options *options)
{
    GtkWidget *tophbox, *bottomhbox, *bottomleftvbox, *bottomrightvbox,
              *scrolled_window, *viewport;
    GtkWidget *control_point_vbox, *status_area_vbox, *remove_buttons_vbox,
              *error_button_vbox;
    GdkCursor *cursor;
    gint i;
    gchar *buffer;

    tophbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (window->main_vbox), tophbox, FALSE, FALSE, 0);

    bottomhbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (window->main_vbox), bottomhbox, TRUE, TRUE, 0);

    bottomleftvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (bottomhbox), bottomleftvbox, FALSE, FALSE, 0);

    bottomrightvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (bottomhbox), bottomrightvbox, TRUE, TRUE, 0);

    control_point_vbox = g3data_window_control_points_add (window, options);
    gtk_box_pack_start (GTK_BOX (tophbox), control_point_vbox, FALSE, FALSE, 0);

    status_area_vbox = g3data_window_status_area_add (window);
    gtk_box_pack_start (GTK_BOX (tophbox), status_area_vbox, FALSE, FALSE, 0);

    remove_buttons_vbox = g3data_window_remove_buttons_add (window);
    gtk_box_pack_start (GTK_BOX (bottomleftvbox), remove_buttons_vbox, FALSE, FALSE, 0);

    window->zoom_area_vbox = g3data_window_zoom_area_add (window);
    gtk_box_pack_start (GTK_BOX (bottomleftvbox), window->zoom_area_vbox, FALSE, FALSE, 0);

    window->log_buttons_vbox = g3data_window_log_buttons_add (window, options);
    gtk_box_pack_start (GTK_BOX (bottomleftvbox), window->log_buttons_vbox, FALSE, FALSE, 0);

    window->sort_buttons_vbox = g3data_window_sort_buttons_add (window);
    gtk_box_pack_start (GTK_BOX (bottomleftvbox), window->sort_buttons_vbox, FALSE, FALSE, 0);

    error_button_vbox = g3data_window_error_buttons_add ();
    gtk_box_pack_start (GTK_BOX (bottomleftvbox), error_button_vbox, FALSE, FALSE, 0);

    /* Create a scrolled window to hold image */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    viewport = gtk_viewport_new (NULL, NULL);
    gtk_box_pack_start (GTK_BOX (bottomrightvbox), scrolled_window, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (scrolled_window), viewport);

    /* No cursor position yet: the zoom area shows only its marker
     * until the pointer moves over the image */
    window->x = window->y = -1;

    if (g3data_image_insert (window, filename, viewport, options) == 0) {
        cursor = gdk_cursor_new_for_display (gtk_widget_get_display (viewport),
                                             GDK_CROSSHAIR);
        gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (viewport)), cursor);
        g_object_unref (cursor);
    }

	g_signal_connect (G_OBJECT (scrolled_window),
                      "key-press-event",
                      G_CALLBACK (key_press_event),
                      NULL);

    /* Print current image name in title bar*/
    buffer = g_strdup_printf (g3data_window_title, g_path_get_basename (filename));
    gtk_window_set_title (GTK_WINDOW (window), buffer);
    g_free (buffer);

    g3data_window_set_image_actions_enabled (window, TRUE);

    /* Allocate memory to store the point coordinates */
    window->points = (gint **) g_malloc (sizeof (gint *) * MAXPOINTS);
    for (i = 0; i < MAXPOINTS; i++) {
        window->points[i] = (gint *) g_malloc (sizeof (gint) * 2);
    }
    window->numpoints = 0;
    window->size = MAXPOINTS;

    gtk_widget_show_all (window->main_vbox);
}


/* Add control points area. */
static GtkWidget *g3data_window_control_points_add (G3dataWindow *window, struct g3data_options *options) {
    GtkWidget *control_point_header, *control_point_label;
    GtkWidget *vbox, *hbox, *table, *label;
    int i;
    gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    /* Create header for control points section */
    control_point_header = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (control_point_header), control_point_header_text);
    gtk_widget_set_halign (control_point_header, GTK_ALIGN_START);
    gtk_widget_set_valign (control_point_header, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (vbox), control_point_header, FALSE, FALSE, 0);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    table = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (table), 0);
    gtk_grid_set_column_spacing (GTK_GRID (table), 0);
    gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 0);

    for (i = 0; i < 4; i++) {
        /* buttons for setting control points x_1, x_2, etc. */
        label = gtk_label_new (NULL);
        gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), control_point_button_text[i]);
        window->control_point_button[i] = gtk_toggle_button_new();
        gtk_container_add (GTK_CONTAINER (window->control_point_button[i]), label);
        g_signal_connect (G_OBJECT (window->control_point_button[i]),
                          "toggled",
                          G_CALLBACK (control_point_button_toggled),
                          (gpointer) window);
        gtk_widget_set_tooltip_text (window->control_point_button[i], control_point_tooltip[i]);

        /* labels for control points x_1, x_2, etc. */
        control_point_label = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL (control_point_label), control_point_label_text[i]);

        /* text entries to enter control points x_1, x_2, etc. */
        window->control_point_entry[i] = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY (window->control_point_entry[i]), 20);
        gtk_widget_set_sensitive (window->control_point_entry[i], FALSE);
        if (options->control_point_coords[i] != G_MAXDOUBLE) {
            g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", options->control_point_coords[i]);
            gtk_entry_set_text (GTK_ENTRY (window->control_point_entry[i]), buf);
            gtk_widget_set_sensitive (window->control_point_entry[i], TRUE);
        }
    	g_signal_connect (G_OBJECT (window->control_point_entry[i]),
                          "changed",
                          G_CALLBACK (control_point_entry_read),
                          (gpointer) window);
        gtk_widget_set_tooltip_text (window->control_point_entry[i], control_point_entry_tooltip[i]);

        /* Packing the control points labels and entries */
	    gtk_grid_attach (GTK_GRID (table), window->control_point_button[i], 0, i, 1, 1);
	    gtk_grid_attach (GTK_GRID (table), control_point_label, 1, i, 1, 1);
	    gtk_grid_attach (GTK_GRID (table), window->control_point_entry[i], 2, i, 1, 1);
    }
    return vbox;
}


/* Add status area. */
static GtkWidget *g3data_window_status_area_add (G3dataWindow *window) {
    GtkWidget *x_label, *y_label;
    GtkWidget *pm_label, *pm_label2;
    GtkWidget *nump_label;
    GtkWidget *status_area_label;
    GtkWidget *vbox, *table;

    x_label = gtk_label_new (x_string);
    y_label = gtk_label_new (y_string);
    window->xc_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (window->xc_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (window->xc_entry), FALSE);
    window->yc_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (window->yc_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (window->yc_entry), FALSE);

    /* plus/minus (+/-) symbol labels */
    pm_label = gtk_label_new (pm_string);
    pm_label2 = gtk_label_new (pm_string);
    /* labels and error text entries */
    window->xerr_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (window->xerr_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (window->xerr_entry), FALSE);
    window->yerr_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (window->yerr_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (window->yerr_entry), FALSE);

    /* Number of points label and entry */
    nump_label = gtk_label_new (nump_string);
    window->nump_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (window->nump_entry), 10);
    gtk_editable_set_editable (GTK_EDITABLE (window->nump_entry),FALSE);
    gtk_entry_set_text (GTK_ENTRY (window->nump_entry), "0");

    /* Packing the status area */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    status_area_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (status_area_label), status_area_header);
    gtk_widget_set_halign (status_area_label, GTK_ALIGN_START);
    gtk_widget_set_valign (status_area_label, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (vbox), status_area_label, FALSE, FALSE, 0);

    table = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (table), 0);
    gtk_grid_set_column_spacing (GTK_GRID (table), 0);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (table), x_label, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (table), window->xc_entry, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (table), pm_label, 2, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (table), window->xerr_entry, 3, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (table), y_label, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (table), window->yc_entry, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (table), pm_label2, 2, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (table), window->yerr_entry, 3, 1, 1, 1);

    /* Pack number of points boxes */
    table = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_widget_set_halign (nump_label, GTK_ALIGN_START);
    gtk_widget_set_valign (nump_label, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (table), nump_label, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (table), window->nump_entry, 1, 0, 1, 1);

    return vbox;
}


/* Add remove point and remove all buttons. */
static GtkWidget *g3data_window_remove_buttons_add (G3dataWindow *window) {
    GtkWidget *vbox, *subvbox;

    window->remove_last_button = gtk_button_new_with_mnemonic (remove_last_button_text);
    gtk_widget_set_sensitive (window->remove_last_button, FALSE);
    gtk_widget_set_tooltip_text (window->remove_last_button, remove_last_tooltip);
    g_signal_connect (G_OBJECT (window->remove_last_button), "clicked",
                  G_CALLBACK (remove_last), (gpointer) window);


    window->remove_all_button = gtk_button_new_with_mnemonic (remove_all_button_text);
    gtk_widget_set_sensitive (window->remove_all_button, FALSE);
    gtk_widget_set_tooltip_text (window->remove_all_button, remove_all_tooltip);
    g_signal_connect (G_OBJECT (window->remove_all_button), "clicked",
                  G_CALLBACK (remove_all), (gpointer) window);

    /* Pack remove points buttons */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    subvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), subvbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), window->remove_last_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), window->remove_all_button, FALSE, FALSE, 0);

    return vbox;
}


/* Add zoom area */
static GtkWidget *g3data_window_zoom_area_add (G3dataWindow *window) {
    GtkWidget *vbox, *label;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), zoom_area_header);

    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    window->zoom_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (window->zoom_area, ZOOMPIXSIZE, ZOOMPIXSIZE);
    g_signal_connect (G_OBJECT (window->zoom_area), "draw", G_CALLBACK (zoom_area_draw), (gpointer) window);

    gtk_box_pack_start (GTK_BOX (vbox), window->zoom_area, FALSE, FALSE, 0);

    return vbox;
}


/* Add buttons to toggle if x or y axis is logarithmic. */
static GtkWidget *g3data_window_log_buttons_add (G3dataWindow *window, struct g3data_options *options) {
    GtkWidget *vbox, *label;

    /* Logarithmic axes */
    window->x_log = gtk_check_button_new_with_mnemonic(x_log_text);
    gtk_widget_set_tooltip_text (window->x_log, x_log_tooltip);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (window->x_log), options->x_is_log);
    g_signal_connect (G_OBJECT (window->x_log), "toggled", G_CALLBACK (log_button_callback), (gpointer) window);

    window->y_log = gtk_check_button_new_with_mnemonic(y_log_text);
    gtk_widget_set_tooltip_text (window->y_log, y_log_tooltip);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (window->y_log), options->y_is_log);
    g_signal_connect (G_OBJECT (window->y_log), "toggled", G_CALLBACK (log_button_callback), (gpointer) window);

    /* Pack logarithmic axes */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), log_header);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), window->x_log, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), window->y_log, FALSE, FALSE, 0);

    return vbox;
}


/* Add radio buttons for sorting output. */
static GtkWidget *g3data_window_sort_buttons_add (G3dataWindow *window) {
    int i;
    GtkWidget *vbox, *label;
    GSList *group = NULL;

    for (i = 0; i < 3; i++) {
        window->sort_button[i] = gtk_radio_button_new_with_label (group, sort_button_text[i]);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (window->sort_button[i]));
    }
    /* Set no ordering button active */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (window->sort_button[0]), TRUE);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), sort_header);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    for (i = 0; i < 3; i++) {
        gtk_box_pack_start (GTK_BOX (vbox), window->sort_button[i], FALSE, FALSE, 0);
    }

    return vbox;
}


static void error_button_callback (GtkWidget *widget)
{
    use_error = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}


/* Add check buttons for including errors in output. */
static GtkWidget *g3data_window_error_buttons_add (void) {
    GtkWidget *vbox, *label;
    GtkWidget *error_button;

    /* Create and pack value errors button */
    error_button = gtk_check_button_new_with_mnemonic (error_button_text);
    gtk_widget_set_tooltip_text (error_button, error_tooltip);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (error_button), use_error);
    g_signal_connect (G_OBJECT (error_button), "toggled", G_CALLBACK (error_button_callback), NULL);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), error_header);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_END);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), error_button, FALSE, FALSE, 0);

    return vbox;
}


/* Insert image into g3data_window */
static gint g3data_image_insert (G3dataWindow *window,
                                 const gchar *filename,
                                 GtkWidget *container,
                                 struct g3data_options *options) {
    gboolean has_alpha;
    gint w, h;
    gint width = options->width;
    gint height = options->height;
    gdouble scale = options->scale;
    GdkPixbuf *temp_pixbuf;
    GtkWidget *dialog;

    temp_pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    /* If unable to load image, notify user with dialog */
    if (temp_pixbuf == NULL) {
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_CLOSE,
                        "Error loading file '%s'",
                        filename);
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
        return -1;
    }

    w = gdk_pixbuf_get_width (temp_pixbuf);
    h = gdk_pixbuf_get_height (temp_pixbuf);
    has_alpha = gdk_pixbuf_get_has_alpha (temp_pixbuf);

    if (scale == G_MAXDOUBLE) {
        if (width == -1 && height != -1) {
            scale = (gdouble) height / (gdouble) h;
        } else if (width != -1 && height == -1) {
            scale = (gdouble) width / (gdouble) w;
        } else if (width != -1 && height != -1) {
            scale = MIN( (gdouble) width / (gdouble) w, (gdouble) height / (gdouble) h);
        }
    }

    if (scale != G_MAXDOUBLE) {
        w = w * scale;
        h = h * scale;
        window->image = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, w, h);
        gdk_pixbuf_composite (temp_pixbuf, window->image, 0, 0, w, h,
                             0, 0, scale, scale, GDK_INTERP_BILINEAR, 255);
        g_object_unref (temp_pixbuf);
    } else {
        window->image = temp_pixbuf;
    }

    window->drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (window->drawing_area, w, h);

    g_signal_connect (G_OBJECT (window->drawing_area), "motion_notify_event",
                      G_CALLBACK (motion_notify_event), (gpointer) window);

    g_signal_connect (G_OBJECT (window->drawing_area), "draw",
                      G_CALLBACK (image_area_draw), (gpointer) window);

    g_signal_connect (G_OBJECT (window->drawing_area), "button_press_event",
                      G_CALLBACK (button_press_event), (gpointer) window);

    gtk_widget_set_events (window->drawing_area, GDK_EXPOSURE_MASK |
                                         GDK_BUTTON_PRESS_MASK | 
                                         GDK_BUTTON_RELEASE_MASK |
                                         GDK_POINTER_MOTION_MASK);

    /* Keep the image size and top-left position: a stretched drawing
     * area would accept clicks outside the image */
    gtk_widget_set_halign (window->drawing_area, GTK_ALIGN_START);
    gtk_widget_set_valign (window->drawing_area, GTK_ALIGN_START);
    gtk_container_add (GTK_CONTAINER (container), window->drawing_area);

    gtk_widget_show (window->drawing_area);

    return 0;
}


/* Draw callback for image area. */
static gboolean image_area_draw (GtkWidget *widget G_GNUC_UNUSED, cairo_t *cr, gpointer data) {
    gint i;
    G3dataWindow *window = G3DATA_WINDOW (data);

    gdk_cairo_set_source_pixbuf (cr, window->image, 0, 0);
    cairo_paint (cr);

    for (i = 0; i < 4; i++) {
        if (window->control_point_image_coords[i][0] != -1 &&
            window->control_point_image_coords[i][1] != -1) {
            DrawMarker (cr, window->control_point_image_coords[i][0], window->control_point_image_coords[i][1], i/2);
        }
    }
    for (i = 0; i < window->numpoints; i++) {
        DrawMarker(cr, window->points[i][0], window->points[i][1], 2);
    }

    return FALSE;
}   


/* Draw callback for the zoom area. */
static gboolean zoom_area_draw (GtkWidget *widget G_GNUC_UNUSED, cairo_t *cr, gpointer data) {
    gint x, y, width, height;
    G3dataWindow *window = G3DATA_WINDOW (data);

    x = window->x;
    y = window->y;
    width = gdk_pixbuf_get_width (window->image);
    height = gdk_pixbuf_get_height (window->image);

    if (x >= 0 && y >= 0 && x < width && y < height) {
        cairo_save(cr);
        cairo_translate(cr, -x*ZOOMFACTOR + ZOOMPIXSIZE/2, -y*ZOOMFACTOR + ZOOMPIXSIZE/2);
        cairo_scale(cr, 1.0*ZOOMFACTOR, 1.0*ZOOMFACTOR);
        gdk_cairo_set_source_pixbuf (cr, window->image, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }

    /* Then draw the square in the middle of the zoom area */
    DrawMarker (cr, ZOOMPIXSIZE/2, ZOOMPIXSIZE/2, 2);
    return TRUE;
}


/* Motion notify callback, for motion over drawing_area */
static gboolean motion_notify_event(GtkWidget *widget G_GNUC_UNUSED, GdkEventMotion *event, gpointer data) {
    gboolean control_points_set = FALSE;
    gboolean islogarithmic[2];
    gint x, y, width, height;
    gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
    struct PointValue CalcVal;
    G3dataWindow *window = G3DATA_WINDOW (data);

    /* Rounded like the gdk_window_get_pointer() call used before */
    x = round (event->x);
    y = round (event->y);
    window->x = x;
    window->y = y;

    width = gdk_pixbuf_get_width (window->image);
    height = gdk_pixbuf_get_height (window->image);

    /* If the control points have been set and their coordinates specified,
       then set control_points_set to TRUE. */
    if (gtk_entry_get_text_length (GTK_ENTRY (window->control_point_entry[0])) != 0 &&
        gtk_entry_get_text_length (GTK_ENTRY (window->control_point_entry[1])) != 0 &&
        gtk_entry_get_text_length (GTK_ENTRY (window->control_point_entry[2])) != 0 &&
        gtk_entry_get_text_length (GTK_ENTRY (window->control_point_entry[3])) != 0) {
        control_points_set = TRUE;
    } else {
        control_points_set = FALSE;
    }

    /* If pointer over image and control_point_set, then print the coordinates. */
    if (x >= 0 && y >= 0 && x < width && y < height && control_points_set == TRUE) {
        islogarithmic[0] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->x_log));
        islogarithmic[1] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->y_log));
        CalcVal = CalcPointValue (x, y, window->control_point_image_coords,
                                 window->control_point_coords, islogarithmic);
        g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%.5f", CalcVal.Xv);
        gtk_entry_set_text (GTK_ENTRY (window->xc_entry), buf);
        g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%.5f", CalcVal.Yv);
        gtk_entry_set_text (GTK_ENTRY (window->yc_entry), buf);
        g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%.5f", CalcVal.Xerr);
        gtk_entry_set_text (GTK_ENTRY (window->xerr_entry), buf);
        g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%.5f", CalcVal.Yerr);
        gtk_entry_set_text (GTK_ENTRY (window->yerr_entry), buf);
    } else {
        gtk_entry_set_text (GTK_ENTRY (window->xc_entry), "");
        gtk_entry_set_text (GTK_ENTRY (window->yc_entry), "");
        gtk_entry_set_text (GTK_ENTRY (window->xerr_entry), "");
        gtk_entry_set_text (GTK_ENTRY (window->yerr_entry), "");
    }

    gtk_widget_queue_draw (window->zoom_area);

    return TRUE;
}


/* When a control point toggle button is toggled active, make it insensitive,
   and make sensitive the corresponding text entry. */
static void control_point_button_toggled (GtkWidget *widget, gpointer data)
{
    gint i;
    G3dataWindow *window = G3DATA_WINDOW (data);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        for (i = 0; i < 4; i++) {
            gtk_widget_set_sensitive (window->control_point_button[i], FALSE);
            if (widget == window->control_point_button[i]) {
                gtk_widget_set_sensitive (window->control_point_entry[i], TRUE);
            }
        }
    }
}


/* Read text in control_point_entry when it changes. */
static void control_point_entry_read (GtkWidget *entry, gpointer func_data)
{
    G3dataWindow *window = G3DATA_WINDOW (func_data);
    const gchar *text;
    gint i;
    
    text = gtk_entry_get_text (GTK_ENTRY (entry));
    for (i = 0; i < 4; i++) {
        if (entry == window->control_point_entry[i]) {
            sscanf (text, "%lf", &window->control_point_coords[i]);
        }
    }
}


static void button_press_event (GtkWidget *widget G_GNUC_UNUSED, GdkEventButton *event, gpointer data)
{
    gint x, y, i, j;
    G3dataWindow *window = G3DATA_WINDOW (data);

    x = round (event->x);
    y = round (event->y);

    if (event->button == 1) {
        /* If none of the control point buttons have been pressed */
        if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[0])) &&
            !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[1])) &&
            !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[2])) &&
            !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[3])) ) {

            if (window->numpoints >= window->size) {
                window->points = (gint **) g_realloc (window->points, sizeof (gint *) * (window->size + MAXPOINTS));
                window->size += MAXPOINTS;
                for (i = window->numpoints; i < window->size; i++) {
                    window->points[i] = g_malloc (sizeof (gint) * 2);
                }
            }
            window->points[window->numpoints][0] = x;
            window->points[window->numpoints][1] = y;
            window->numpoints++;
            SetNumPointsEntry (window->nump_entry, window->numpoints);
        } else {
            for (i = 0; i < 4; i++) {
                /* If any of the control point buttons have been pressed */
                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[i]))) {
                    window->control_point_image_coords[i][0] = x;
                    window->control_point_image_coords[i][1] = y;
                    for (j = 0; j < 4; j++) {
                        gtk_widget_set_sensitive (window->control_point_button[j], TRUE);
                    }
                    gtk_widget_set_sensitive (window->control_point_entry[i], TRUE);
                    gtk_editable_set_editable (GTK_EDITABLE (window->control_point_entry[i]), TRUE);
                    gtk_widget_grab_focus (window->control_point_entry[i]);
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(window->control_point_button[i]), FALSE);
                }
            }
        }
    } else if (event->button == 2) {
        for (i = 0; i < 2; i++) {
            /* If mouse button 2 pressed, and the control points have not been set, set control points for x-axis */
            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[i])) == FALSE &&
                window->control_point_image_coords[i][0] == -1 &&
                window->control_point_image_coords[i][1] == -1) {

                window->control_point_image_coords[i][0] = x;
                window->control_point_image_coords[i][1] = y;
                for (j = 0; j < 4; j++) {
                    gtk_widget_set_sensitive(window->control_point_button[j], TRUE);
                }
                gtk_widget_set_sensitive (window->control_point_entry[i], TRUE);
                gtk_editable_set_editable (GTK_EDITABLE (window->control_point_entry[i]), TRUE);
                gtk_widget_grab_focus (window->control_point_entry[i]);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(window->control_point_button[i]), FALSE);

                break;
            }
        }
    } else if (event->button == 3) {
        for (i = 2; i < 4; i++) {
            /* If mouse button 3 pressed, and the control points have not been set, set control points for y-axis */
            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->control_point_button[i])) == FALSE &&
                window->control_point_image_coords[i][0] == -1 &&
                window->control_point_image_coords[i][1] == -1) {

                window->control_point_image_coords[i][0] = x;
                window->control_point_image_coords[i][1] = y;
                for (j = 0; j < 4; j++) {
                    gtk_widget_set_sensitive(window->control_point_button[j], TRUE);
                }
                gtk_widget_set_sensitive (window->control_point_entry[i], TRUE);
                gtk_editable_set_editable (GTK_EDITABLE (window->control_point_entry[i]), TRUE);
                gtk_widget_grab_focus (window->control_point_entry[i]);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(window->control_point_button[i]), FALSE);

                break;
            }
        }
    }
    gtk_widget_queue_draw (window->drawing_area);
    SetButtonSensitivity (window);
}



/* Set sensitivity of remove buttons */
static void SetButtonSensitivity (G3dataWindow *window)
{
    if (window->numpoints == 0 &&
        window->control_point_image_coords[0][0] == -1 &&
        window->control_point_image_coords[1][0] == -1 &&
        window->control_point_image_coords[2][0] == -1 &&
        window->control_point_image_coords[3][0] == -1) {
        gtk_widget_set_sensitive (window->remove_last_button, FALSE);
        gtk_widget_set_sensitive (window->remove_all_button, FALSE);
    } else if (window->numpoints == 0 &&
              (window->control_point_image_coords[0][0] != -1 ||
               window->control_point_image_coords[1][0] != -1 ||
               window->control_point_image_coords[2][0] != -1 ||
               window->control_point_image_coords[3][0] != -1)) {
        gtk_widget_set_sensitive (window->remove_last_button, FALSE);
        gtk_widget_set_sensitive (window->remove_all_button, TRUE);
    } else {
        gtk_widget_set_sensitive (window->remove_last_button, TRUE);
        gtk_widget_set_sensitive (window->remove_all_button, TRUE);
    }
}


/* Removes the last data point inserted */
static void remove_last (GtkWidget *widget G_GNUC_UNUSED, gpointer data)
{
    G3dataWindow *window = G3DATA_WINDOW (data);

    /* If there are any points, remove one. */
    if (window->numpoints > 0) {
        window->points[window->numpoints][0] = -1;
        window->points[window->numpoints][1] = -1;
        window->numpoints--;
        SetNumPointsEntry (window->nump_entry, window->numpoints);
    }

    SetButtonSensitivity (window);
    gtk_widget_queue_draw (window->drawing_area);
}


/* Remove all data points and control points. */
static void remove_all (GtkWidget *widget, gpointer data) 
{
    gint i;
    G3dataWindow *window = G3DATA_WINDOW (data);

    /* set control_point_image_coords to -1, so the axis points do not get drawn*/
    for (i = 0; i < 4; i++) {
        window->control_point_image_coords[i][0] = -1;
        window->control_point_image_coords[i][1] = -1;
        /* Clear control points text entries, make buttons insensitive */
	    gtk_entry_set_text (GTK_ENTRY(window->control_point_entry[i]), "");
        gtk_widget_set_sensitive (window->control_point_entry[i], FALSE);
    }

    window->numpoints = 0;
    SetNumPointsEntry (window->nump_entry, window->numpoints);

    remove_last (widget, data);
}


static void log_button_callback (GtkWidget *widget, gpointer data)
{
    gboolean islogarithmic;
    gint i;
    G3dataWindow *window = G3DATA_WINDOW (data);

    islogarithmic = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    if (widget == window->x_log)
        i = 0;
    else if (widget == window->y_log)
        i = 2;
    else
        return;
    
    if (islogarithmic == TRUE) {
        if (window->control_point_coords[i] <= 0.0) {
            gtk_entry_set_text (GTK_ENTRY (window->control_point_entry[i]), "");
        }
        if (window->control_point_coords[i + 1] <= 0.0) {
            gtk_entry_set_text (GTK_ENTRY (window->control_point_entry[i + 1]), "");
        }
    }
}


/* Callback for key press events in viewport showing image */
static void key_press_event(GtkWidget *widget, GdkEventKey *event)
{
    GtkAdjustment *adjustment;
    gdouble adj_val;

    if (widget != NULL) {
        if (event->keyval == GDK_KEY_Left) {
            adjustment = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (widget));
            adj_val = gtk_adjustment_get_value (adjustment);
            adj_val -= gtk_adjustment_get_page_size (adjustment) / 10.0;
            if (adj_val < gtk_adjustment_get_lower (adjustment))
                adj_val = gtk_adjustment_get_lower (adjustment);
            gtk_adjustment_set_value (adjustment, adj_val);
            gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (widget), adjustment);
        } else if (event->keyval == GDK_KEY_Right) {
            adjustment = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (widget));
            adj_val = gtk_adjustment_get_value (adjustment);
            adj_val += gtk_adjustment_get_page_size (adjustment) / 10.0;
            if (adj_val > (gtk_adjustment_get_upper (adjustment) - gtk_adjustment_get_page_size (adjustment)))
                adj_val = (gtk_adjustment_get_upper (adjustment) - gtk_adjustment_get_page_size (adjustment));
            gtk_adjustment_set_value (adjustment, adj_val);
            gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (widget), adjustment);
        } else if (event->keyval == GDK_KEY_Up) {
            adjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (widget));
            adj_val = gtk_adjustment_get_value (adjustment);
            adj_val -= gtk_adjustment_get_page_size (adjustment) / 10.0;
            if (adj_val < gtk_adjustment_get_lower (adjustment))
                adj_val = gtk_adjustment_get_lower (adjustment);
            gtk_adjustment_set_value (adjustment, adj_val);
            gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (widget), adjustment);
        } else if (event->keyval == GDK_KEY_Down) {
            adjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (widget));
            adj_val = gtk_adjustment_get_value (adjustment);
            adj_val += gtk_adjustment_get_page_size (adjustment) / 10.0;
            if (adj_val > (gtk_adjustment_get_upper (adjustment) - gtk_adjustment_get_page_size (adjustment)))
                adj_val = (gtk_adjustment_get_upper (adjustment) - gtk_adjustment_get_page_size (adjustment));
            gtk_adjustment_set_value (adjustment, adj_val);
            gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (widget), adjustment);
        }
    }
}
