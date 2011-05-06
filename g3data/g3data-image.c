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
#include <math.h>
#include "g3data-window.h"
#include "g3data-image.h"

#define ZOOMPIXSIZE 200
#define ZOOMFACTOR 4

static GtkWidget *g3data_window_control_points_add (void);
static GtkWidget *g3data_window_status_area_add (void);
static GtkWidget *g3data_window_remove_buttons_add (void);
static GtkWidget *g3data_window_zoom_area_add (G3dataWindow *window);
static GtkWidget *g3data_window_log_buttons_add (G3dataWindow *window);
static GtkWidget *g3data_window_sort_buttons_add (void);
static GtkWidget *g3data_window_error_buttons_add (void);
static gint g3data_image_insert (G3dataWindow *window, const gchar *filename, GtkWidget *drawing_area_alignment);

/* Callbacks */
static gboolean image_area_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data);

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


void g3data_window_insert_image (G3dataWindow *window, const gchar *filename)
{
    GtkWidget *table, *tophbox, *bottomhbox, *bottomvbox, *alignment,
              *scrolled_window, *viewport, *drawing_area_alignment;
    GtkWidget *control_point_vbox, *status_area_vbox, *remove_buttons_vbox,
              *zoom_area_vbox, *log_buttons_vbox, *sort_buttons_vbox,
              *error_button_vbox;
    gchar *buffer;

    table = gtk_table_new (2, 2, FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (table), 0);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_box_pack_start (GTK_BOX (window->main_vbox), table, FALSE, FALSE, 0);

    tophbox = gtk_hbox_new (FALSE, 0);
    alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), tophbox);

    bottomhbox = gtk_hbox_new (FALSE, 0);
    alignment = gtk_alignment_new (0, 0, 1, 1);
    gtk_table_attach (GTK_TABLE (table), alignment, 0, 1, 1, 2, 5, 5, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), bottomhbox);

    bottomvbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (bottomhbox), bottomvbox, FALSE, FALSE, 0);

    control_point_vbox = g3data_window_control_points_add ();
    gtk_box_pack_start (GTK_BOX (tophbox), control_point_vbox, FALSE, FALSE, 0);

    status_area_vbox = g3data_window_status_area_add ();
    gtk_box_pack_start (GTK_BOX (tophbox), status_area_vbox, FALSE, FALSE, 0);

    remove_buttons_vbox = g3data_window_remove_buttons_add ();
    gtk_box_pack_start (GTK_BOX (bottomvbox), remove_buttons_vbox, FALSE, FALSE, 0);

    zoom_area_vbox = g3data_window_zoom_area_add (window);
    gtk_box_pack_start (GTK_BOX (bottomvbox), zoom_area_vbox, FALSE, FALSE, 0);

    log_buttons_vbox = g3data_window_log_buttons_add (window);
    gtk_box_pack_start (GTK_BOX (bottomvbox), log_buttons_vbox, FALSE, FALSE, 0);

    sort_buttons_vbox = g3data_window_sort_buttons_add ();
    gtk_box_pack_start (GTK_BOX (bottomvbox), sort_buttons_vbox, FALSE, FALSE, 0);

    error_button_vbox = g3data_window_error_buttons_add ();
    gtk_box_pack_start (GTK_BOX (bottomvbox), error_button_vbox, FALSE, FALSE, 0);

    /* Create a scrolled window to hold image */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    viewport = gtk_viewport_new (NULL, NULL);
    gtk_box_pack_start (GTK_BOX (bottomhbox), scrolled_window, TRUE, TRUE, 0);
    drawing_area_alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (viewport), drawing_area_alignment);
    gtk_container_add (GTK_CONTAINER (scrolled_window), viewport);

    g3data_image_insert (window, filename, drawing_area_alignment);

    /* Print current image name in title bar*/
    buffer = g_strdup_printf (g3data_window_title, g_path_get_basename (filename));
    gtk_window_set_title (GTK_WINDOW (window), buffer);
    g_free (buffer);

    gtk_widget_show_all (window->main_vbox);
}


/* Add control points area. */
static GtkWidget *g3data_window_control_points_add (void) {
    GtkWidget *control_point_header, *control_point_label;
    GtkWidget *vbox, *hbox, *alignment, *table, *label;
    GtkWidget *control_point_button[4], *control_point_entry[4];
    int i;

    vbox = gtk_vbox_new (FALSE, 0);

    /* Create header for control points section */
    control_point_header = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (control_point_header), control_point_header_text);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), control_point_header);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    table = gtk_table_new (3, 4 ,FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 0);

    for (i = 0; i < 4; i++) {
        /* buttons for setting control points x_1, x_2, etc. */
        label = gtk_label_new (NULL);
        gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), control_point_button_text[i]);
        control_point_button[i] = gtk_toggle_button_new();
        gtk_container_add (GTK_CONTAINER (control_point_button[i]), label);
        gtk_widget_set_tooltip_text (control_point_button[i], control_point_tooltip[i]);

        /* labels for control points x_1, x_2, etc. */
        control_point_label = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL (control_point_label), control_point_label_text[i]);

        /* text entries to enter control points x_1, x_2, etc. */
        control_point_entry[i] = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY (control_point_entry[i]), 20);
        gtk_widget_set_sensitive (control_point_entry[i], FALSE);
        gtk_widget_set_tooltip_text (control_point_entry[i], control_point_entry_tooltip[i]);

        /* Packing the control points labels and entries */
	    gtk_table_attach_defaults (GTK_TABLE (table), control_point_button[i], 0, 1, i, i+1);
	    gtk_table_attach_defaults (GTK_TABLE (table), control_point_label, 1, 2, i, i+1);
	    gtk_table_attach_defaults (GTK_TABLE (table), control_point_entry[i], 2, 3, i, i+1);
    }
    return vbox;
}


