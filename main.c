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


Authors email : jonas.frantz@helsinki.fi

*/

#include <gtk/gtk.h>							/* Include gtk library */
#include <stdio.h>							/* Include stdio library */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>							/* Include stdlib library */
#include <string.h>
#include <math.h>							/* Include math library */
#include "main.h"							/* Include predined variables */
#include "strings.h"							/* Include strings */

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
GtkWidget	*window, *drawing_area, *zoom_area;			/* Windows and drawing areas */
GtkWidget	*xyentry[4];
GtkWidget	*quitbutton, *printbutton, *exportbutton, *remlastbutton; /* Various buttons */
GtkWidget	*setxybutton[4];
GtkWidget	*remallbutton, *fileoutputb;				/* Even more various buttons */
GtkWidget	*xc_entry,*yc_entry,*file_entry, *nump_entry;		/* Coordinate and filename entries */
GtkWidget	*xerr_entry,*yerr_entry;				/* Coordinate and filename entries */
GtkWidget	*pm_label, *pm_label2, *file_label;
GtkWidget	*ViewPort;
GdkColor        *colors;						/* Pointer to colors */
GdkPixbuf       *gpbimage;

#ifdef GTK20X
GdkPixmap       *pixmap;
#endif

/* Declaration of global variables */
gint		axiscoords[4][2];					/* X,Y coordinates of axispoints */
gint		**points;						/* Indexes of graphpoints and their coordinates */
gint		*lastpoints;						/* Indexes of last points put out */
gint		numpoints = 0, numlastpoints = 0;			/* Number of points on graph and last put out */
gint		remthis = 0, ordering = 0;				/* Various control variables */
gint		XSize, YSize;
gint		file_name_length;
gint 		MaxPoints = MAXPOINTS;
gint		Action;
gdouble		realcoords[4];						/* X,Y coords on graph */
gboolean	UseErrors, WinFullScreen;
gboolean 	UseScrolling, UsePreSetCoords;
gboolean	setxypressed[4];
gboolean	bpressed[4] = {FALSE,FALSE,FALSE,FALSE};		/* What axispoints have been set out ? */
gboolean	valueset[4] = {FALSE,FALSE,FALSE,FALSE};
gboolean	logxy[2] = {FALSE, FALSE};
gchar 		*file_name;						/* Pointer to filename */
FILE		*FP;							/* File pointer */

/* Declaration of extern functions */

extern	void SetNumPointsEntry(GtkWidget *np_entry, gint np);
extern	gint min(gint x, gint y);
extern	void DrawMarker(GtkWidget *da, gint x, gint y, gint type, GdkColor *color);
extern	struct PointValue CalcPointValue(gint Xpos, gint Ypos);
extern	void print_results(GtkWidget *widget, gpointer func_data);
extern	gboolean setcolors(GdkColor **color);

/* Explicit declaration of functions */

void remove_last(GtkWidget *widget, gpointer data);
void SetOrdering(GtkWidget *widget, gpointer func_data);
void SetAction(GtkWidget *widget, gpointer func_data);
void UseErrCB(GtkWidget *widget, gpointer func_data);
void read_file_entry(GtkWidget *entry, GtkWidget *widget);

/****************************************************************/
/* This function closes the window when the application is 	*/
/* killed.							*/
/****************************************************************/
gint close_application(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_main_quit();							/* Quit gtk */
    return FALSE;
}


/****************************************************************/
/* When a button is pressed inside the drawing area this 	*/
/* function is called, it handles axispoints and graphpoints	*/
/* and paints a square in that position.			*/
/****************************************************************/
gint button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GdkModifierType state;
  gint x, y, i, j;

    gdk_window_get_pointer (event->window, &x, &y, &state); 		/* Get pointer state */

    if (event->button == 1) {						/* If button 1 (leftmost) is pressed */

/* If none of the set axispoint buttons been pressed */
	if (!setxypressed[0] && !setxypressed[1] && !setxypressed[2] && !setxypressed[3]) {
	    if (numpoints>MaxPoints-1) {
		i = MaxPoints;
		MaxPoints += MAXPOINTS;
		lastpoints = realloc(lastpoints,sizeof(gint) * (MaxPoints+4));
		if (lastpoints==NULL) {
		    printf("Error reallocating memory for lastpoints. Exiting.\n");
		    exit(-1);
		}
		points = realloc(points,sizeof(gint *) * MaxPoints);
		if (points==NULL) {
		    printf("Error reallocating memory for points. Exiting.\n");
		    exit(-1);
		}
		for (;i<MaxPoints;i++) {
		    points[i] = malloc(sizeof(gint) * 2);
		    if (points[i]==NULL) {
			printf("Error allocating memory for points[%d]. Exiting.\n",i);
			exit(-1);
		    }
		}
	    }
	    points[numpoints][0]=x;					/* Save x coordinate */
	    points[numpoints][1]=y;					/* Save x coordinate */
	    lastpoints[numlastpoints]=numpoints;			/* Save index of point */
	    numlastpoints++;						/* Increase lastpoint index */
	    numpoints++;						/* Increase point counter */
	    SetNumPointsEntry(nump_entry, numpoints);
	    gtk_widget_set_sensitive(remlastbutton,TRUE);		/* Activate "Remove last" button */
	    gtk_widget_set_sensitive(remallbutton,TRUE);		/* Activate "Remove all" button */

	    DrawMarker(drawing_area, x, y, 2, colors);
	} else {
	    for (i=0;i<4;i++) if (setxypressed[i]) {			/* If the "Set point 1 on x axis" button is pressed */
		axiscoords[i][0]=x;					/* Save coordinates */
		axiscoords[i][1]=y;
		for (j=0;j<4;j++) if (i!=j) gtk_widget_set_sensitive(setxybutton[j],TRUE);
		gtk_widget_set_sensitive(xyentry[i],TRUE);		/* Sensitize the entry */
		gtk_editable_set_editable((GtkEditable *) xyentry[i],TRUE);
		gtk_widget_grab_focus(xyentry[i]);			/* Focus on entry */
		setxypressed[i]=FALSE;					/* Mark the button as not pressed */
		bpressed[i]=TRUE;					/* Mark that axis point's been set */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setxybutton[i]),FALSE); /* Pop up the button */
		lastpoints[numlastpoints]=-(i+1);			/* Remember that the points been put out */
		numlastpoints++;					/* Increase index of lastpoints */
		gtk_widget_set_sensitive(remlastbutton,TRUE);		/* Activate "Remove last" button */
		gtk_widget_set_sensitive(remallbutton,TRUE);		/* Activate "Remove all" button */

		DrawMarker(drawing_area, x, y, i/2, colors);					/* Draw marker */
	    }
	}

	if (bpressed[0] && bpressed[1] && bpressed[2] && bpressed[3] && valueset[0] && valueset[1] && valueset[2] &&
	    valueset[3] && numpoints > 0) {
	    if (GTK_IS_WIDGET(printbutton)) gtk_widget_set_sensitive(printbutton,TRUE);	
	    gtk_widget_set_sensitive(exportbutton,TRUE);	
	    if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE);
	}
    } else if (event->button == 2) {					/* Is the middle button pressed ? */
	for (i=0;i<2;i++) if (!bpressed[i]) {
	    axiscoords[i][0]=x;
	    axiscoords[i][1]=y;
	    for (j=0;j<4;j++) if (i!=j) gtk_widget_set_sensitive(setxybutton[j],TRUE);
	    gtk_widget_set_sensitive(xyentry[i],TRUE);
	    gtk_editable_set_editable((GtkEditable *) xyentry[i],TRUE);
	    gtk_widget_grab_focus(xyentry[i]);
	    setxypressed[i]=FALSE;
	    bpressed[i]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setxybutton[i]),FALSE);
	    lastpoints[numlastpoints]=-(i+1);
	    numlastpoints++;
	    gtk_widget_set_sensitive(remlastbutton,TRUE);
	    gtk_widget_set_sensitive(remallbutton,TRUE);
	    DrawMarker(drawing_area, x, y, 0, colors);
	    break;
	}
    } else if (event->button == 3) {					/* Is the right button pressed ? */
	for (i=2;i<4;i++) if (!bpressed[i]) {
	    axiscoords[i][0]=x;
	    axiscoords[i][1]=y;
	    for (j=0;j<4;j++) if (i!=j) gtk_widget_set_sensitive(setxybutton[j],TRUE);
	    gtk_widget_set_sensitive(xyentry[i],TRUE);
	    gtk_editable_set_editable((GtkEditable *) xyentry[i],TRUE);
	    gtk_widget_grab_focus(xyentry[i]);
	    setxypressed[i]=FALSE;
	    bpressed[i]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setxybutton[i]),FALSE);
	    lastpoints[numlastpoints]=-(i+1);
	    numlastpoints++;
	    gtk_widget_set_sensitive(remlastbutton,TRUE);
	    gtk_widget_set_sensitive(remallbutton,TRUE);
	    DrawMarker(drawing_area, x, y, 1, colors);
	    break;
	}
    }
    return TRUE;
}


