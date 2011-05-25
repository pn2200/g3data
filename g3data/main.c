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
#include "strings.h"
#include "points.h"

#ifdef NOSPACING
#define SECT_SEP 0
#define GROUP_SEP 0
#define ELEM_SEP 0
#define FRAME_INDENT 0
#define WINDOW_BORDER 0
#else
#define SECT_SEP 12
#define GROUP_SEP 12
#define ELEM_SEP 6
#define FRAME_INDENT 18
#define WINDOW_BORDER 12
#endif

/* Declaration of gtk variables */
GtkWidget	*window;								/* Window */
GtkWidget	*drawing_area;			/* Drawing areas */
GtkWidget	*xyentry[4];
GtkWidget	*setxybutton[4];
GtkWidget	*xc_entry,*yc_entry;
GtkWidget	*nump_entry;
GtkWidget	*xerr_entry,*yerr_entry;			/* Coordinate and filename entries */
GtkWidget       *logbox = {NULL}, *zoomareabox = {NULL}, *oppropbox = {NULL};
GtkWidget	*pm_label, *pm_label2, *file_label;
GtkWidget	*ViewPort = NULL;
GdkColor        *colors;								/* Pointer to colors */
GdkPixbuf       *gpbimage;
GtkWidget *mainvbox;

/* Declaration of global variables */
/* axiscoords[][][0] will be set to -1 when not used */
gint		axiscoords[4][2];						/* X,Y coordinates of axispoints */
gint		**points;							/* Indexes of graphpoints and their coordinates */
gint		numpoints;
gint		ordering;
gint		XSize, YSize;
gint		file_name_length;
gint 		MaxPoints = {MAXPOINTS};
gint		NoteBookNumPages = 0;
gint xpointer = -1;
gint ypointer = -1;
static gint width = -1;
static gint height = -1;
gboolean UseErrors = FALSE;
gboolean logxy[2] = {FALSE, FALSE};
static gdouble scale = -1;
gdouble		realcoords[4];						/* X,Y coords on graph */
gboolean	setxypressed[4];
gboolean	bpressed[4];						/* What axispoints have been set out ? */
gboolean	valueset[4];
gboolean        ShowLog = FALSE, ShowZoomArea = FALSE, ShowOpProp = FALSE;
gchar		*FileNames;
static const gchar **filenames;
FILE		*FP;									/* File pointer */

GtkWidget 	*drawing_area_alignment;

static gint configure_event(GtkWidget *widget, GdkEventConfigure *event,gpointer data);
static void toggle_xy(GtkWidget *widget, gpointer func_data);
static void SetOrdering(GtkWidget *widget, gpointer func_data);
static void UseErrCB(GtkWidget *widget, gpointer func_data);
static void read_xy_entry(GtkWidget *entry, gpointer func_data);
static void islogxy(GtkWidget *widget, gpointer func_data);
static gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer);
static gint InsertImage(char *filename, gdouble Scale, gdouble maxX, gdouble maxY);
static gint SetupNewTab(char *filename, gdouble Scale, gdouble maxX, gdouble maxY, gboolean UsePreSetCoords);
static void drag_data_received(GtkWidget *widget,
                              GdkDragContext *drag_context,
                              gint x, gint y,
                              GtkSelectionData *data,
                              guint info,
                              guint event_time,
                              gpointer user_data);

static const GOptionEntry goption_options[] =
{
	{ "height", 'h', 0, G_OPTION_ARG_INT, &height, "The maximum height of image. Larger images will be scaled to this height.", "H"},
	{ "width", 'w', 0, G_OPTION_ARG_INT, &width, "The maximum width of image. Larger images will be scaled to this width.", "W"},
	{ "scale", 's', 0, G_OPTION_ARG_DOUBLE, &scale, "Scale image by scale factor.", "S"},
	{ "error", 'e', 0, G_OPTION_ARG_NONE, &UseErrors, "Output estimates of error", NULL },
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
/* This function is called when the drawing area is configured	*/
/* for the first time, currently this function does not perform	*/
/* any task.							*/
/****************************************************************/
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event,gpointer data)
{
    return TRUE;
}