/* Add status area. */
static GtkWidget *g3data_window_status_area_add (void) {
    GtkWidget *x_label, *y_label, *xc_entry, *yc_entry;
    GtkWidget *pm_label, *pm_label2;
    GtkWidget *xerr_entry, *yerr_entry, *nump_label, *nump_entry;
    GtkWidget *status_area_label;
    GtkWidget *vbox, *alignment, *table;

    x_label = gtk_label_new (x_string);
    y_label = gtk_label_new (y_string);
    xc_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (xc_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (xc_entry), FALSE);
    yc_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (yc_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (yc_entry), FALSE);

    /* plus/minus (+/-) symbol labels */
    pm_label = gtk_label_new (pm_string);
    pm_label2 = gtk_label_new (pm_string);
    /* labels and error text entries */
    xerr_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (xerr_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (xerr_entry), FALSE);
    yerr_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (yerr_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (yerr_entry), FALSE);

    /* Number of points label and entry */
    nump_label = gtk_label_new (nump_string);
    nump_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (nump_entry), 10);
    gtk_editable_set_editable (GTK_EDITABLE (nump_entry),FALSE);
    gtk_entry_set_text (GTK_ENTRY (nump_entry), "0");

    /* Packing the status area */
    vbox = gtk_vbox_new (FALSE, 0);

    status_area_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (status_area_label), status_area_header);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), status_area_label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

    table = gtk_table_new (4, 2 ,FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), x_label, 0, 1, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), xc_entry, 1, 2, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), pm_label, 2, 3, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), xerr_entry, 3, 4, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), y_label, 0, 1, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE (table), yc_entry, 1, 2, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE (table), pm_label2, 2, 3, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE (table), yerr_entry, 3, 4, 1, 2);

    /* Pack number of points boxes */
    table = gtk_table_new (3, 1 ,FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), nump_label);
    gtk_table_attach (GTK_TABLE (table), alignment, 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach (GTK_TABLE (table), nump_entry, 1, 2, 0, 1, 0, 0, 0, 0);

    return vbox;
}


/* Add remove point and remove all buttons. */
static GtkWidget *g3data_window_remove_buttons_add (void) {
    GtkWidget *vbox, *subvbox;
    GtkWidget *remove_last_button, *remove_all_button;

    remove_last_button = gtk_button_new_with_mnemonic (remove_last_button_text);
    gtk_widget_set_sensitive (remove_last_button, FALSE);
    gtk_widget_set_tooltip_text (remove_last_button, remove_last_tooltip);

    remove_all_button = gtk_button_new_with_mnemonic (remove_all_button_text);
    gtk_widget_set_sensitive (remove_all_button, FALSE);
    gtk_widget_set_tooltip_text (remove_all_button, remove_last_tooltip);

    /* Pack remove points buttons */
    vbox = gtk_vbox_new (FALSE, 0);

    subvbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), subvbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), remove_last_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), remove_all_button, FALSE, FALSE, 0);

    return vbox;
}


/* Add zoom area */
static GtkWidget *g3data_window_zoom_area_add (G3dataWindow *window) {
    GtkWidget *vbox, *label, *alignment;

    vbox = gtk_vbox_new (FALSE, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), zoom_area_header);

    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

    window->zoom_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (window->zoom_area, ZOOMPIXSIZE, ZOOMPIXSIZE);
    g_signal_connect (G_OBJECT (window->zoom_area), "expose_event", G_CALLBACK (expose_event_callback), (gpointer) window);

    gtk_box_pack_start (GTK_BOX (vbox), window->zoom_area, FALSE, FALSE, 0);

    return vbox;
}


/* Add buttons to toggle if x or y axis is logarithmic. */
static GtkWidget *g3data_window_log_buttons_add (G3dataWindow *window) {
    GtkWidget *vbox, *label, *alignment;
    GtkWidget *x_log, *y_log;

    /* Logarithmic axes */
    x_log = gtk_check_button_new_with_mnemonic(x_log_text);
    gtk_widget_set_tooltip_text (x_log, x_log_tooltip);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (x_log), FALSE);

    y_log = gtk_check_button_new_with_mnemonic(y_log_text);
    gtk_widget_set_tooltip_text (y_log, y_log_tooltip);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (y_log), FALSE);

    /* Pack logarithmic axes */
    vbox = gtk_vbox_new (FALSE, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), log_header);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add ( GTK_CONTAINER(alignment), label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), x_log, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), y_log, FALSE, FALSE, 0);

    return vbox;
}