/****************************************************************/
/* This function is called when a button is released on the	*/
/* drawing area, currently this function does not perform any	*/
/* task.							*/
/****************************************************************/
gint button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (event->button == 1) {
    }
    else if (event->button == 2) {
    }
    else if (event->button == 3) {
    }
    return TRUE;
}


/****************************************************************/
/****************************************************************/
void export_values(GtkWidget *widget, gpointer data)
{
  static GtkWidget *dialog;
  GtkWidget	*subvbox, *subhbox;
  GtkWidget	*closebutton, *ordercheckb[3], *UseErrCheckB, *actioncheckb[2];
  GtkWidget	*Olabel, *Elabel, *Alabel;
  GtkWidget	*alignment, *fixed;
  GtkTooltips	*tooltip;
  GSList 	*group;
  gint 		i;

    if (GTK_IS_WIDGET(dialog)) {
	gtk_window_present (GTK_WINDOW(dialog));
	return;
    }

    dialog = gtk_dialog_new();
    gtk_container_set_border_width (GTK_CONTAINER (dialog), WINDOW_BORDER);
    gtk_box_set_spacing((GtkBox *) GTK_DIALOG(dialog)->vbox, GROUP_SEP);
    gtk_window_set_title (GTK_WINDOW (dialog), Dialog_Title);				/* Set window title */

    tooltip = gtk_tooltips_new();

    group = NULL;
    for (i=0;i<ORDERBNUM;i++) {
	ordercheckb[i] = gtk_radio_button_new_with_label (group, orderlabel[i]);	/* Create radio button */
	g_signal_connect (G_OBJECT (ordercheckb[i]), "toggled",				/* Connect button */
			  G_CALLBACK (SetOrdering), GINT_TO_POINTER (i));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (ordercheckb[i]));		/* Get buttons group */
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ordercheckb[0]), TRUE);	/* Set no ordering button active */

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), subvbox);
    Olabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Olabel), Oheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Olabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<ORDERBNUM;i++) {
	fixed = gtk_fixed_new ();
	gtk_fixed_put((GtkFixed *) fixed, ordercheckb[i], FRAME_INDENT, 0);
	gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);		/* Pack radiobutton in vert. box */
    }

    UseErrCheckB = gtk_check_button_new_with_mnemonic(PrintErrCBLabel);
    g_signal_connect (G_OBJECT (UseErrCheckB), "toggled",
		      G_CALLBACK (UseErrCB), NULL);
    gtk_tooltips_set_tip (tooltip,UseErrCheckB,uetts,uett);
    gtk_toggle_button_set_active (( GtkToggleButton *) UseErrCheckB, UseErrors);

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), subvbox);
    Elabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Elabel), Eheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Elabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, UseErrCheckB, FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);

    group = NULL;
    for (i=0;i<ACTIONBNUM;i++) {
	actioncheckb[i] = gtk_radio_button_new_with_label (group, actionlabel[i]);	/* Create radio button */
	g_signal_connect (G_OBJECT (actioncheckb[i]), "toggled",			/* Connect button */
			  G_CALLBACK (SetAction), GINT_TO_POINTER (i));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (actioncheckb[i]));	/* Get buttons group */
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (actioncheckb[0]), TRUE);		/* Set no ordering button active */

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), subvbox);
    Alabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Alabel), Aheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Alabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<ACTIONBNUM;i++) {
	fixed = gtk_fixed_new ();
	gtk_fixed_put((GtkFixed *) fixed, actioncheckb[i], FRAME_INDENT, 0);
	gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);
    }

    subhbox = gtk_hbox_new (FALSE, ELEM_SEP);
    file_label = gtk_label_new(filen_string);
    file_entry = gtk_entry_new();					/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (file_entry), 128);
    gtk_editable_set_editable((GtkEditable *) file_entry,TRUE);
    g_signal_connect (G_OBJECT (file_entry), "changed",			/* Init the entry to call */
                    G_CALLBACK (read_file_entry),NULL);
    gtk_tooltips_set_tip (tooltip,file_entry,filenamett,filenamett);

    gtk_box_pack_start (GTK_BOX (subhbox), file_label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subhbox), file_entry, FALSE, FALSE, 0);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, subhbox, 3*FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(file_label,FALSE);
    gtk_widget_set_sensitive(file_entry,FALSE);

    printbutton = gtk_button_new_from_stock(GTK_STOCK_PRINT);
    closebutton = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), printbutton);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), closebutton);

    g_signal_connect (G_OBJECT (printbutton), "clicked",
                  G_CALLBACK (print_results), GINT_TO_POINTER(FALSE));
    gtk_tooltips_set_tip (tooltip,printbutton,printrestt,printrestt);

    g_signal_connect_swapped (GTK_OBJECT (closebutton),
                             "clicked", 
                             G_CALLBACK (gtk_widget_destroy),
                             GTK_OBJECT (dialog));

    gtk_widget_show_all(dialog);

  return;
}

