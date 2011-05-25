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

#ifndef G3DATA_WINDOW_H
#define G3DATA_WINDOW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _G3dataWindow G3dataWindow;
typedef struct _G3dataWindowClass G3dataWindowClass;
typedef struct _G3dataWindowPrivate G3dataWindowPrivate;

#define G3DATA_TYPE_WINDOW              (g3data_window_get_type())
#define G3DATA_WINDOW(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), G3DATA_TYPE_WINDOW, G3dataWindow))
#define G3DATA_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), G3DATA_TYPE_WINDOW, G3dataWindowClass))
#define G3DATA_IS_WINDOW(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), G3DATA_TYPE_WINDOW))
#define G3DATA_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), G3DATA_TYPE_WINDOW))
#define G3DATA_WINDOW_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), G3DATA_TYPE_WINDOW, G3dataWindowClass))

struct _G3dataWindow {
    GtkWindow base_instance;
    GtkWidget *main_vbox;
    GtkWidget *control_point_button[4], *control_point_entry[4];
    GtkWidget *nump_entry;
    GtkWidget *xc_entry, *yc_entry, *xerr_entry, *yerr_entry;
    GtkWidget *remove_last_button, *remove_all_button;
    GtkWidget *zoom_area, *zoom_area_vbox;
    GtkWidget *log_buttons_vbox;
    GtkWidget *sort_buttons_vbox;
    GtkWidget *drawing_area;
    GtkActionGroup *action_group;
    GdkPixbuf *image;
    gint x, y, size, numpoints;
    gint **points;
    gint control_point_image_coords[4][2];
    gdouble control_point_coords[4];
};

struct _G3dataWindowClass {
    GtkWindowClass base_class;
};

GType g3data_window_get_type (void) G_GNUC_CONST;
G3dataWindow *g3data_window_new (void);

G_END_DECLS

#endif /* !G3DATA_WINDOW_H */