/* Add radio buttons for sorting output. */
static GtkWidget *g3data_window_sort_buttons_add (void) {
    int i;
    GtkWidget *vbox, *label, *alignment;
    GtkWidget *sort_button[3];
    GSList *group = NULL;

    for (i = 0; i < 3; i++) {
        sort_button[i] = gtk_radio_button_new_with_label (group, sort_button_text[i]);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (sort_button[i]));
    }
    /* Set no ordering button active */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sort_button[0]), TRUE);

    vbox = gtk_vbox_new (FALSE, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), sort_header);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER(alignment), label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
    for (i = 0; i < 3; i++) {
        gtk_box_pack_start (GTK_BOX (vbox), sort_button[i], FALSE, FALSE, 0);
    }

    return vbox;
}


/* Add check buttons for including errors in output. */
static GtkWidget *g3data_window_error_buttons_add (void) {
    GtkWidget *vbox, *label, *alignment;
    GtkWidget *error_button;

    /* Create and pack value errors button */
    error_button = gtk_check_button_new_with_mnemonic (error_button_text);
    gtk_widget_set_tooltip_text (error_button, error_tooltip);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (error_button), FALSE);

    vbox = gtk_vbox_new (FALSE, 0);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), error_header);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), error_button, FALSE, FALSE, 0);

    return vbox;
}


/* Insert image into g3data_window */
static gint g3data_image_insert (G3dataWindow *window, const gchar *filename, GtkWidget *drawing_area_alignment) {
    gboolean has_alpha;
    gint w, h;
    gint width = -1;
    gint height = -1;
    gdouble scale = -1;
    GdkPixbuf *temp_pixbuf;
    GtkWidget *dialog, *drawing_area;

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

    if (width != -1 && height != -1 && scale == -1) {
        if (w > width || h > height) {
            scale = fmin((double) (width/w), (double) (height/h));
        }
    }

    if (scale != -1) {
        w = w * scale;
        h = h * scale;
        window->image = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, w, h);
        gdk_pixbuf_composite (temp_pixbuf, window->image, 0, 0, w, h,
                             0, 0, scale, scale, GDK_INTERP_BILINEAR, 255);
        g_object_unref (temp_pixbuf);
    } else {
        window->image = temp_pixbuf;
    }

    drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (drawing_area, w, h);

    g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",
                      G_CALLBACK (motion_notify_event), (gpointer) window);

    g_signal_connect (G_OBJECT (drawing_area), "expose_event",
                      G_CALLBACK (image_area_expose_event), (gpointer) window);

    gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK |
                                         GDK_BUTTON_PRESS_MASK | 
                                         GDK_BUTTON_RELEASE_MASK |
                                         GDK_POINTER_MOTION_MASK | 
                                         GDK_POINTER_MOTION_HINT_MASK);

    gtk_container_add (GTK_CONTAINER (drawing_area_alignment), drawing_area);

    gtk_widget_show (drawing_area);

    return 0;
}


/* Expose event callback for image area. */
static gboolean image_area_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    G3dataWindow *window = G3DATA_WINDOW (data);
    cairo_t *cr = gdk_cairo_create (gtk_widget_get_window (widget));

    gdk_cairo_set_source_pixbuf (cr, window->image, 0, 0);
    cairo_paint (cr);

    cairo_destroy (cr);
    return FALSE;
}   


/* Expose event callback for the zoom area. */
static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    gint x, y, width, height;
    cairo_t *cr;
    G3dataWindow *window = G3DATA_WINDOW (data);

    x = window->x;
    y = window->y;
    width = gdk_pixbuf_get_width (window->image);
    height = gdk_pixbuf_get_height (window->image);

    cr = gdk_cairo_create (gtk_widget_get_window (widget));

    if (x >= 0 && y >= 0 && x < width && y < height) {
        cairo_save(cr);
        cairo_translate(cr, -x*ZOOMFACTOR + ZOOMPIXSIZE/2, -y*ZOOMFACTOR + ZOOMPIXSIZE/2);
        cairo_scale(cr, 1.0*ZOOMFACTOR, 1.0*ZOOMFACTOR);
        gdk_cairo_set_source_pixbuf (cr, window->image, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }

    cairo_destroy (cr);
    return TRUE;
}


/* Motion notify callback, for motion over drawing_area */
static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    gint x, y;
    G3dataWindow *window = G3DATA_WINDOW (data);

    gdk_window_get_pointer (event->window, &x, &y, NULL);
    window->x = x;
    window->y = y;

    gtk_widget_queue_draw (window->zoom_area);

    return TRUE;
}