/****************************************************************/
/* This function is called when movement is detected in the	*/
/* drawing area, it captures the coordinates and zoom in om the */
/* position and plots it on the zoom area.			*/
/****************************************************************/
gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  gint x, y;
  gchar buf[32];
  GdkModifierType 	state;
  static gboolean 	FirstTime = TRUE;
  static GdkGC 		*mngc;						/* Graphic context */
  static GdkPixbuf	*gpbzoomimage;
  struct PointValue	CalcVal;

    gdk_window_get_pointer (event->window, &x, &y, &state);		/* Grab mousepointers coordinates */
									/* on drawing area. */
    mngc = gdk_gc_new (zoom_area->window);				/* Create graphics context */

    if (FirstTime) {
	gpbzoomimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, ZOOMPIXSIZE, ZOOMPIXSIZE);
	FirstTime = FALSE;
    }

    if (x>=0 && y>=0 && x<XSize && y<YSize) {

	gdk_pixbuf_composite(gpbimage, gpbzoomimage, 0, 0, ZOOMPIXSIZE, 
			     ZOOMPIXSIZE, -x*ZOOMFACTOR + ZOOMPIXSIZE/2, 
			     -y*ZOOMFACTOR + ZOOMPIXSIZE/2, 1.0*ZOOMFACTOR, 
			     1.0*ZOOMFACTOR, GDK_INTERP_BILINEAR, 255);
#ifdef GTK22X
	gdk_draw_pixbuf(zoom_area->window,zoom_area->style->white_gc,gpbzoomimage,
			0,0,0,0,ZOOMPIXSIZE,ZOOMPIXSIZE,GDK_RGB_DITHER_NONE,0,0);
#else
        gdk_pixbuf_render_to_drawable(gpbzoomimage,zoom_area->window,zoom_area->style->white_gc,
                                      0,0,0,0,ZOOMPIXSIZE,ZOOMPIXSIZE,GDK_RGB_DITHER_NONE,0,0);
#endif

	DrawMarker(zoom_area, ZOOMPIXSIZE/2, ZOOMPIXSIZE/2, 2, colors);		/* Then draw the square in the middle of the zoom area */

	if (valueset[0] && valueset[1] && valueset[2] && valueset[3]) {
	    CalcVal = CalcPointValue(x,y);

	    sprintf(buf,"%.12g",CalcVal.Xv);
	    gtk_entry_set_text(GTK_ENTRY(xc_entry),buf);		/* Put out coordinates in entries */
	    sprintf(buf,"%.12g",CalcVal.Yv);
	    gtk_entry_set_text(GTK_ENTRY(yc_entry),buf);
	    sprintf(buf,"%.12g",CalcVal.Xerr);
	    gtk_entry_set_text(GTK_ENTRY(xerr_entry),buf);		/* Put out coordinates in entries */
	    sprintf(buf,"%.12g",CalcVal.Yerr);
	    gtk_entry_set_text(GTK_ENTRY(yerr_entry),buf);
	}
	else {
	    gtk_entry_set_text(GTK_ENTRY(xc_entry),"");			/* Else clear entries */
	    gtk_entry_set_text(GTK_ENTRY(yc_entry),"");
	    gtk_entry_set_text(GTK_ENTRY(xerr_entry),"");
	    gtk_entry_set_text(GTK_ENTRY(yerr_entry),"");
	}
    } else {
	gtk_entry_set_text(GTK_ENTRY(xc_entry),"");			/* Else clear entries */
	gtk_entry_set_text(GTK_ENTRY(yc_entry),"");
	gtk_entry_set_text(GTK_ENTRY(xerr_entry),"");
	gtk_entry_set_text(GTK_ENTRY(yerr_entry),"");
    }
    g_object_unref(mngc);						/* Kill graphics context */
    return TRUE;
}


/****************************************************************/
/* This function is called when the drawing area is exposed, it	*/
/* simply redraws the pixmap on it.				*/
/****************************************************************/
static gint expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  gint i;

/*
  printf("event->area.x = %d\n",event->area.x);
  printf("event->area.y = %d\n",event->area.y);
  printf("event->area.width = %d\n",event->area.width);
  printf("event->area.height = %d\n",event->area.height);
  printf("XSize = %d\n", XSize);
  printf("YSize = %d\n\n", YSize);
*/

/* Using the following function will cause g3data to crash. A GTK+ bug ??? */
/*
    gdk_draw_pixbuf(widget->window,widget->style->white_gc,gpbimage,
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    min(event->area.width,XSize), min(event->area.height,YSize),
		    GDK_RGB_DITHER_NONE,0,0);
*/

#ifdef GTK22X
    gdk_draw_pixbuf(widget->window,widget->style->white_gc,gpbimage,		/* Forcing a redra of the whole pixbuf seems to	*/
		    0, 0,							/* work just fine although it is slower.	*/
		    0, 0,
		    XSize, YSize,
		    GDK_RGB_DITHER_NONE,0,0);
#else
    gdk_draw_pixmap(widget->window,widget->style->white_gc,pixmap,
          event->area.x, event->area.y,
          event->area.x, event->area.y,
          event->area.width, event->area.height);
