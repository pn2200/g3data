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

#include <stdlib.h>
#include "g3data-application.h"
#include "g3data-window.h"
#include "g3data-image.h"
#include "g3data-about.h"
#include "main.h"
#include "points.h"
#include "sort.h"


G_DEFINE_TYPE (G3dataWindow, g3data_window, GTK_TYPE_WINDOW);

extern gboolean use_error;

static void update_preview_cb (GtkFileChooser *chooser, gpointer data);
static void file_open_dialog_response_cb (GtkWidget *chooser,
                                          gint response_id,
                                          GtkAdjustment *scaleadj);
static void g3data_window_file_open (GtkAction *action, G3dataWindow *window);
static void g3data_window_print_results(FILE *fp, G3dataWindow *window);
static void g3data_window_file_save_as (GtkAction *action, G3dataWindow *window);
static void g3data_window_file_close (GtkAction *action, G3dataWindow *window);
static void g3data_window_help_about (GtkAction *action, G3dataWindow *window);
static void full_screen_action_callback(GtkWidget *widget, gpointer func_data);
static void hide_zoom_area_callback(GtkWidget *widget, gpointer func_data);
static void hide_log_buttons_callback(GtkWidget *widget, gpointer func_data);
static void hide_sort_buttons_callback(GtkWidget *widget, gpointer func_data);


static const GtkActionEntry entries[] = {
    { "FileMenu", NULL, "_File", NULL, NULL, NULL },
    { "ViewMenu", NULL, "_View", NULL, NULL, NULL },
    { "HelpMenu", NULL, "_Help", NULL, NULL, NULL },
    { "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open an image in a new tab", G_CALLBACK( g3data_window_file_open ) },
    { "Save As", GTK_STOCK_SAVE_AS, "Save _As...", "<control><shift>S", "Save data", G_CALLBACK( g3data_window_file_save_as ) },
    { "Close", GTK_STOCK_CLOSE, "_Close", "<control>W", "Close window", G_CALLBACK( g3data_window_file_close ) },
    { "About", GTK_STOCK_HELP, "_About", "", "About g3data", G_CALLBACK( g3data_window_help_about ) }
};

static const GtkToggleActionEntry toggle_entries[] = {
    { "Zoom area", NULL, "Zoom area", "F5", "Zoom area", G_CALLBACK( hide_zoom_area_callback ), TRUE },
    { "Axis settings", NULL, "Axis settings", "F6", "Axis settings", G_CALLBACK( hide_log_buttons_callback ), TRUE },
    { "Output properties", NULL, "Output properties", "F7", "Output properties", G_CALLBACK( hide_sort_buttons_callback ), TRUE }
};

static const GtkToggleActionEntry full_screen[] = {
    { "FullScreen", NULL, "_Full Screen", "F11", "Switch between full screen and windowed mode", G_CALLBACK( full_screen_action_callback ), FALSE }
};

static const gchar *ui_description =
    "<ui>"
    "  <menubar name='MainMenu'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='Open'/>"
    "      <menuitem action='Save As'/>"
    "      <separator />"
    "      <menuitem action='Close'/>"
    "    </menu>"
    "    <menu action='ViewMenu'>"
    "      <menuitem action='Zoom area'/>"
    "      <menuitem action='Axis settings'/>"
    "      <menuitem action='Output properties'/>"
    "      <separator />"
    "      <menuitem action='FullScreen'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='About'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";


void g3data_set_default_options (struct g3data_options *options) {
    if (options == NULL) {
        return;
    }

    options->height = -1;
    options->width = -1;
    options->scale = G_MAXDOUBLE;
    options->x_is_log = FALSE;
    options->y_is_log = FALSE;
    options->control_point_coords[0] = G_MAXDOUBLE;
    options->control_point_coords[1] = G_MAXDOUBLE;
    options->control_point_coords[2] = G_MAXDOUBLE;
    options->control_point_coords[3] = G_MAXDOUBLE;
}


static void update_preview_cb (GtkFileChooser *chooser, gpointer data) {
    GtkWidget *preview;
    GdkPixbuf *pixbuf;
    gchar *filename;
    gboolean have_preview;

    preview = GTK_WIDGET (data);
    filename = gtk_file_chooser_get_preview_filename (chooser);

    if (filename != NULL) {
        pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 128, 128, NULL);
        have_preview = (pixbuf != NULL);
        g_free (filename);

        gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);

        if (pixbuf)
            g_object_unref (pixbuf);

        gtk_file_chooser_set_preview_widget_active (chooser, have_preview);
    }
}


