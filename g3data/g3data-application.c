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
#include "g3data-image.h"

typedef enum {
    URI_LIST,
    PNG_DATA,
    JPEG_DATA,
    APP_X_COLOR,
    NUM_IMAGE_DATA,
} UI_DROP_TARGET_INFO;

static const GtkTargetEntry ui_drop_target_entries[] = {
  {"text/uri-list", 0, URI_LIST},
  {"image/png",     0, PNG_DATA},
  {"image/jpeg",    0, JPEG_DATA},
  {"application/x-color",    0, APP_X_COLOR}
};


struct _G3dataApplicationClass {
    GObjectClass base_class;
};

static G3dataApplication *instance;

G_DEFINE_TYPE (G3dataApplication, g3data_application, G_TYPE_OBJECT);


G3dataApplication *g3data_application_get_instance (void)
{
    if (!instance) {
        instance = G3DATA_APPLICATION (g_object_new (G3DATA_TYPE_APPLICATION, NULL));
    }

    return instance;
}


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


static void drag_data_received(GtkWidget *widget,
                              GdkDragContext *drag_context,
                              gint x, gint y,
                              GtkSelectionData *data,
                              guint info,
                              guint event_time,
                              gpointer user_data)
{
    gchar *filename;
    gchar **uri_list;
    gint i;
    struct g3data_options *options = g_malloc0 (sizeof (struct g3data_options));
    GError *error;

    g3data_set_default_options (options);

    if (info == URI_LIST) {
        uri_list = gtk_selection_data_get_uris (data);
        i = 0;
        while (uri_list[i] != NULL) {
            error = NULL;
            filename = g_filename_from_uri (uri_list[i], NULL, &error);
            if (filename == NULL) {
                g_message ("Null filename: %s", error->message);
                g_error_free (error);
            } else {
                if (instance->current_window == NULL || instance->current_window->image != NULL) {
                    g3data_create_window (instance);
                }
                g3data_window_insert_image (instance->current_window, filename, options);
            }
            i++;
        }
        g_strfreev (uri_list);
    }
    gtk_drag_finish (drag_context, TRUE, FALSE, event_time);
}


void g3data_create_window (G3dataApplication *application)
{
    G3dataWindow *window;

    window = g3data_window_new ();
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (g3data_window_destroy_cb), application);

    application->windows = g_slist_prepend (application->windows, window);
    application->current_window = G3DATA_WINDOW (window);

    gtk_drag_dest_set (GTK_WIDGET(window), GTK_DEST_DEFAULT_ALL, ui_drop_target_entries, NUM_IMAGE_DATA, (GDK_ACTION_COPY | GDK_ACTION_MOVE));
    g_signal_connect (G_OBJECT (window), "drag-data-received",
                        G_CALLBACK (drag_data_received), (gpointer) application);

    gtk_widget_show_all (GTK_WIDGET (window));
}


void load_files (const gchar **files, struct g3data_options *options)
{
    int i;

    if (instance == NULL) {
        instance = G3DATA_APPLICATION (g_object_new (G3DATA_TYPE_APPLICATION, NULL));
    }

    if (files != NULL) {
        for (i = 0; files[i]; i++) {
            const gchar *filename;

            filename = files[i];

            g3data_create_window (instance);
            g3data_window_insert_image (instance->current_window, filename, options);
        }
    } else {
        g3data_create_window (instance);
    }

    gtk_main();
}