#endif

    for (i=0;i<4;i++) if (bpressed[i]) DrawMarker(drawing_area, axiscoords[i][0], axiscoords[i][1], i/2, colors);
    for (i=0;i<numpoints;i++) DrawMarker(drawing_area, points[i][0], points[i][1], 2, colors);

    return FALSE;
}   


/****************************************************************/
/* This function is called when the drawing area is configured	*/
/* for the first time, currently this function does not perform	*/
/* any task.							*/
/****************************************************************/
gint configure_event(GtkWidget *widget, GdkEventConfigure *event,gpointer data)
{
    return TRUE;
}


/****************************************************************/
/* This function is called when the "Set point 1/2 on x/y axis"	*/
/* button is pressed. It inactivates the other "Set" buttons	*/
/* and makes sure the button stays down even when pressed on.	*/
/****************************************************************/
void toggle_xy(GtkWidget *widget, gpointer func_data)
{
  gint index, i;

    index = GPOINTER_TO_INT (func_data);

    if (GTK_TOGGLE_BUTTON (widget)->active) {				/* Is the button pressed on ? */
	setxypressed[index]=TRUE;					/* The button is pressed down */
	for (i=0;i<4;i++) {
	    if (index != i) gtk_widget_set_sensitive(setxybutton[i],FALSE);
	}
	if (bpressed[index]) {						/* If the x axis point is already set */
	    remthis=-(index+1);						/* remove the square */
	    remove_last(widget,NULL);
	}
	bpressed[index]=FALSE;						/* Set x axis point 1 to unset */
    } else {								/* If button is trying to get unpressed */
	if (setxypressed[index]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE); /* Set button down */
    }
}


/****************************************************************/
/* Set type of ordering at output of data.			*/
/****************************************************************/
void SetOrdering(GtkWidget *widget, gpointer func_data)
{
    ordering = GPOINTER_TO_INT (func_data);				/* Set ordering control variable */
}


/****************************************************************/
/****************************************************************/
void SetAction(GtkWidget *widget, gpointer func_data)
{
    Action = GPOINTER_TO_INT (func_data);
    if (Action == PRINT2FILE) {
	gtk_widget_set_sensitive(file_label, TRUE);
	gtk_widget_set_sensitive(file_entry, TRUE);
	if (strlen(gtk_entry_get_text(GTK_ENTRY (file_entry))) > 0) gtk_widget_set_sensitive(printbutton, TRUE);
	else gtk_widget_set_sensitive(printbutton, FALSE);
    } else {
	gtk_widget_set_sensitive(file_label, FALSE);
	gtk_widget_set_sensitive(file_entry, FALSE);
	gtk_widget_set_sensitive(printbutton, TRUE);
    }
}


/****************************************************************/
/* Set whether to use error evaluation and printing or not.	*/
/****************************************************************/
void UseErrCB(GtkWidget *widget, gpointer func_data)
{
    UseErrors = (GTK_TOGGLE_BUTTON (widget)->active);
}

/****************************************************************/
/* When the value of the entry of any axis point is changed, 	*/
/* this function gets called.					*/
/****************************************************************/
void read_xy_entry(GtkWidget *entry, gpointer func_data)
{
  gchar *xy_text;
  gint index;
    
    index = GPOINTER_TO_INT (func_data);

    xy_text = (gchar *) gtk_entry_get_text(GTK_ENTRY (entry));
    sscanf(xy_text,"%lf",&realcoords[index]);				/* Convert string to double value and */
									/* store in realcoords[0]. */
    if (logxy[index/2] && realcoords[index] > 0) valueset[index]=TRUE;
    else if (logxy[index/2]) valueset[index]=FALSE;
    else valueset[index]= TRUE; 

    if (bpressed[0] && bpressed[1] && bpressed[2] && 			/* Are all buttons pressed and all values set ? */
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0) {
	if (GTK_IS_WIDGET(printbutton)) gtk_widget_set_sensitive(printbutton,TRUE);	
	gtk_widget_set_sensitive(exportbutton,TRUE);			/* Activate printoutbutton */
	if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE); /* If filename entered activate printouttofilebutton */
    }
    else {
	if (GTK_IS_WIDGET(printbutton)) gtk_widget_set_sensitive(printbutton,FALSE);			/* Else deactivate buttons */
	gtk_widget_set_sensitive(exportbutton,FALSE);			/* Else deactivate buttons */
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
}


/****************************************************************/
/* If all the axispoints has been put out, values for these	*/
/* have been assigned and at least one point has been set on	*/
/* the graph activate the write to file button.			*/
/****************************************************************/
void read_file_entry(GtkWidget *entry, GtkWidget *widget)
{
    file_name = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
    file_name_length = strlen(file_name);				/* Get length of string */

    if (bpressed[0] && bpressed[1] && bpressed[2] && 
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0 &&
	file_name_length>0) {
	gtk_widget_set_sensitive(printbutton,TRUE);
    }
    else gtk_widget_set_sensitive(printbutton,FALSE);

}


/****************************************************************/
/* If the "X/Y axis is logarithmic" check button is toggled	*/
/* this function gets called. It sets the logx variable to its	*/
/* correct value corresponding to the buttons state.		*/
/****************************************************************/
void islogxy(GtkWidget *widget, gpointer func_data)
{
  gint index;

    index = GPOINTER_TO_INT (func_data);

    logxy[index] = (GTK_TOGGLE_BUTTON (widget)->active); 		/* If checkbutton is pressed down */
									/* logxy = TRUE else FALSE. */
    if (logxy[index]) {
	if (realcoords[index*2] <= 0) {					/* If a negative value has been insert */
	    valueset[index*2]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(xyentry[index*2]),"");		/* Zero it */
	}
	if (realcoords[index*2+1] <= 0) {				/* If a negative value has been insert */
	    valueset[index*2+1]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(xyentry[index*2+1]),"");	/* Zero it */
        }
    }
}