static void file_open_dialog_response_cb (GtkWidget *chooser,
                                          gint response_id,
                                          GtkAdjustment *scaleadj)
{
    G3dataApplication *instance;
    instance = g3data_application_get_instance ();

    if (response_id == GTK_RESPONSE_ACCEPT) {
        gchar *filename;
        gdouble scale;
        struct g3data_options *options = g_malloc0 (sizeof (struct g3data_options));

        g3data_set_default_options (options);
        scale = gtk_adjustment_get_value (scaleadj);
        options->scale = scale;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
        if (instance->current_window == NULL || instance->current_window->image != NULL) {
            g3data_create_window (instance);
        }
        g3data_window_insert_image (instance->current_window, filename, options);

        g_free (filename);
    }

    gtk_widget_destroy (chooser);
    gtk_window_present (GTK_WINDOW (instance->current_window));
}


static void g3data_window_file_open (GtkAction *action, G3dataWindow *window)
{
    GtkWidget *dialog, *scalespinbutton, *hboxextra, *scalelabel;
    GtkImage *preview;
    GtkAdjustment *scaleadj;
    GtkFileFilter *filefilter;
    const gchar scale_string[] = "Scale image : ";

    dialog = gtk_file_chooser_dialog_new ("Open Image",
                                          GTK_WINDOW (window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    /* Set filtering of files to open to filetypes gdk_pixbuf can handle */
    filefilter = gtk_file_filter_new ();
    gtk_file_filter_add_pixbuf_formats (filefilter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog),
                                 GTK_FILE_FILTER (filefilter));

    hboxextra = gtk_hbox_new (FALSE, 0);

    scalelabel = gtk_label_new (scale_string);

    scaleadj = GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.1, 100, 0.1, 0.1, 0));
    scalespinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (scaleadj), 0.1, 1);

    gtk_box_pack_start (GTK_BOX (hboxextra), scalelabel, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hboxextra), scalespinbutton, FALSE, FALSE, 0);

    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(dialog), hboxextra);

    gtk_widget_show (hboxextra);
    gtk_widget_show (scalelabel);
    gtk_widget_show (scalespinbutton);

    preview = GTK_IMAGE (gtk_image_new ());
    gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (dialog), GTK_WIDGET (preview));
    g_signal_connect (dialog,
                      "update-preview",
                      G_CALLBACK (update_preview_cb),
                      preview);

    g_signal_connect (dialog,
                      "response",
                      G_CALLBACK (file_open_dialog_response_cb),
                      scaleadj);

    gtk_widget_show (dialog);
}


static void g3data_window_print_results(FILE *fp, G3dataWindow *window)
{
    gboolean islogarithmic[2];
    gint i;
    gint ordering = 0;
    gint n = window->numpoints;
    struct PointValue *RealPos;

    RealPos = (struct PointValue *) g_malloc(sizeof(struct PointValue) * n);

    islogarithmic[0] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->x_log));
    islogarithmic[1] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->y_log));

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->sort_button[1]))) {
        ordering = 1;
    } else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->sort_button[2]))) {
        ordering = 2;
    }

    /* Calculate the real positions of the points */
    for (i = 0; i < n; i++) {
        RealPos[i] = CalcPointValue (window->points[i][0],
                                     window->points[i][1],
                                     window->control_point_image_coords,
                                     window->control_point_coords,
                                     islogarithmic);
    }

    if (ordering != 0) {
        Order (RealPos, n, ordering);
    }

    for (i = 0; i < n; i++) {
        fprintf (fp, "%.12g  %.12g", RealPos[i].Xv, RealPos[i].Yv);
        if (use_error) {
            fprintf (fp, "\t%.12g  %.12g\n", RealPos[i].Xerr, RealPos[i].Yerr);
        } else fprintf(fp, "\n");
    }

    g_free (RealPos);

    fclose (fp);
}


