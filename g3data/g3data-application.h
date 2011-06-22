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

#ifndef G3DATA_APPLICATION_H
#define G3DATA_APPLICATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include "g3data-window.h"

G_BEGIN_DECLS

typedef struct _G3dataApplication G3dataApplication;
typedef struct _G3dataApplicationClass G3dataApplicationClass;

struct _G3dataApplication {
    GObject base_instance;

    G3dataWindow *current_window;
    GSList *windows;
};

#define G3DATA_TYPE_APPLICATION                 (g3data_application_get_type ())
#define G3DATA_APPLICATION(object)              (G_TYPE_CHECK_INSTANCE_CAST((object), G3DATA_TYPE_APPLICATION, G3dataApplication))
#define G3DATA_APPLICATION_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST((klass), G3DATA_TYPE_APPLICATION, G3dataApplicationClass))
#define G3DATA_IS_APPLICATION(object)           (G_TYPE_CHECK_INSTANCE_TYPE((object), G3DATA_TYPE_APPLICATION))
#define G3DATA_IS_APPLICATION_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE((klass), G3DATA_TYPE_APPLICATION))
#define G3DATA_APPLICATION_GET_CLASS(object)    (G_TYPE_INSTANCE_GET_CLASS((object), G3DATA_TYPE_APPLICATION, G3dataApplicationClass))

#define G3DATA_APP					(g3data_application_get_instance ())

GType g3data_application_get_type (void) G_GNUC_CONST;
G3dataApplication *g3data_application_get_instance (void);
void load_files (const char **files, struct g3data_options *options);
void g3data_create_window (G3dataApplication *application);

G_END_DECLS

#endif /* !G3DATA_APPLICATION_H */