/****************************************************************/
/* This function removes the last inserted point or the point	*/
/* indexed by remthis (<0).					*/
/****************************************************************/
void remove_last(GtkWidget *widget, gpointer data) 
{
  gint i, j;

/* First redraw the drawing_area with the original image, to clean it. */

#ifdef GTK22X
    gdk_draw_pixbuf(drawing_area->window,widget->style->white_gc,gpbimage,
		    0, 0, 0, 0, XSize, YSize,GDK_RGB_DITHER_NONE,0,0);
#else
    gdk_draw_pixmap(drawing_area->window,widget->style->white_gc,
                    pixmap,0,0,0,0,XSize,YSize);
#endif 

    if (numlastpoints>0) {						/* If points been put out, remove last one */
	if (remthis==0) {						/* If remthis is 0, ignore it.		*/
	    numlastpoints--;
	    for (i=0;i<4;i++) if (lastpoints[numlastpoints]==-(i+1)) {	/* If point to be removed is axispoint 1-4 */
		bpressed[i]=FALSE;					/* Mark it unpressed.			*/
		gtk_widget_set_sensitive(xyentry[i],FALSE);		/* Inactivate entry for point.		*/
		break;
	    }
	    if (i==4) numpoints--;					/* If its none of the X/Y markers then	*/
	    SetNumPointsEntry(nump_entry, numpoints);			/* its an ordinary marker, remove it.	 */
	}

	if (numlastpoints>0) {						/* If more than 0 points left, start to redraw */
	    for (i=0;i<numlastpoints;i++) {				/* Loop over all remaining points.	*/
		for (j=0;j<4;j++) if (lastpoints[i]==-(j+1) && remthis!=-(j+1)) DrawMarker(drawing_area, axiscoords[j][0], axiscoords[j][1], j/2, colors);
		if (lastpoints[i]>=0) DrawMarker(drawing_area, points[lastpoints[i]][0], points[lastpoints[i]][1], 2, colors);
	    }
	}
    }

/* If no points are left, deactive the "Remove last" and "Remove All" button. */
    if (numlastpoints==0) {
	gtk_widget_set_sensitive(remlastbutton,FALSE);
	gtk_widget_set_sensitive(remallbutton,FALSE);
    }

/* If not enough points to print data, deactivate "Print data" and "Print to file" button */
    if (bpressed[0]==FALSE || bpressed[1]==FALSE || bpressed[2]==FALSE || 
	bpressed[3]==FALSE || numpoints==0) {
	if (GTK_IS_WIDGET(printbutton)) gtk_widget_set_sensitive(printbutton,FALSE);
	gtk_widget_set_sensitive(exportbutton,FALSE);
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
    remthis = 0;							/* Reset remthis variable */
}


/****************************************************************/
/* This function sets the proper variables and then calls 	*/
/* remove_last, to remove all points except the axis points.	*/
/****************************************************************/
void remove_all(GtkWidget *widget, gpointer data) 
{
  gint i, j, index;

    if (numlastpoints>0 && numpoints>0) {
	index = 0;
	for (i=0;i<numlastpoints;i++) for (j=0;j<4;j++) {		/* Search for axispoints and store them in */
	    if (lastpoints[i]==-(j+1)) {				/* lastpoints at the first positions.      */
		lastpoints[index] = -(j+1);
		index++;
	    }
	}
	lastpoints[index] = 0;

	numlastpoints = index+1;
	numpoints = 1;
	SetNumPointsEntry(nump_entry, numpoints);

	remove_last(widget,data);					/* Call remove_last() */
    }
    else if (numlastpoints>0 && numpoints==0) {
	numlastpoints = 0;						/* Nullify amount of points */
	for (i=0;i<4;i++) {
	    valueset[i] = FALSE;
	    bpressed[i] = FALSE;
	    gtk_entry_set_text((GtkEntry *) xyentry[i], "");
	}
	remove_last(widget,data);					/* Call remove_last() */
    }
}


/****************************************************************/
/* This function is called when the "Print to file" button is	*/
/* pressed, it writes the data to a file named after the string	*/
/* contained in the filename entry box. After that it calls the	*/
/* print_results function to do the actual processing and 	*/
/* printing of the data.					*/
/****************************************************************/
void fileoutput(GtkWidget *widget, gpointer data) 
{
    FP = fopen(file_name,"w");						/* Open file for writing */
    if (FP==NULL) printf("Could not open %s for writing\n",file_name);	/* If unable to open print error */
    else {								/* If open successfull */
	print_results(widget,GINT_TO_POINTER(FALSE));			/* Call print_results() */
	fclose(FP);							/* Close the file */
    }
}


/****************************************************************/
/****************************************************************/
gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer)
{
  GtkAdjustment *adjustment;
  gdouble adj_val;

  if (UseScrolling) {
    if (event->keyval==GDK_Left) {
	adjustment = gtk_viewport_get_hadjustment((GtkViewport *) ViewPort);
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val -= adjustment->page_size/10.0;
	if (adj_val < adjustment->lower) adj_val = adjustment->lower;
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_hadjustment((GtkViewport *) ViewPort, adjustment);
    } else if (event->keyval==GDK_Right) {
	adjustment = gtk_viewport_get_hadjustment((GtkViewport *) ViewPort);
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val += adjustment->page_size/10.0;
	if (adj_val > (adjustment->upper-adjustment->page_size)) adj_val = (adjustment->upper-adjustment->page_size);
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_hadjustment((GtkViewport *) ViewPort, adjustment);
    } else if (event->keyval==GDK_Up) {
	adjustment = gtk_viewport_get_vadjustment((GtkViewport *) ViewPort);
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val -= adjustment->page_size/10.0;
	if (adj_val < adjustment->lower) adj_val = adjustment->lower;
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_vadjustment((GtkViewport *) ViewPort, adjustment);
    } else if (event->keyval==GDK_Down) {
	adjustment = gtk_viewport_get_vadjustment((GtkViewport *) ViewPort);
	adj_val = gtk_adjustment_get_value(adjustment);
	adj_val += adjustment->page_size/10.0;
	if (adj_val > (adjustment->upper-adjustment->page_size)) adj_val = (adjustment->upper-adjustment->page_size);
	gtk_adjustment_set_value(adjustment, adj_val);
	gtk_viewport_set_vadjustment((GtkViewport *) ViewPort, adjustment);
    }
  }
    if (event->keyval==GDK_F10) {
	if (!WinFullScreen) {
	    gtk_window_fullscreen(GTK_WINDOW (window));
	    WinFullScreen = TRUE;
	} else {
	    gtk_window_unfullscreen(GTK_WINDOW (window));
	    WinFullScreen = FALSE;
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
  gchar 	buf[100];						/* Text buffer for window title */
  gint 		FileIndex, i, maxX, maxY, newX, newY;
  gdouble 	Scale, mScale;

  GtkWidget 	*table;							/* GTK table/box variables for packing */
  GtkWidget	*tophbox, *bottomhbox;
  GtkWidget	*trvbox, *tlvbox, *brvbox, *blvbox, *subvbox;
  GtkWidget 	*xy_label[4];						/* Labels for texts in window */
  GtkWidget 	*logcheckb[2];						/* Logarithmic checkbuttons */
  GtkWidget 	*nump_label, *ScrollWindow;				/* Various widgets */
  GtkWidget 	*x_label, *y_label, *tmplabel;
  GtkWidget	*APlabel, *PIlabel, *ZAlabel, *Llabel;
  GtkTooltips 	*tooltip;
  GtkWidget 	*alignment, *fixed;
  GdkPixbuf	*loadgpbimage;
  GdkCursor	*cursor;

#ifdef GTK20X
  GdkBitmap	*mask;
#endif

    gtk_init (&argc, &argv);						/* Init GTK */

    if (argc<2 || strcmp(argv[1],"-h")==0 ||				/* If no parameters given, -h or --help */
	strcmp(argv[1],"--help")==0) {
	printf("%s",HelpText);						/* Print help */
	exit(0);							/* and exit */
    }

    maxX = -1;
    maxY = -1;
    Scale = -1;
    FileIndex = -1;
    UseScrolling = FALSE;
    UseErrors = FALSE;
    UsePreSetCoords = FALSE;
    for (i=1;i<argc;i++) {
	if (*(argv[i])=='-') {
	    if (strcmp(argv[i],"-scale")==0) {
		if (argc-i < 2) {
		    printf("Too few parameters for -scale\n");
		    exit(0);
		}
		if (sscanf(argv[i+1],"%lf",&Scale)!=1) {
		    printf("-scale parameter in invalid form !\n");
		    exit(0);
		}
		i++;
		if (i >= argc) break;
	    }
	    else if (strcmp(argv[i],"-scroll")==0) {
		UseScrolling = TRUE;
	    }
	    else if (strcmp(argv[i],"-errors")==0) {
		UseErrors = TRUE;
	    }
	    else if (strcmp(argv[i],"-lnx")==0) {
		logxy[0] = TRUE;
	    }
	    else if (strcmp(argv[i],"-lny")==0) {
		logxy[1] = TRUE;
	    }
	    else if (strcmp(argv[i],"-max")==0) {
		if (argc-i < 3) {
		    printf("Too few parameters for -max\n");
		    exit(0);
		}
		if (sscanf(argv[i+1],"%d", &maxX)!=1) {
		    printf("-max first parameter in invalid form !\n");
		    exit(0);
		}
		if (sscanf(argv[i+2],"%d", &maxY)!=1) {
		    printf("-max second parameter in invalid form !\n");
		    exit(0);
		}
		i+=2;
		if (i >= argc) break;
	    }
	    else if (strcmp(argv[i],"-coords")==0) {
		UsePreSetCoords = TRUE;
		if (argc-i < 5) {
		    printf("Too few parameters for -coords\n");
		    exit(0);
		}
		if (sscanf(argv[i+1],"%lf", &realcoords[0])!=1) {
		    printf("-max first parameter in invalid form !\n");
		    exit(0);
		} else {bpressed[0] = TRUE; valueset[0] = TRUE;}
		if (sscanf(argv[i+2],"%lf", &realcoords[2])!=1) {
		    printf("-max second parameter in invalid form !\n");
		    exit(0);
		} else {bpressed[1] = TRUE; valueset[1] = TRUE;}
		if (sscanf(argv[i+3],"%lf", &realcoords[1])!=1) {
		    printf("-max third parameter in invalid form !\n");
		    exit(0);
		} else {bpressed[2] = TRUE; valueset[2] = TRUE;}
		if (sscanf(argv[i+4],"%lf", &realcoords[3])!=1) {
		    printf("-max fourth parameter in invalid form !\n");
		    exit(0);
		} else {bpressed[3] = TRUE; valueset[3] = TRUE;}
		i+=4;
		if (i >= argc) break;
	    }
	    else {
		printf("Unknown parameter : %s\n", argv[i]);
		exit(0);
	    }
	    continue;
	}
	else if (FileIndex == -1) FileIndex=i;
	else {
	    printf("You cant specify multiple files !\n");
	    exit(0);
	}
    }
    if (FileIndex == -1) {
	printf("You have to specify a file to open !\n");
	exit(0);
    }

    lastpoints = (gint *) malloc(sizeof(gint) * (MaxPoints+4));
    if (lastpoints==NULL) {
	printf("Error allocating memory for lastpoints. Exiting.\n");
	return -1;
    }
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

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);			/* Create window */
    sprintf(buf, Window_Title, argv[FileIndex]);			/* Print window title in buffer */
    gtk_window_set_title (GTK_WINDOW (window), buf);			/* Set window title */
    gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (window), WINDOW_BORDER);	/* Set borders in window */

    g_signal_connect (G_OBJECT (window), "delete_event",		/* Init delete event of window */
                        G_CALLBACK (close_application), NULL);

    tooltip = gtk_tooltips_new();

    for(i=0;i<4;i++) {
	xyentry[i] = gtk_entry_new();  						/* Create text entry */
	gtk_entry_set_max_length (GTK_ENTRY (xyentry[i]), 20);
	gtk_editable_set_editable((GtkEditable *) xyentry[i],FALSE);
	gtk_widget_set_sensitive(xyentry[i],FALSE);				/* Inactivate it */
	g_signal_connect (G_OBJECT (xyentry[i]), "changed",			/* Init the entry to call */
			  G_CALLBACK (read_xy_entry), GINT_TO_POINTER (i));	/* read_x1_entry whenever */
	gtk_tooltips_set_tip (tooltip,xyentry[i],entryxytt[i],entryxytt[i]);
    }

    x_label = gtk_label_new(x_string);
    y_label = gtk_label_new(y_string);
    xc_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (xc_entry), 16);
    gtk_editable_set_editable((GtkEditable *) xc_entry,FALSE);
    yc_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (yc_entry), 16);
    gtk_editable_set_editable((GtkEditable *) yc_entry,FALSE);

    pm_label = gtk_label_new(pm_string);
    pm_label2 = gtk_label_new(pm_string);
    xerr_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (xerr_entry), 16);
    gtk_editable_set_editable((GtkEditable *) xerr_entry,FALSE);
    yerr_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (yerr_entry), 16);
    gtk_editable_set_editable((GtkEditable *) yerr_entry,FALSE);

    nump_label = gtk_label_new(nump_string);
    nump_entry = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (nump_entry), 10);
    gtk_editable_set_editable((GtkEditable *) nump_entry,FALSE);
    SetNumPointsEntry(nump_entry, numpoints);
    
    loadgpbimage = gdk_pixbuf_new_from_file(argv[FileIndex],NULL);	/* Load image */
    if (loadgpbimage==NULL) {						/* If unable to load image */
	printf("Error : Error during loading of file : %s\n",argv[FileIndex]);	/* Print error message and */
	return -1;							/* exit */
    }
    XSize = gdk_pixbuf_get_width(loadgpbimage);				/* Get image width */
    YSize = gdk_pixbuf_get_height(loadgpbimage);			/* Get image height */

    mScale = -1;
    if (maxX != -1 && maxY != -1) {
	if (XSize > maxX) {
	    mScale = (double) maxX/XSize;
	}
	if (YSize > maxY && (double) maxY/YSize < mScale) mScale = (double) maxY/YSize;
    }

    if (Scale == -1 && mScale != -1) Scale = mScale;

    if (Scale != -1) {
	newX = XSize*Scale;
	newY = YSize*Scale;
	gpbimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, newX, newY);
	gdk_pixbuf_composite(loadgpbimage, gpbimage, 0, 0, newX, newY,
			     0, 0, Scale, Scale, GDK_INTERP_BILINEAR, 255);
	g_object_unref(loadgpbimage);
    }
    else gpbimage = loadgpbimage;

    XSize = gdk_pixbuf_get_width(gpbimage);				/* Get image width */
    YSize = gdk_pixbuf_get_height(gpbimage);				/* Get image height */

