/*

g3data : A program for grabbing data from scanned graphs
Copyright (C) 2000 Jonas Frantz

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


Authors email : jonas.frantz@welho.com

*/

#include <gtk/gtk.h>									/* Include gtk library */
#include <stdio.h>									/* Include stdio library */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>									/* Include stdlib library */
#include <string.h>									/* Include string library */
#include <math.h>									/* Include math library */
#include <libgen.h>
#include "main.h"
#include "g3data-application.h"
#include "drawing.h"
#include "points.h"

gboolean use_error = FALSE;
gboolean logxy[2] = {FALSE, FALSE};
static gdouble scale = -1;
gdouble		realcoords[4];						/* X,Y coords on graph */
static const gchar **filenames;


static gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer);

static const GOptionEntry goption_options[] =
{
	{ "height", 'h', 0, G_OPTION_ARG_INT, &height, "The maximum height of image. Larger images will be scaled to this height.", "H"},
	{ "width", 'w', 0, G_OPTION_ARG_INT, &width, "The maximum width of image. Larger images will be scaled to this width.", "W"},
	{ "scale", 's', 0, G_OPTION_ARG_DOUBLE, &scale, "Scale image by scale factor.", "S"},
	{ "error", 'e', 0, G_OPTION_ARG_NONE, &use_error, "Output estimates of error", NULL },
	{ "lnx", 0, 0, G_OPTION_ARG_NONE, &logxy[0], "Use logarithmic scale for x coordinates", NULL },
	{ "lny", 0, 0, G_OPTION_ARG_NONE, &logxy[1], "Use logarithmic scale for y coordinates", NULL},
	{ "x0", 0, 0, G_OPTION_ARG_DOUBLE, &realcoords[0], "Preset the x-coordinate for the lower left corner", "x0" },
	{ "x1", 0, 0, G_OPTION_ARG_DOUBLE, &realcoords[1], "Preset the x-coordinate for the upper right corner", "x1" },
	{ "y0", 0, 0, G_OPTION_ARG_DOUBLE, &realcoords[2], "Preset the y-coordinate for the lower left corner", "y0" },
	{ "y1", 0, 0, G_OPTION_ARG_DOUBLE, &realcoords[3], "Preset the y-coordinate for the upper right corner", "y1" },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &filenames, NULL, "[FILE...]" },
	{ NULL }
};


/****************************************************************/
/* This function handles all of the keypresses done within the	*/
/* main window and handles the  appropriate measures.		*/
/****************************************************************/
static gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer)
{
  GtkAdjustment *adjustment;
  gdouble adj_val;

    if (ViewPort != NULL) {

    if (event->keyval==GDK_Left) {
	adjustment = gtk_viewport_get_hadjustment(GTK_VIEWPORT(ViewPort));
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val -= gtk_adjustment_get_page_size(adjustment)/10.0;
	if (adj_val < gtk_adjustment_get_lower(adjustment)) adj_val = gtk_adjustment_get_lower(adjustment);
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_hadjustment(GTK_VIEWPORT(ViewPort), adjustment);
    } else if (event->keyval==GDK_Right) {
	adjustment = gtk_viewport_get_hadjustment(GTK_VIEWPORT(ViewPort));
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val += gtk_adjustment_get_page_size(adjustment)/10.0;
	if (adj_val > (gtk_adjustment_get_upper(adjustment)-gtk_adjustment_get_page_size(adjustment))) adj_val = (gtk_adjustment_get_upper(adjustment)-gtk_adjustment_get_page_size(adjustment));
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_hadjustment(GTK_VIEWPORT(ViewPort), adjustment);
    } else if (event->keyval==GDK_Up) {
	adjustment = gtk_viewport_get_vadjustment(GTK_VIEWPORT(ViewPort));
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val -= gtk_adjustment_get_page_size(adjustment)/10.0;
	if (adj_val < gtk_adjustment_get_lower(adjustment)) adj_val = gtk_adjustment_get_lower(adjustment);
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_vadjustment(GTK_VIEWPORT(ViewPort), adjustment);
    } else if (event->keyval==GDK_Down) {
	adjustment = gtk_viewport_get_vadjustment(GTK_VIEWPORT(ViewPort));
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val += gtk_adjustment_get_page_size(adjustment)/10.0;
	if (adj_val > (gtk_adjustment_get_upper(adjustment)-gtk_adjustment_get_page_size(adjustment))) adj_val = (gtk_adjustment_get_upper(adjustment)-gtk_adjustment_get_page_size(adjustment));
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_vadjustment(GTK_VIEWPORT(ViewPort), adjustment);
    }
    }

  return 0;
}


/****************************************************************/
/* This is the main function, this function gets called when	*/
/* the program is executed. It allocates the necessary work-	*/
/* spaces and initialized the main window and its widgets.	*/
/****************************************************************/
int main (int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    gtk_init (&argc, &argv);

    context = g_option_context_new ("- grab graph data");
    g_option_context_add_main_entries (context, goption_options, NULL);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
        g_print ("option parsing failed: %s\n", error->message);
        g_error_free (error);
        g_option_context_free (context);
        exit (EXIT_FAILURE);
    }
    g_option_context_free (context);

    g_set_application_name ("Grab graph data");
    gtk_window_set_default_icon_name ("g3data-icon");

    load_files (filenames);

    return (EXIT_SUCCESS);
}