/****************************************************************/
/* This function is called when the "Set point 1/2 on x/y axis"	*/
/* button is pressed. It inactivates the other "Set" buttons	*/
/* and makes sure the button stays down even when pressed on.	*/
/****************************************************************/
static void toggle_xy(GtkWidget *widget, gpointer func_data)
{
    gint i, j;

    i = GPOINTER_TO_INT (func_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget))) {
	setxypressed[i] = TRUE;						/* The button is pressed down */
	for (j = 0; j < 4; j++) {
	    if (i != j) gtk_widget_set_sensitive(setxybutton[j],FALSE);
	}
	if (bpressed[i]) {								/* If the x axis point is already set */
        axiscoords[i][0] = -1;
        axiscoords[i][1] = -1;
	}
	bpressed[i]=FALSE;						/* Set x axis point 1 to unset */
    } else {										/* If button is trying to get unpressed */
	if (setxypressed[i]) 
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE); 		/* Set button down */
    }
    gtk_widget_queue_draw(drawing_area);
}


/****************************************************************/
/* Set type of ordering at output of data.			*/
/****************************************************************/
static void SetOrdering(GtkWidget *widget, gpointer func_data)
{
    ordering = GPOINTER_TO_INT (func_data);				/* Set ordering control variable */
}


/****************************************************************/
/* Set whether to use error evaluation and printing or not.	*/
/****************************************************************/
static void UseErrCB(GtkWidget *widget, gpointer func_data)
{
    UseErrors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


/****************************************************************/
/* When the value of the entry of any axis point is changed, 	*/
/* this function gets called.					*/
/****************************************************************/
static void read_xy_entry(GtkWidget *entry, gpointer func_data)
{
    const gchar *xy_text;
    gint i;
    
    i = GPOINTER_TO_INT (func_data);

    xy_text = gtk_entry_get_text(GTK_ENTRY (entry));
    sscanf(xy_text,"%lf",&realcoords[i]);				/* Convert string to double value and */
											/* store in realcoords[0]. */
    if (logxy[i/2] && realcoords[i] > 0) valueset[i]=TRUE;
    else if (logxy[i/2]) valueset[i]=FALSE;
    else valueset[i] = TRUE;

}


/****************************************************************/
/* If the "X/Y axis is logarithmic" check button is toggled	*/
/* this function gets called. It sets the logx variable to its	*/
/* correct value corresponding to the buttons state.		*/
/****************************************************************/
static void islogxy(GtkWidget *widget, gpointer func_data)
{
  gint i;

    i = GPOINTER_TO_INT (func_data);

    logxy[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
											/* logxy = TRUE else FALSE. */
    if (logxy[i]) {
	if (realcoords[i*2] <= 0) {					/* If a negative value has been insert */
	    valueset[i*2]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(xyentry[i*2]),"");		/* Zero it */
	}
	if (realcoords[i*2+1] <= 0) {					/* If a negative value has been insert */
	    valueset[i*2+1]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(xyentry[i*2+1]),"");		/* Zero it */
        }
    }
}


/****************************************************************/
/* This function handles all of the keypresses done within the	*/
/* main window and handles the  appropriate measures.		*/
/****************************************************************/
static gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer)
{
  GtkAdjustment *adjustment;
  gdouble adj_val;
  GdkCursor	*cursor;

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
/* This function loads the image, and inserts it into the tab	*/
/* and sets up all of the different signals associated with it.	*/
/****************************************************************/
static gint InsertImage(char *filename, gdouble Scale, gdouble maxX, gdouble maxY) {
    gboolean has_alpha;
    gint w, h;
    GdkPixbuf *loadgpbimage;
    GdkCursor *cursor;
    GtkWidget *dialog;

    loadgpbimage = gdk_pixbuf_new_from_file(filename,NULL);				/* Load image */
    if (loadgpbimage==NULL) {								/* If unable to load image */
	dialog = gtk_message_dialog_new (GTK_WINDOW(window),				/* Notify user of the error */
                                  GTK_DIALOG_DESTROY_WITH_PARENT,			/* with a dialog */
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Error loading file '%s'",
                                  filename);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (dialog);

	return -1;									/* exit */
    }

    w = gdk_pixbuf_get_width(loadgpbimage);
    h = gdk_pixbuf_get_height(loadgpbimage);
    has_alpha = gdk_pixbuf_get_has_alpha(loadgpbimage);

    if (width != -1 && height != -1 && scale == -1) {
        if (w > width || h > height) {
            scale = fmin((double) (width/w), (double) (height/h));
        }
    }

    if (scale != -1) {
        w = w * scale;
        h = h * scale;
        gpbimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, w, h);
        gdk_pixbuf_composite(loadgpbimage, gpbimage, 0, 0, w, h,
    	         0, 0, scale, scale, GDK_INTERP_BILINEAR, 255);
        g_object_unref(loadgpbimage);
    } else {
        gpbimage = loadgpbimage;
    }

    XSize = w;
    YSize = h;

    drawing_area = gtk_drawing_area_new ();					/* Create new drawing area */
    gtk_widget_set_size_request (drawing_area, XSize, YSize);

    g_signal_connect (G_OBJECT (drawing_area), "configure_event",		/* Connect drawing area to */
              G_CALLBACK (configure_event), NULL);			/* configure_event. */

    gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK |			/* Set the events active */
			   GDK_BUTTON_PRESS_MASK | 
			   GDK_BUTTON_RELEASE_MASK |
			   GDK_POINTER_MOTION_MASK | 
			   GDK_POINTER_MOTION_HINT_MASK);

    gtk_container_add(GTK_CONTAINER(drawing_area_alignment), drawing_area);

    gtk_widget_show(drawing_area);

    cursor = gdk_cursor_new (GDK_CROSSHAIR);
    gdk_window_set_cursor (gtk_widget_get_window(drawing_area), cursor);
 
    return 0;
}