#ifdef GTK20X
    gdk_pixbuf_render_pixmap_and_mask(gpbimage, &pixmap, &mask, 255);
#endif

    drawing_area = gtk_drawing_area_new ();				/* Create new drawing area */
    gtk_widget_set_size_request (drawing_area, XSize, YSize);

    zoom_area = gtk_drawing_area_new ();				/* Create new drawing area */
    gtk_widget_set_size_request (zoom_area, ZOOMPIXSIZE, ZOOMPIXSIZE);

    setcolors(&colors);

    g_signal_connect (G_OBJECT (drawing_area), "expose_event",		/* Connect drawing area to */
              G_CALLBACK (expose_event), NULL);				/* expose_event. */

    g_signal_connect (G_OBJECT (drawing_area), "configure_event",	/* Connect drawing area to */
              G_CALLBACK (configure_event), NULL);			/* configure_event. */

    g_signal_connect (G_OBJECT (drawing_area), "button_press_event",	/* Connect drawing area to */
              G_CALLBACK (button_press_event), NULL);			/* button_press_event. */

    g_signal_connect (G_OBJECT (drawing_area), "button_release_event",	/* Connect drawing area to */
              G_CALLBACK (button_release_event), NULL);			/* button_release_event */

    g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",	/* Connect drawing area to */
              G_CALLBACK (motion_notify_event), NULL);			/* motion_notify_event. */

    gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK |		/* Set the events active */
			   GDK_BUTTON_PRESS_MASK | 
			   GDK_BUTTON_RELEASE_MASK |
			   GDK_POINTER_MOTION_MASK | 
			   GDK_POINTER_MOTION_HINT_MASK);

    for (i=0;i<4;i++) {
	xy_label[i] = gtk_label_new(NULL);
	gtk_label_set_markup((GtkLabel *) xy_label[i], xy_label_text[i]);
    }

    for (i=0;i<4;i++) {
	tmplabel = gtk_label_new(NULL);
	gtk_label_set_markup_with_mnemonic((GtkLabel *) tmplabel, setxylabel[i]);
	setxybutton[i] = gtk_toggle_button_new();				/* Create button */
	gtk_container_add((GtkContainer *) setxybutton[i], tmplabel);
	g_signal_connect (G_OBJECT (setxybutton[i]), "toggled",			/* Connect button */
			  G_CALLBACK (toggle_xy), GINT_TO_POINTER (i));
	gtk_tooltips_set_tip (tooltip,setxybutton[i],setxytts[i],setxytt[i]);
    }

    quitbutton = gtk_button_new_from_stock (GTK_STOCK_QUIT);		/* Create button */
    g_signal_connect (G_OBJECT (quitbutton), "clicked",			/* Connect button */
                  G_CALLBACK (gtk_main_quit), NULL);

    exportbutton = gtk_button_new_with_mnemonic (PrintBLabel);		/* Create button */
    g_signal_connect (G_OBJECT (exportbutton), "clicked",		/* Connect button */
                  G_CALLBACK (export_values), GINT_TO_POINTER(FALSE));
