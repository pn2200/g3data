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

#define ZOOMPIXSIZE 200

static GtkWidget *g3data_window_control_points_add (void);
static GtkWidget *g3data_window_status_area_add (void);
static GtkWidget *g3data_window_zoom_area_add (void);

static const gchar control_point_header_text[] = "<b>Axis points</b>";
static const gchar status_area_header[] = "<b>Processing information</b>";
static const gchar zoom_area_header[] = "<b>Zoom area</b>";

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

static const gchar x_string[] = " X : ";
static const gchar y_string[] = " Y : ";
static const gchar pm_string[] = " ± ";
static const gchar nump_string[] = "Number of points : ";


void g3data_window_insert_image (G3dataWindow *window, const gchar *filename)
{
    GtkWidget *table, *tophbox, *bottomhbox, *bottomvbox, *alignment,
              *scrolled_window, *viewport, *drawing_area_alignment;
    GtkWidget *control_point_vbox, *status_area_vbox, *zoom_area_vbox;

    table = gtk_table_new (2, 2, FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (table), 0);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_box_pack_start (GTK_BOX (window->main_vbox), table, FALSE, FALSE, 0);

    tophbox = gtk_hbox_new (FALSE, 0);
    alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), tophbox);

    bottomhbox = gtk_hbox_new (FALSE, 0);
    alignment = gtk_alignment_new (0, 0, 1, 1);
    gtk_table_attach (GTK_TABLE (table), alignment, 0, 1, 1, 2, 5, 5, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), bottomhbox);

    bottomvbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (bottomhbox), bottomvbox, FALSE, FALSE, 0);

    control_point_vbox = g3data_window_control_points_add ();
    gtk_box_pack_start (GTK_BOX (tophbox), control_point_vbox, FALSE, FALSE, 0);

    status_area_vbox = g3data_window_status_area_add ();
    gtk_box_pack_start (GTK_BOX (tophbox), status_area_vbox, FALSE, FALSE, 0);

    zoom_area_vbox = g3data_window_zoom_area_add ();
    gtk_box_pack_start (GTK_BOX (bottomvbox), zoom_area_vbox, FALSE, FALSE, 0);

    /* Create a scrolled window to hold image */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    viewport = gtk_viewport_new (NULL, NULL);
    gtk_box_pack_start (GTK_BOX (bottomhbox), scrolled_window, TRUE, TRUE, 0);
    drawing_area_alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (viewport), drawing_area_alignment);
    gtk_container_add (GTK_CONTAINER (scrolled_window), viewport);

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


/* Add status area. */
static GtkWidget *g3data_window_status_area_add (void) {
    GtkWidget *x_label, *y_label, *xc_entry, *yc_entry;
    GtkWidget *pm_label, *pm_label2;
    GtkWidget *xerr_entry, *yerr_entry, *nump_label, *nump_entry;
    GtkWidget *status_area_label;
    GtkWidget *vbox, *alignment, *table;

    x_label = gtk_label_new (x_string);
    y_label = gtk_label_new (y_string);
    xc_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (xc_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (xc_entry), FALSE);
    yc_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (yc_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (yc_entry), FALSE);

    /* plus/minus (+/-) symbol labels */
    pm_label = gtk_label_new (pm_string);
    pm_label2 = gtk_label_new (pm_string);
    /* labels and error text entries */
    xerr_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (xerr_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (xerr_entry), FALSE);
    yerr_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (yerr_entry), 16);
    gtk_editable_set_editable (GTK_EDITABLE (yerr_entry), FALSE);

    /* Number of points label and entry */
    nump_label = gtk_label_new (nump_string);
    nump_entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (nump_entry), 10);
    gtk_editable_set_editable (GTK_EDITABLE (nump_entry),FALSE);
    gtk_entry_set_text (GTK_ENTRY (nump_entry), "0");

    /* Packing the status area */
    vbox = gtk_vbox_new (FALSE, 0);

    status_area_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (status_area_label), status_area_header);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), status_area_label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

    table = gtk_table_new (4, 2 ,FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), x_label, 0, 1, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), xc_entry, 1, 2, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), pm_label, 2, 3, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), xerr_entry, 3, 4, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (table), y_label, 0, 1, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE (table), yc_entry, 1, 2, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE (table), pm_label2, 2, 3, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE (table), yerr_entry, 3, 4, 1, 2);

    /* Pack number of points boxes */
    table = gtk_table_new (3, 1 ,FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), nump_label);
    gtk_table_attach (GTK_TABLE (table), alignment, 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach (GTK_TABLE (table), nump_entry, 1, 2, 0, 1, 0, 0, 0, 0);

    return vbox;
}


/* Add zoom area */
static GtkWidget *g3data_window_zoom_area_add (void) {
    GtkWidget *zoom_area;
    GtkWidget *vbox, *label, *alignment;

    vbox = gtk_vbox_new (FALSE, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), zoom_area_header);

    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), label);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

    zoom_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (zoom_area, ZOOMPIXSIZE, ZOOMPIXSIZE);
    gtk_box_pack_start (GTK_BOX (vbox), zoom_area, FALSE, FALSE, 0);

    return vbox;
}