/****************************************************************/
/* This function sets up a new tab, sets up all of the widgets 	*/
/* needed.							*/
/****************************************************************/
static gint SetupNewTab(char *filename, gdouble Scale, gdouble maxX, gdouble maxY, gboolean UsePreSetCoords)
{
  GtkWidget 	*table;									/* GTK table/box variables for packing */
  GtkWidget	*tophbox, *bottomhbox;
  GtkWidget	*trvbox, *tlvbox, *brvbox, *blvbox, *subvbox;
  GtkWidget 	*xy_label[4];								/* Labels for texts in window */
  GtkWidget 	*logcheckb[2];								/* Logarithmic checkbuttons */
  GtkWidget 	*nump_label, *ScrollWindow;						/* Various widgets */
  GtkWidget	*APlabel, *PIlabel, *ZAlabel, *Llabel, *tab_label;
  GtkWidget 	*alignment;
  GtkWidget 	*x_label, *y_label, *tmplabel;
  GtkWidget	*ordercheckb[3], *UseErrCheckB;
  GtkWidget	*Olabel, *Elabel;
  GSList 	*group;
  GtkWidget	*dialog;

    gchar buf[20];
    gchar *buffer;
  gint 		i;

    table = gtk_table_new(1, 2 ,FALSE);							/* Create table */
    gtk_container_set_border_width (GTK_CONTAINER (table), WINDOW_BORDER);
    gtk_table_set_row_spacings(GTK_TABLE(table), SECT_SEP);				/* Set spacings */
    gtk_table_set_col_spacings(GTK_TABLE(table), 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), table, FALSE, FALSE, 0);