//    g_signal_connect (G_OBJECT (exportbutton), "clicked",		/* Connect button */
//                  G_CALLBACK (print_results), GINT_TO_POINTER(FALSE));
    gtk_widget_set_sensitive(exportbutton,FALSE);
    gtk_tooltips_set_tip (tooltip,exportbutton,printrestt,printrestt);

    remlastbutton = gtk_button_new_with_mnemonic (RemLastBLabel);/* Create button */
    g_signal_connect (G_OBJECT (remlastbutton), "clicked",		/* Connect button */
                  G_CALLBACK (remove_last), NULL);
    gtk_widget_set_sensitive(remlastbutton,FALSE);
    gtk_tooltips_set_tip (tooltip,remlastbutton,removeltt,removeltt);

    remallbutton = gtk_button_new_with_mnemonic (RemAllBLabel);	/* Create button */
    g_signal_connect (G_OBJECT (remallbutton), "clicked",		/* Connect button */
                  G_CALLBACK (remove_all), NULL);
    gtk_widget_set_sensitive(remallbutton,FALSE);
    gtk_tooltips_set_tip (tooltip,remallbutton,removeatts,removeatt);

    fileoutputb = gtk_button_new_with_mnemonic (PrintToFileBLabel);	/* Create button */
    g_signal_connect (G_OBJECT (fileoutputb), "clicked",		/* Connect button */
                  G_CALLBACK (fileoutput), NULL);
    gtk_widget_set_sensitive(fileoutputb,FALSE);

    for (i=0;i<2;i++) {
	logcheckb[i] = gtk_check_button_new_with_mnemonic(loglabel[i]);	/* Create check button */
	g_signal_connect (G_OBJECT (logcheckb[i]), "toggled",		/* Connect button */
			  G_CALLBACK (islogxy), GINT_TO_POINTER (i));
	gtk_tooltips_set_tip (tooltip,logcheckb[i],logxytt[i],logxytt[i]);
	gtk_toggle_button_set_active (( GtkToggleButton *) logcheckb[i], logxy[i]);
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
	    gtk_editable_set_editable((GtkEditable *) xyentry[i],TRUE);
	    sprintf(buf,"%lf",realcoords[i]);
	    gtk_entry_set_text((GtkEntry *) xyentry[i], buf);
	    lastpoints[numlastpoints]=-(i+1);
	    numlastpoints++;
	    valueset[i] = TRUE;
	    bpressed[i] = TRUE;
	    setxypressed[i]=FALSE;
	}
	gtk_widget_set_sensitive(exportbutton,TRUE);
     }

    table = gtk_table_new(1, 2 ,FALSE);					/* Create table */
    gtk_table_set_row_spacings(GTK_TABLE(table), SECT_SEP);		/* Set spacings */
    gtk_table_set_col_spacings(GTK_TABLE(table), 0);
    gtk_container_add( GTK_CONTAINER(window),table);

    tophbox = gtk_hbox_new (FALSE, SECT_SEP);
    alignment = gtk_alignment_new (0,0,0,0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 5, 0, 0, 0);
    gtk_container_add((GtkContainer *) alignment, tophbox);

    bottomhbox = gtk_hbox_new (FALSE, SECT_SEP);
    if (UseScrolling) alignment = gtk_alignment_new (0, 0, 1, 1);
    else alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 1, 2, 5, 5, 0, 0);
    gtk_container_add((GtkContainer *) alignment, bottomhbox);

    tlvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (tophbox), tlvbox, FALSE, FALSE, ELEM_SEP);
    APlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (APlabel), APheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, APlabel);
    gtk_box_pack_start (GTK_BOX (tlvbox), alignment, FALSE, FALSE, 0);
    table = gtk_table_new(3, 4 ,FALSE);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, table, FRAME_INDENT, 0);
    gtk_table_set_row_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_table_set_col_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (tlvbox), fixed, FALSE, FALSE, 0);
    for (i=0;i<4;i++) {
	gtk_table_attach(GTK_TABLE(table), setxybutton[i], 0, 1, i, i+1, 5, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), xy_label[i], 1, 2, i, i+1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), xyentry[i], 2, 3, i, i+1, 0, 0, 0, 0);
    }

    trvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (tophbox), trvbox, FALSE, FALSE, ELEM_SEP);

    PIlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (PIlabel), PIheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, PIlabel);
    gtk_box_pack_start (GTK_BOX (trvbox), alignment, FALSE, FALSE, 0);

    table = gtk_table_new(4, 2 ,FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), ELEM_SEP);
    gtk_table_set_col_spacings(GTK_TABLE(table), ELEM_SEP);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, table, FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (trvbox), fixed, FALSE, FALSE, 0);
    gtk_table_attach(GTK_TABLE(table), x_label, 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), xc_entry, 1, 2, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), pm_label, 2, 3, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), xerr_entry, 3, 4, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), y_label, 0, 1, 1, 2, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), yc_entry, 1, 2, 1, 2, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), pm_label2, 2, 3, 1, 2, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), yerr_entry, 3, 4, 1, 2, 0, 0, 0, 0);

    table = gtk_table_new(3, 1 ,FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 6);
    gtk_table_set_col_spacings(GTK_TABLE(table), 6);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, table, FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (trvbox), fixed, FALSE, FALSE, 0);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, nump_label);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), nump_entry, 1, 2, 0, 1, 0, 0, 0, 0);
