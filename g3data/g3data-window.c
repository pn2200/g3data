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

#include <errno.h>
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
static void g3data_window_file_open (GSimpleAction *action, GVariant *parameter, gpointer data);
static void g3data_window_print_results(FILE *fp, G3dataWindow *window);
static void g3data_window_file_save_as (GSimpleAction *action, GVariant *parameter, gpointer data);
static void g3data_window_file_close (GSimpleAction *action, GVariant *parameter, gpointer data);
static void g3data_window_help_about (GSimpleAction *action, GVariant *parameter, gpointer data);
static void full_screen_change_state (GSimpleAction *action, GVariant *state, gpointer data);
static void zoom_area_change_state (GSimpleAction *action, GVariant *state, gpointer data);
static void log_buttons_change_state (GSimpleAction *action, GVariant *state, gpointer data);
static void sort_buttons_change_state (GSimpleAction *action, GVariant *state, gpointer data);


/* Window actions, used by the menu as "win.<name>" */
static const GActionEntry win_entries[] = {
    { "open", g3data_window_file_open, NULL, NULL, NULL, {0} },
    { "save-as", g3data_window_file_save_as, NULL, NULL, NULL, {0} },
    { "close", g3data_window_file_close, NULL, NULL, NULL, {0} },
    { "about", g3data_window_help_about, NULL, NULL, NULL, {0} },
    { "fullscreen", NULL, NULL, "false", full_screen_change_state, {0} },
    { "zoom-area", NULL, NULL, "true", zoom_area_change_state, {0} },
    { "axis-settings", NULL, NULL, "true", log_buttons_change_state, {0} },
    { "output-properties", NULL, NULL, "true", sort_buttons_change_state, {0} }
};

/* Actions that only make sense once an image is loaded */
static const gchar *image_actions[] = {
    "zoom-area", "axis-settings", "output-properties"
};

static const struct {
    const gchar *action;
    const gchar *accel;
} win_accels[] = {
    { "open", "<Primary>o" },
    { "save-as", "<Primary><Shift>s" },
    { "close", "<Primary>w" },
    { "zoom-area", "F5" },
    { "axis-settings", "F6" },
    { "output-properties", "F7" },
    { "fullscreen", "F11" }
};


/* Append a menu item that shows its keyboard shortcut. */
static void g3data_menu_append (GMenu *menu, const gchar *label,
                                const gchar *action, const gchar *accel)
{
    GMenuItem *item = g_menu_item_new (label, action);

    g_menu_item_set_attribute (item, "accel", "s", accel);
    g_menu_append_item (menu, item);
    g_object_unref (item);
}


static gboolean g3data_action_enabled (GActionGroup *group, const gchar *name)
{
    return g_action_group_has_action (group, name) &&
           g_action_group_get_action_enabled (group, name);
}


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


