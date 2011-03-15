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

G_DEFINE_TYPE (G3dataWindow, g3data_window, GTK_TYPE_WINDOW);

static void g3data_window_file_open (GtkAction *action, gpointer data);
static void g3data_window_file_save_as (GtkAction *action, gpointer data);
static void g3data_window_file_close (GtkAction *action, G3dataWindow *window);
static void g3data_window_help_about (GtkAction *action, gpointer data);

static const GtkActionEntry entries[] = {
    { "FileMenu", NULL, "_File", NULL, NULL, NULL },
    { "ViewMenu", NULL, "_View", NULL, NULL, NULL },
    { "HelpMenu", NULL, "_Help", NULL, NULL, NULL },
    { "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open an image in a new tab", G_CALLBACK( g3data_window_file_open ) },
    { "Save As", GTK_STOCK_SAVE_AS, "Save _As...", "<control><shift>S", "Save data", G_CALLBACK( g3data_window_file_save_as ) },
    { "Close", GTK_STOCK_CLOSE, "_Close", "<control>W", "Close window", G_CALLBACK( g3data_window_file_close ) },
    { "About", GTK_STOCK_HELP, "_About", "", "About g3data", G_CALLBACK( g3data_window_help_about ) }
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
    "    <menu action='HelpMenu'>"
    "      <menuitem action='About'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";


static void g3data_window_file_open (GtkAction *action, gpointer data)
{
}


static void g3data_window_file_save_as (GtkAction *action, gpointer data)
{
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


static void g3data_window_help_about (GtkAction *action, gpointer data)
{
}


static void g3data_window_init (G3dataWindow *g3data_window)
{
    GtkWidget *mainvbox, *menubar;
    GtkActionGroup *action_group;
    GtkUIManager *ui_manager;
    GtkAccelGroup *accel_group;
    GError *error = NULL;

    gtk_window_set_default_size (GTK_WINDOW (g3data_window), 640, 480);
    gtk_window_set_title (GTK_WINDOW (g3data_window), "g3data - no file");
    gtk_window_set_resizable (GTK_WINDOW (g3data_window), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (g3data_window), 0);

    mainvbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (g3data_window), mainvbox);

    /* Create menu */
    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), g3data_window);

    ui_manager = gtk_ui_manager_new();
    gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);

    accel_group = gtk_ui_manager_get_accel_group (ui_manager);
    gtk_window_add_accel_group (GTK_WINDOW (g3data_window), accel_group);
 
    error = NULL;
    if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
        g_message("building menus failed: %s", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
     }
 
    menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (mainvbox), menubar, FALSE, FALSE, 0);
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