//    alignment = gtk_alignment_new (0, 1, 0, 0);
//    gtk_container_add((GtkContainer *) alignment, file_label);
//    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 1, 2, 5, 0, 0, 0);
//    gtk_table_attach(GTK_TABLE(table), file_entry, 1, 2, 1, 2, 0, 0, 0, 0);
//    gtk_table_attach(GTK_TABLE(table), fileoutputb, 2, 3, 1, 2, 0, 0, 0, 0);

    blvbox = gtk_vbox_new (FALSE, GROUP_SEP);
    gtk_box_pack_start (GTK_BOX (bottomhbox), blvbox, FALSE, FALSE, ELEM_SEP);

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    ZAlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (ZAlabel), ZAheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, ZAlabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, zoom_area, FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    Llabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Llabel), Lheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Llabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<2;i++) {
	fixed = gtk_fixed_new ();
	gtk_fixed_put((GtkFixed *) fixed, logcheckb[i], FRAME_INDENT, 0);
	gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);		/* Pack checkbutton in vert. box */
    }

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), remlastbutton, FALSE, FALSE, 0);	/* Pack button in vert. box */
    gtk_box_pack_start (GTK_BOX (subvbox), remallbutton, FALSE, FALSE, 0);		/* Pack button in vert. box */
    gtk_box_pack_start (GTK_BOX (subvbox), exportbutton, FALSE, FALSE, 0);		/* Pack button in vert. box */
    gtk_box_pack_start (GTK_BOX (subvbox), quitbutton, FALSE, FALSE, 0);		/* Pack button in vert. box */

    brvbox = gtk_vbox_new (FALSE, GROUP_SEP);
    gtk_box_pack_start (GTK_BOX (bottomhbox), brvbox, TRUE, TRUE, 0);

    if (UseScrolling) {
	ScrollWindow = gtk_scrolled_window_new(NULL,NULL);
	ViewPort = gtk_viewport_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (brvbox), ScrollWindow, TRUE, TRUE, 0);
	alignment = gtk_alignment_new (0, 0, 0, 0);
	gtk_container_add (GTK_CONTAINER (ViewPort), alignment);
	gtk_container_add((GtkContainer *) alignment, drawing_area);
	gtk_container_add (GTK_CONTAINER (ScrollWindow), ViewPort);
    } else {
	gtk_box_pack_start (GTK_BOX (brvbox), drawing_area, FALSE, FALSE, 0);
    }

        g_signal_connect_swapped (G_OBJECT (window), "key_press_event",
			          G_CALLBACK (key_press_event), NULL);

    gtk_widget_show_all(window);					/* Show all widgets */

    cursor = gdk_cursor_new (GDK_CROSSHAIR);
    gdk_window_set_cursor (drawing_area->window, cursor);

    gtk_main();								/* This is where it all starts */
              
    free(colors);							/* Deallocate memory */

    return(0);								/* Exit. */
}
