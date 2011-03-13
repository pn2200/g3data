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

#include "g3data-application.h"
#include "g3data-window.h"

struct _G3dataApplicationClass {
    GObjectClass base_class;
};

static G3dataApplication *instance;

G_DEFINE_TYPE (G3dataApplication, g3data_application, G_TYPE_OBJECT);


static void g3data_application_finalize (GObject *object)
{
    G3dataApplication *application = G3DATA_APPLICATION (object);

    application->current_window = NULL;

    g_slist_foreach (application->windows,
                     (GFunc) gtk_widget_destroy,
                     NULL);
    g_slist_free (application->windows);

    G_OBJECT_CLASS (g3data_application_parent_class)->finalize (object);
}


static void g3data_application_class_init (G3dataApplicationClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = g3data_application_finalize;
}


static void g3data_application_init (G3dataApplication *application)
{
    application->windows = NULL;
    application->current_window = NULL;
}


static void g3data_window_destroy_cb (GtkWidget *widget,
                                      gpointer user_data)
{
    G3dataWindow *window = G3DATA_WINDOW (widget);
    G3dataApplication *application = G3DATA_APPLICATION (user_data);

    g_assert (G3DATA_IS_APPLICATION (application));

    application->windows = g_slist_remove (application->windows, window);

    if (window == application->current_window)
        application->current_window = application->windows ? application->windows->data : NULL;

    if (application->windows == NULL)
        gtk_main_quit ();  
}


static void g3data_create_window (G3dataApplication *application)
{
    G3dataWindow *window;

    window = g3data_window_new ();
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (g3data_window_destroy_cb), application);

    application->windows = g_slist_prepend (application->windows, window);
    application->current_window = G3DATA_WINDOW (window);

    gtk_widget_show (GTK_WIDGET (window));
}


void load_files (const char **files)
{
    int i;

    if (instance == NULL) {
        instance = G3DATA_APPLICATION (g_object_new (G3DATA_TYPE_APPLICATION, NULL));
    }

    if (files != NULL) {
        for (i = 0; files[i]; i++) {
            const gchar *filename;
            GFile       *file;

            filename = files[i];

            file = g_file_new_for_commandline_arg (filename);

            g3data_create_window (instance);
            g3data_window_insert_image (instance->current_window, file);
        }
    } else {
        g3data_create_window (instance);
    }

    gtk_main();
}