static void g3data_window_file_save_as (GtkAction *action, G3dataWindow *window)
{
    GtkWidget *dialog;
    gchar *filename;
    FILE *fp;

    dialog = gtk_file_chooser_dialog_new ("Save As...",
                                          GTK_WINDOW (window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_current_name(dialog, "untitled.txt");

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        fp = fopen (filename, "w");
        if (fp == NULL) {
            g_free (filename);
            return;
        } else {
            g3data_window_print_results (fp, window);
            g_free (filename);
        }
    }

    gtk_widget_destroy (dialog);

    return;
}


static void g3data_window_file_close (GtkAction *action, G3dataWindow *window)
{
    /* if this was called from the uimanager, destroy the widget;
    * otherwise, if it was called from the delete_event, the widget
    * will destroy itself.
    */
    if (action != NULL)
        gtk_widget_destroy (GTK_WIDGET (window));
}


static void full_screen_action_callback(GtkWidget *widget, gpointer func_data) {
    G3dataWindow *window = G3DATA_WINDOW (func_data);

    if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget))) {
        gtk_window_fullscreen (GTK_WINDOW (window));
    } else {
        gtk_window_unfullscreen (GTK_WINDOW (window));
    }
}


/* Hide or show zoom area. */
static void hide_zoom_area_callback(GtkWidget *widget, gpointer func_data) {
    G3dataWindow *window = G3DATA_WINDOW (func_data);

    if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget))) {
        gtk_widget_show (window->zoom_area_vbox);
    } else {
        gtk_widget_hide (window->zoom_area_vbox);
    }
}


/* Hide or show logarithmic axes buttons. */
static void hide_log_buttons_callback(GtkWidget *widget, gpointer func_data) {
    G3dataWindow *window = G3DATA_WINDOW (func_data);

    if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget))) {
        gtk_widget_show (window->log_buttons_vbox);
    } else {
        gtk_widget_hide (window->log_buttons_vbox);
    }
}


/* Hide or show output sorting buttons. */
static void hide_sort_buttons_callback(GtkWidget *widget, gpointer func_data) {
    G3dataWindow *window = G3DATA_WINDOW (func_data);

    if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (widget))) {
        gtk_widget_show (window->sort_buttons_vbox);
    } else {
        gtk_widget_hide (window->sort_buttons_vbox);
    }
}


static void g3data_window_help_about (GtkAction *action, G3dataWindow *window)
{
    g3data_about (GTK_WIDGET (window));
}


static void g3data_window_init (G3dataWindow *g3data_window)
{
    int i;
    GtkWidget *menubar;
    GtkActionGroup *action_group;
    GtkUIManager *ui_manager;
    GtkAccelGroup *accel_group;
    GError *error = NULL;

    gtk_window_set_default_size (GTK_WINDOW (g3data_window), 640, 480);
    gtk_window_set_title (GTK_WINDOW (g3data_window), "g3data - no file");
    gtk_window_set_resizable (GTK_WINDOW (g3data_window), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (g3data_window), 0);

    g3data_window->main_vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (g3data_window), g3data_window->main_vbox);

    /* Create menu */
    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), g3data_window);
    gtk_action_group_add_toggle_actions (action_group, full_screen, G_N_ELEMENTS (full_screen), g3data_window);

    g3data_window->action_group = gtk_action_group_new ("WindowActions");
    gtk_action_group_add_toggle_actions (g3data_window->action_group,
                                         toggle_entries,
                                         G_N_ELEMENTS (toggle_entries),
                                         g3data_window);
    gtk_action_group_set_sensitive (g3data_window->action_group, FALSE);

    ui_manager = gtk_ui_manager_new ();
    gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
    gtk_ui_manager_insert_action_group (ui_manager, g3data_window->action_group, 0);

    accel_group = gtk_ui_manager_get_accel_group (ui_manager);
    gtk_window_add_accel_group (GTK_WINDOW (g3data_window), accel_group);
 
    error = NULL;
    if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
        g_message("building menus failed: %s", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
     }
 
    menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (g3data_window->main_vbox), menubar, FALSE, FALSE, 0);

    g3data_window->image = NULL;
    for (i = 0; i < 4; i++) {
        g3data_window->control_point_image_coords[i][0] = -1;
        g3data_window->control_point_image_coords[i][1] = -1;
    }
}


static void g3data_window_class_init (G3dataWindowClass *g3data_window_class)
{
}


G3dataWindow *g3data_window_new (void)
{
    G3dataWindow *g3data_window;

    g3data_window = g_object_new (G3DATA_TYPE_WINDOW,
                                  "type", GTK_WINDOW_TOPLEVEL,
                                  NULL);

    return g3data_window;
}
