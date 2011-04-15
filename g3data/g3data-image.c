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

#include "g3data-window.h"
#include "g3data-image.h"

static GtkWidget *g3data_window_control_points_add (void);

static const gchar control_point_header_text[] = "<b>Axis points</b>";

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


void g3data_window_insert_image (G3dataWindow *window, const gchar *filename)
{
    GtkWidget *table, *tophbox, *alignment;
    GtkWidget *control_point_vbox;

    table = gtk_table_new (1, 2, FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (table), 0);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_box_pack_start (GTK_BOX (window->main_vbox), table, FALSE, FALSE, 0);

    tophbox = gtk_hbox_new (FALSE, 0);
    alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), tophbox);

    control_point_vbox = g3data_window_control_points_add ();
    gtk_box_pack_start (GTK_BOX (tophbox), control_point_vbox, FALSE, FALSE, 0);
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