/* Init datastructures */
    FileNames = g_strdup_printf("%s", basename(filename));

    for (i = 0; i < 4; i++) {
        axiscoords[i][0] = -1;
        axiscoords[i][1] = -1;
        bpressed[i] = FALSE;
        valueset[i] = FALSE;
    }

    numpoints = 0;
    ordering = 0;

    points = (void *) malloc(sizeof(gint *) * MaxPoints);
    if (points==NULL) {
	printf("Error allocating memory for points. Exiting.\n");
	return -1;
    }
    for (i=0;i<MaxPoints;i++) {
	points[i] = (gint *) malloc(sizeof(gint) * 2);
	if (points[i]==NULL) {
	    printf("Error allocating memory for points[%d]. Exiting.\n",i);
	    return -1;
	}
    }

    for (i=0;i<4;i++) {
    /* buttons for setting axis points x_1, x_2, etc. */
	tmplabel = gtk_label_new(NULL);
	gtk_label_set_markup_with_mnemonic(GTK_LABEL(tmplabel), setxylabel[i]);
	setxybutton[i] = gtk_toggle_button_new();				/* Create button */
	gtk_container_add(GTK_CONTAINER(setxybutton[i]), tmplabel);
	g_signal_connect (G_OBJECT (setxybutton[i]), "toggled",			/* Connect button */
			  G_CALLBACK (toggle_xy), GINT_TO_POINTER (i));
        gtk_widget_set_tooltip_text(setxybutton[i],setxytts[i]);

    /* labels for axis points x_1, x_2, etc. */
	xy_label[i] = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(xy_label[i]), xy_label_text[i]);

    /* text entries to enter axis points x_1, x_2, etc. */
	xyentry[i] = gtk_entry_new();  						/* Create text entry */
	gtk_entry_set_max_length (GTK_ENTRY (xyentry[i]), 20);
	gtk_widget_set_sensitive(xyentry[i],FALSE);				/* Inactivate it */
	g_signal_connect (G_OBJECT (xyentry[i]), "changed",			/* Init the entry to call */
			  G_CALLBACK (read_xy_entry), GINT_TO_POINTER (i));		/* read_x1_entry whenever */
        gtk_widget_set_tooltip_text (xyentry[i],entryxytt[i]);
    }

    /* Processing information labels and text entries */
    x_label = gtk_label_new(x_string);
    y_label = gtk_label_new(y_string);
    xc_entry = gtk_entry_new();							/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (xc_entry), 16);
    gtk_editable_set_editable(GTK_EDITABLE(xc_entry),FALSE);
    yc_entry = gtk_entry_new();							/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (yc_entry), 16);
    gtk_editable_set_editable(GTK_EDITABLE(yc_entry),FALSE);

    /* plus/minus (+/-) symbol labels */
    pm_label = gtk_label_new(pm_string);
    pm_label2 = gtk_label_new(pm_string);
    /* labels and error text entries */
    xerr_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (xerr_entry), 16);
    gtk_editable_set_editable(GTK_EDITABLE(xerr_entry),FALSE);
    yerr_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (yerr_entry), 16);
    gtk_editable_set_editable(GTK_EDITABLE(yerr_entry),FALSE);

    /* Number of points label and entry */
    nump_label = gtk_label_new(nump_string);
    nump_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (nump_entry), 10);
    gtk_editable_set_editable(GTK_EDITABLE(nump_entry),FALSE);
    SetNumPointsEntry(nump_entry, numpoints);

    setcolors(&colors);

    /* Logarithmic axes */
    for (i=0;i<2;i++) {
	logcheckb[i] = gtk_check_button_new_with_mnemonic(loglabel[i]);			/* Create check button */
	g_signal_connect (G_OBJECT (logcheckb[i]), "toggled",				/* Connect button */
			  G_CALLBACK (islogxy), GINT_TO_POINTER (i));
        gtk_widget_set_tooltip_text (logcheckb[i],logxytt[i]);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(logcheckb[i]), logxy[i]);
    }

    tophbox = gtk_hbox_new (FALSE, SECT_SEP);
    alignment = gtk_alignment_new (0,0,0,0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 5, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), tophbox);

    bottomhbox = gtk_hbox_new (FALSE, SECT_SEP);
    alignment = gtk_alignment_new (0, 0, 1, 1);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 1, 2, 5, 5, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), bottomhbox);

    /* Packing the axis points labels and entries */
    tlvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (tophbox), tlvbox, FALSE, FALSE, ELEM_SEP);
    APlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (APlabel), APheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, APlabel);
    gtk_box_pack_start (GTK_BOX (tlvbox), alignment, FALSE, FALSE, 0);
    table = gtk_table_new(3, 4 ,FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_table_set_col_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (tlvbox), table, FALSE, FALSE, 0);
    for (i=0;i<4;i++) {
	    gtk_table_attach_defaults(GTK_TABLE(table), setxybutton[i], 0, 1, i, i+1);
	    gtk_table_attach_defaults(GTK_TABLE(table), xy_label[i], 1, 2, i, i+1);
	    gtk_table_attach_defaults(GTK_TABLE(table), xyentry[i], 2, 3, i, i+1);
    }

    /* Packing the point information boxes */
    trvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (tophbox), trvbox, FALSE, FALSE, ELEM_SEP);

    PIlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (PIlabel), PIheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), PIlabel);
    gtk_box_pack_start (GTK_BOX (trvbox), alignment, FALSE, FALSE, 0);

    table = gtk_table_new(4, 2 ,FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_table_set_col_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (trvbox), table, FALSE, FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), x_label, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), xc_entry, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), pm_label, 2, 3, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), xerr_entry, 3, 4, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), y_label, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), yc_entry, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), pm_label2, 2, 3, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), yerr_entry, 3, 4, 1, 2);

    /* Pack number of points boxes */
    table = gtk_table_new(3, 1 ,FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 6);
    gtk_table_set_col_spacings(GTK_TABLE(table), 6);
    gtk_box_pack_start (GTK_BOX (trvbox), table, FALSE, FALSE, 0);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), nump_label);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), nump_entry, 1, 2, 0, 1, 0, 0, 0, 0);

    blvbox = gtk_vbox_new (FALSE, GROUP_SEP);
    gtk_box_pack_start (GTK_BOX (bottomhbox), blvbox, FALSE, FALSE, ELEM_SEP);

    /* Pack zoom area */
    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    zoomareabox = subvbox;
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    ZAlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (ZAlabel), ZAheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, ZAlabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);

    /* Pack logarithmic axes */
    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    logbox = subvbox;
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    Llabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Llabel), Lheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Llabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<2;i++) {
	gtk_box_pack_start (GTK_BOX (subvbox), logcheckb[i], FALSE, FALSE, 0);			/* Pack checkbutton in vert. box */
    }

    /* Create and pack radio buttons for sorting */
    group = NULL;
    for (i=0;i<ORDERBNUM;i++) {
	ordercheckb[i] = gtk_radio_button_new_with_label (group, orderlabel[i]);	/* Create radio button */
	g_signal_connect (G_OBJECT (ordercheckb[i]), "toggled",				/* Connect button */
			  G_CALLBACK (SetOrdering), GINT_TO_POINTER (i));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (ordercheckb[i]));		/* Get buttons group */
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ordercheckb[0]), TRUE);		/* Set no ordering button active */

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    oppropbox = subvbox;
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    Olabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Olabel), Oheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Olabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<ORDERBNUM;i++) {
	gtk_box_pack_start (GTK_BOX (subvbox), ordercheckb[i], FALSE, FALSE, 0);			/* Pack radiobutton in vert. box */
    }

    /* Create and pack value errors button */
    UseErrCheckB = gtk_check_button_new_with_mnemonic(PrintErrCBLabel);
    g_signal_connect (G_OBJECT (UseErrCheckB), "toggled",
		      G_CALLBACK (UseErrCB), NULL);
    gtk_widget_set_tooltip_text (UseErrCheckB,uetts);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(UseErrCheckB), UseErrors);

    Elabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Elabel), Eheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), Elabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), UseErrCheckB, FALSE, FALSE, 0);

    /* Print current image name in title bar*/
    buffer = g_strdup_printf(Window_Title, filename);
    gtk_window_set_title (GTK_WINDOW (window), buffer);

    brvbox = gtk_vbox_new (FALSE, GROUP_SEP);
    gtk_box_pack_start (GTK_BOX (bottomhbox), brvbox, TRUE, TRUE, 0);

    /* Create a scroll window to hold image */
    ScrollWindow = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(ScrollWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    ViewPort = gtk_viewport_new(NULL,NULL);
    gtk_box_pack_start (GTK_BOX (brvbox), ScrollWindow, TRUE, TRUE, 0);
    drawing_area_alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (ViewPort), drawing_area_alignment);
    gtk_container_add (GTK_CONTAINER (ScrollWindow), ViewPort);

    gtk_widget_show_all(window);

    if (InsertImage(filename, Scale, maxX, maxY) == -1) {
	return -1;
    }

    if (UsePreSetCoords) {
	axiscoords[0][0] = 0;
	axiscoords[0][1] = YSize-1;
	axiscoords[1][0] = XSize-1;
	axiscoords[1][1] = YSize-1;
	axiscoords[2][0] = 0;
	axiscoords[2][1] = YSize-1;
	axiscoords[3][0] = 0;
	axiscoords[3][1] = 0;
	for (i=0;i<4;i++) {
	    gtk_widget_set_sensitive(xyentry[i],TRUE);
	    gtk_editable_set_editable(GTK_EDITABLE(xyentry[i]),TRUE);
        g_ascii_formatd(buf, 20, "%lf", realcoords[i]);
	    gtk_entry_set_text(GTK_ENTRY(xyentry[i]), buf);
	    valueset[i] = TRUE;
	    bpressed[i] = TRUE;
	    setxypressed[i]=FALSE;
	}
    }

    if (ShowZoomArea)
        if (zoomareabox != NULL)
            gtk_widget_show(zoomareabox);
    if (ShowLog)
        if (logbox != NULL)
            gtk_widget_show(logbox);
    if (ShowOpProp)
        if (oppropbox != NULL)
            gtk_widget_show(oppropbox);

  return 0;
}


/****************************************************************/
/****************************************************************/
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
    GError *error;

    if (info == URI_LIST) {
        uri_list = gtk_selection_data_get_uris(data);
        i = 0;
        while (uri_list[i] != NULL) {
            error = NULL;
            filename = g_filename_from_uri(uri_list[i], NULL, &error);
            if (filename == NULL) {
                g_message("Null filename: %s", error->message);
                g_error_free(error);
            } else{
        	    SetupNewTab(filename, 1.0, -1, -1, FALSE);
            }
            i++;
        }
        g_strfreev(uri_list);
    }
    gtk_drag_finish (drag_context, TRUE, FALSE, event_time);
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
