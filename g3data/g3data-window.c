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


static gboolean g3data_window_delete_event (GtkWidget *widget,
                                            GdkEventAny *event)
{
    gtk_widget_destroy (GTK_WIDGET (widget));
    return FALSE;
}


static void g3data_window_init (G3dataWindow *g3data_window)
{
    gtk_window_set_default_size (GTK_WINDOW (g3data_window), 640, 480);
    gtk_window_set_title (GTK_WINDOW (g3data_window), "g3data - no file");
    gtk_window_set_resizable (GTK_WINDOW (g3data_window), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (g3data_window), 0);
}


static void g3data_window_class_init (G3dataWindowClass *g3data_window_class)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (g3data_window_class);

    widget_class->delete_event = g3data_window_delete_event;
}


G3dataWindow *g3data_window_new (void)
{
    G3dataWindow *g3data_window;

    g3data_window = g_object_new (G3DATA_TYPE_WINDOW,
                                  "type", GTK_WINDOW_TOPLEVEL,
                                  NULL);

    return g3data_window;
}