static void g3data_window_file_open (GSimpleAction *action G_GNUC_UNUSED, GVariant *parameter G_GNUC_UNUSED, gpointer data)
{
    G3dataWindow *window = G3DATA_WINDOW (data);
    GtkWidget *dialog, *scalespinbutton, *hboxextra, *scalelabel;
    GtkImage *preview;
    GtkAdjustment *scaleadj;
    GtkFileFilter *filefilter;
    const gchar scale_string[] = "Scale image : ";

    dialog = gtk_file_chooser_dialog_new ("Open Image",
                                          GTK_WINDOW (window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Open", GTK_RESPONSE_ACCEPT,
                                          NULL);

    /* Set filtering of files to open to filetypes gdk_pixbuf can handle */
    filefilter = gtk_file_filter_new ();
    gtk_file_filter_add_pixbuf_formats (filefilter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog),
                                 GTK_FILE_FILTER (filefilter));

    hboxextra = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

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


static void g3data_window_file_save_as (GSimpleAction *action G_GNUC_UNUSED, GVariant *parameter G_GNUC_UNUSED, gpointer data)
{
    G3dataWindow *window = G3DATA_WINDOW (data);
    GtkWidget *dialog;
    gchar *filename;
    FILE *fp;

    dialog = gtk_file_chooser_dialog_new ("Save As...",
                                          GTK_WINDOW (window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                          "_Save", GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "untitled.txt");

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        fp = fopen (filename, "w");
        if (fp == NULL) {
            gint saved_errno = errno;

            /* Close the file chooser, then tell the user why nothing was saved */
            gtk_widget_destroy (dialog);
            dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE,
                            "Error saving file '%s': %s",
                            filename, g_strerror (saved_errno));
            gtk_dialog_run (GTK_DIALOG (dialog));
            gtk_widget_destroy (dialog);
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


static void g3data_window_file_close (GSimpleAction *action G_GNUC_UNUSED, GVariant *parameter G_GNUC_UNUSED, gpointer data)
{
    gtk_widget_destroy (GTK_WIDGET (data));
}


/* Store the new state of a toggle action and return it. */
static gboolean toggle_set_state (GSimpleAction *action, GVariant *state)
{
    g_simple_action_set_state (action, state);
    return g_variant_get_boolean (state);
}


static void full_screen_change_state (GSimpleAction *action, GVariant *state, gpointer data) {
    G3dataWindow *window = G3DATA_WINDOW (data);

    if (toggle_set_state (action, state)) {
        gtk_window_fullscreen (GTK_WINDOW (window));
    } else {
        gtk_window_unfullscreen (GTK_WINDOW (window));
    }
}


/* Hide or show zoom area. */
static void zoom_area_change_state (GSimpleAction *action, GVariant *state, gpointer data) {
    G3dataWindow *window = G3DATA_WINDOW (data);

    gtk_widget_set_visible (window->zoom_area_vbox, toggle_set_state (action, state));
}


/* Hide or show logarithmic axes buttons. */
static void log_buttons_change_state (GSimpleAction *action, GVariant *state, gpointer data) {
    G3dataWindow *window = G3DATA_WINDOW (data);

    gtk_widget_set_visible (window->log_buttons_vbox, toggle_set_state (action, state));
}


/* Hide or show output sorting buttons. */
static void sort_buttons_change_state (GSimpleAction *action, GVariant *state, gpointer data) {
    G3dataWindow *window = G3DATA_WINDOW (data);

    gtk_widget_set_visible (window->sort_buttons_vbox, toggle_set_state (action, state));
}


static void g3data_window_help_about (GSimpleAction *action G_GNUC_UNUSED, GVariant *parameter G_GNUC_UNUSED, gpointer data)
{
    g3data_about (GTK_WIDGET (data));
}


/* Enable or disable the actions that need a loaded image. */
void g3data_window_set_image_actions_enabled (G3dataWindow *window, gboolean enabled)
{
    guint i;

    for (i = 0; i < G_N_ELEMENTS (image_actions); i++) {
        GAction *action = g_action_map_lookup_action (G_ACTION_MAP (window->actions),
                                                      image_actions[i]);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (action), enabled);
    }
}


/* Keyboard shortcut handler: activate the window action of entry
 * number data in win_accels, if it is currently enabled. */
static gboolean accel_activate (GtkAccelGroup *accel_group G_GNUC_UNUSED, GObject *acceleratable,
                                guint keyval G_GNUC_UNUSED, GdkModifierType modifier G_GNUC_UNUSED, gpointer data)
{
    G3dataWindow *window = G3DATA_WINDOW (acceleratable);
    GActionGroup *group = G_ACTION_GROUP (window->actions);
    const gchar *name = win_accels[GPOINTER_TO_INT (data)].action;

    if (!g3data_action_enabled (group, name))
        return FALSE;
    g_action_group_activate_action (group, name, NULL);
    return TRUE;
}


static GMenuModel *g3data_window_menu_model (void)
{
    GMenu *menubar, *menu, *section;

    menubar = g_menu_new ();

    menu = g_menu_new ();
    section = g_menu_new ();
    g3data_menu_append (section, "_Open", "win.open", "<Primary>o");
    g3data_menu_append (section, "Save _As...", "win.save-as", "<Primary><Shift>s");
    g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
    g_object_unref (section);
    section = g_menu_new ();
    g3data_menu_append (section, "_Close", "win.close", "<Primary>w");
    g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
    g_object_unref (section);
    g_menu_append_submenu (menubar, "_File", G_MENU_MODEL (menu));
    g_object_unref (menu);

    menu = g_menu_new ();
    section = g_menu_new ();
    g3data_menu_append (section, "Zoom area", "win.zoom-area", "F5");
    g3data_menu_append (section, "Axis settings", "win.axis-settings", "F6");
    g3data_menu_append (section, "Output properties", "win.output-properties", "F7");
    g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
    g_object_unref (section);
    section = g_menu_new ();
    g3data_menu_append (section, "_Full Screen", "win.fullscreen", "F11");
    g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
    g_object_unref (section);
    g_menu_append_submenu (menubar, "_View", G_MENU_MODEL (menu));
    g_object_unref (menu);

    menu = g_menu_new ();
    g_menu_append (menu, "_About", "win.about");
    g_menu_append_submenu (menubar, "_Help", G_MENU_MODEL (menu));
    g_object_unref (menu);

    return G_MENU_MODEL (menubar);
}


static void g3data_window_init (G3dataWindow *g3data_window)
{
    int i;
    GtkWidget *menubar;
    GMenuModel *menu_model;
    GtkAccelGroup *accel_group;

    gtk_window_set_default_size (GTK_WINDOW (g3data_window), 640, 480);
    gtk_window_set_title (GTK_WINDOW (g3data_window), "g3data - no file");
    gtk_window_set_resizable (GTK_WINDOW (g3data_window), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (g3data_window), 0);

    g3data_window->main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (g3data_window), g3data_window->main_vbox);

    /* Create menu */
    g3data_window->actions = g_simple_action_group_new ();
    g_action_map_add_action_entries (G_ACTION_MAP (g3data_window->actions),
                                     win_entries, G_N_ELEMENTS (win_entries),
                                     g3data_window);
    gtk_widget_insert_action_group (GTK_WIDGET (g3data_window), "win",
                                    G_ACTION_GROUP (g3data_window->actions));
    g3data_window_set_image_actions_enabled (g3data_window, FALSE);

    accel_group = gtk_accel_group_new ();
    for (i = 0; i < (int) G_N_ELEMENTS (win_accels); i++) {
        guint key;
        GdkModifierType mods;

        gtk_accelerator_parse (win_accels[i].accel, &key, &mods);
        gtk_accel_group_connect (accel_group, key, mods, 0,
                                 g_cclosure_new (G_CALLBACK (accel_activate),
                                                 GINT_TO_POINTER (i), NULL));
    }
    gtk_window_add_accel_group (GTK_WINDOW (g3data_window), accel_group);
    g_object_unref (accel_group);

    menu_model = g3data_window_menu_model ();
    menubar = gtk_menu_bar_new_from_model (menu_model);
    g_object_unref (menu_model);
    gtk_box_pack_start (GTK_BOX (g3data_window->main_vbox), menubar, FALSE, FALSE, 0);

    g3data_window->image = NULL;
    for (i = 0; i < 4; i++) {
        g3data_window->control_point_image_coords[i][0] = -1;
        g3data_window->control_point_image_coords[i][1] = -1;
    }
}


static void g3data_window_class_init (G3dataWindowClass *g3data_window_class G_GNUC_UNUSED)
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
