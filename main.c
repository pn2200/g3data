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
#include <unistd.h>
#include "main.h"									/* Include predined variables */
#include "strings.h"									/* Include strings */

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
GtkWidget	*drawing_area[MAXNUMTABS], *zoom_area[MAXNUMTABS];			/* Drawing areas */
GtkWidget	*xyentry[MAXNUMTABS][4];
GtkWidget	*exportbutton[MAXNUMTABS], *remlastbutton[MAXNUMTABS]; 			/* Various buttons */
GtkWidget	*setxybutton[MAXNUMTABS][4];
GtkWidget	*remallbutton[MAXNUMTABS];						/* Even more various buttons */
GtkWidget	*xc_entry[MAXNUMTABS],*yc_entry[MAXNUMTABS];
GtkWidget	*file_entry[MAXNUMTABS], *nump_entry[MAXNUMTABS];
GtkWidget	*xerr_entry[MAXNUMTABS],*yerr_entry[MAXNUMTABS];			/* Coordinate and filename entries */
GtkWidget       *logbox[MAXNUMTABS] = {NULL}, *zoomareabox[MAXNUMTABS] = {NULL}, *oppropbox[MAXNUMTABS] = {NULL};
GtkWidget	*pm_label, *pm_label2, *file_label;
GtkWidget	*ViewPort;
GdkColor        *colors;								/* Pointer to colors */
GdkPixbuf       *gpbimage[MAXNUMTABS];
GtkWidget	*mainnotebook;
GtkActionGroup	*tab_action_group;
GtkTooltips 	*tooltip;

/* Declaration of global variables */
gint		axiscoords[MAXNUMTABS][4][2];						/* X,Y coordinates of axispoints */
gint		**points[MAXNUMTABS];							/* Indexes of graphpoints and their coordinates */
gint		*lastpoints[MAXNUMTABS];						/* Indexes of last points put out */
gint		numpoints[MAXNUMTABS];
gint		numlastpoints[MAXNUMTABS];						/* Number of points on graph and last put out */
gint		remthis = 0, ordering[MAXNUMTABS];					/* Various control variables */
gint		XSize[MAXNUMTABS], YSize[MAXNUMTABS];
gint		file_name_length[MAXNUMTABS];
gint 		MaxPoints[MAXNUMTABS] = {MAXPOINTS};
gint		Action[MAXNUMTABS];
gint		ViewedTabNum = -1;							/* The currently viewed tab */
gint		NoteBookNumPages = 0;
gdouble		realcoords[MAXNUMTABS][4];						/* X,Y coords on graph */
gboolean	UseErrors[MAXNUMTABS], WinFullScreen;
gboolean	setxypressed[MAXNUMTABS][4];
gboolean	bpressed[MAXNUMTABS][4];						/* What axispoints have been set out ? */
gboolean	valueset[MAXNUMTABS][4];
gboolean	logxy[MAXNUMTABS][2] = {{FALSE,FALSE}};
gboolean	MovePointMode = FALSE;
gboolean        HideLog = FALSE, HideZoomArea = FALSE, HideOpProp = FALSE;
gchar 		*file_name[MAXNUMTABS];							/* Pointer to filename */
gchar		FileNames[MAXNUMTABS][256];
FILE		*FP;									/* File pointer */

GtkWidget 	*drawing_area_alignment;

/* Declaration of extern functions */

extern	void SetNumPointsEntry(GtkWidget *np_entry, gint np);
extern	gint min(gint x, gint y);
extern	void DrawMarker(GtkWidget *da, gint x, gint y, gint type, GdkColor *color);
extern	struct PointValue CalcPointValue(gint Xpos, gint Ypos, gint TabNum);
extern	void print_results(GtkWidget *widget, gpointer func_data);
extern	gboolean setcolors(GdkColor **color);

/* Explicit declaration of functions */

void remove_last(GtkWidget *widget, gpointer data);
void SetOrdering(GtkWidget *widget, gpointer func_data);
void SetAction(GtkWidget *widget, gpointer func_data);
void UseErrCB(GtkWidget *widget, gpointer func_data);
void read_file_entry(GtkWidget *entry, gpointer func_data);

// GCallback full_screen_action_callback(GtkWidget *widget, gpointer func_dat);

/****************************************************************/
/* This function closes the window when the application is 	*/
/* killed.							*/
/****************************************************************/
gint close_application(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_main_quit();									/* Quit gtk */
    return FALSE;
}


/****************************************************************/
/* This function sets the sensitivity of the buttons depending	*/
/* the control variables.					*/
/****************************************************************/
void SetButtonSensitivity(int TabNum)
{
  char ttbuf[256];

    if (Action[TabNum] == PRINT2FILE) {
	snprintf(ttbuf, sizeof(ttbuf), printfilett, gtk_entry_get_text(GTK_ENTRY (file_entry[TabNum])));
	gtk_tooltips_set_tip (tooltip,exportbutton[TabNum],ttbuf,ttbuf);
	gtk_widget_set_sensitive(file_entry[TabNum], TRUE);
	if (valueset[TabNum][0] && valueset[TabNum][1] && 
	    valueset[TabNum][2] && valueset[TabNum][3] && 
	    bpressed[TabNum][0] && bpressed[TabNum][1] && 
	    bpressed[TabNum][2] && bpressed[TabNum][3] && 
	    numpoints[TabNum] > 0 && file_name_length[TabNum] > 0) 
		gtk_widget_set_sensitive(exportbutton[TabNum], TRUE);
	else gtk_widget_set_sensitive(exportbutton[TabNum], FALSE);
    } else {
	gtk_tooltips_set_tip (tooltip,exportbutton[TabNum],printrestt,printrestt);
	gtk_widget_set_sensitive(file_entry[TabNum], FALSE);
	if (valueset[TabNum][0] && valueset[TabNum][1] && 
	    valueset[TabNum][2] && valueset[TabNum][3] && 
	    bpressed[TabNum][0] && bpressed[TabNum][1] && 
	    bpressed[TabNum][2] && bpressed[TabNum][3] && 
	    numpoints[TabNum] > 0)
		gtk_widget_set_sensitive(exportbutton[TabNum], TRUE);
	else gtk_widget_set_sensitive(exportbutton[TabNum], FALSE);
    }

    if (numlastpoints[TabNum]==0) {
	gtk_widget_set_sensitive(remlastbutton[TabNum],FALSE);
	gtk_widget_set_sensitive(remallbutton[TabNum],FALSE);
    } else {
	gtk_widget_set_sensitive(remlastbutton[TabNum],TRUE);
	gtk_widget_set_sensitive(remallbutton[TabNum],TRUE);
    }
}


/****************************************************************/
/* When a button is pressed inside the drawing area this 	*/
/* function is called, it handles axispoints and graphpoints	*/
/* and paints a square in that position.			*/
/****************************************************************/
gint button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GdkModifierType state;
  gint x, y, i, j, TabNum;

    TabNum = GPOINTER_TO_INT(data);

    gdk_window_get_pointer (event->window, &x, &y, &state); 				/* Get pointer state */

    if (event->button == 1) {								/* If button 1 (leftmost) is pressed */
	if (MovePointMode) {
	    for (i=0;i<numpoints[TabNum];i++) {
		if (abs(points[TabNum][i][0]-x) < MOVETRESHOLD &&
		    abs(points[TabNum][i][1]-y) < MOVETRESHOLD) {
		    printf("Moving point %d\n",i);
		}
	    }
	} else {
/* If none of the set axispoint buttons been pressed */
	if (!setxypressed[TabNum][0] && !setxypressed[TabNum][1] && !setxypressed[TabNum][2] && !setxypressed[TabNum][3]) {
	    if (numpoints[TabNum] > MaxPoints[TabNum]-1) {
		i = MaxPoints[TabNum];
		MaxPoints[TabNum] += MAXPOINTS;
		lastpoints[TabNum] = realloc(lastpoints[TabNum],sizeof(gint) * (MaxPoints[TabNum]+4));
		if (lastpoints[TabNum]==NULL) {
		    printf("Error reallocating memory for lastpoints. Exiting.\n");
		    exit(-1);
		}
		points[TabNum] = realloc(points[TabNum],sizeof(gint *) * MaxPoints[TabNum]);
		if (points[TabNum]==NULL) {
		    printf("Error reallocating memory for points. Exiting.\n");
		    exit(-1);
		}
		for (;i<MaxPoints[TabNum];i++) {
		    points[TabNum][i] = malloc(sizeof(gint) * 2);
		    if (points[TabNum][i]==NULL) {
			printf("Error allocating memory for points[%d]. Exiting.\n",i);
			exit(-1);
		    }
		}
	    }
	    points[TabNum][numpoints[TabNum]][0]=x;					/* Save x coordinate */
	    points[TabNum][numpoints[TabNum]][1]=y;					/* Save x coordinate */
	    lastpoints[TabNum][numlastpoints[TabNum]]=numpoints[TabNum];		/* Save index of point */
	    numlastpoints[TabNum]++;							/* Increase lastpoint index */
	    numpoints[TabNum]++;							/* Increase point counter */
	    SetNumPointsEntry(nump_entry[TabNum], numpoints[TabNum]);

	    DrawMarker(drawing_area[TabNum], x, y, 2, colors);
	} else {
	    for (i=0;i<4;i++) if (setxypressed[TabNum][i]) {				/* If the "Set point 1 on x axis" button is pressed */
		axiscoords[TabNum][i][0]=x;						/* Save coordinates */
		axiscoords[TabNum][i][1]=y;
		for (j=0;j<4;j++) if (i!=j) gtk_widget_set_sensitive(setxybutton[TabNum][j],TRUE);
		gtk_widget_set_sensitive(xyentry[TabNum][i],TRUE);			/* Sensitize the entry */
		gtk_editable_set_editable((GtkEditable *) xyentry[TabNum][i],TRUE);
		gtk_widget_grab_focus(xyentry[TabNum][i]);				/* Focus on entry */
		setxypressed[TabNum][i]=FALSE;						/* Mark the button as not pressed */
		bpressed[TabNum][i]=TRUE;						/* Mark that axis point's been set */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setxybutton[TabNum][i]),FALSE); /* Pop up the button */
		lastpoints[TabNum][numlastpoints[TabNum]]=-(i+1);			/* Remember that the points been put out */
		numlastpoints[TabNum]++;						/* Increase index of lastpoints */

		DrawMarker(drawing_area[TabNum], x, y, i/2, colors);			/* Draw marker */
	    }
	}
	SetButtonSensitivity(ViewedTabNum);
	}
    } else if (event->button == 2) {							/* Is the middle button pressed ? */
	for (i=0;i<2;i++) if (!bpressed[TabNum][i]) {
	    axiscoords[TabNum][i][0]=x;
	    axiscoords[TabNum][i][1]=y;
	    for (j=0;j<4;j++) if (i!=j) gtk_widget_set_sensitive(setxybutton[TabNum][j],TRUE);
	    gtk_widget_set_sensitive(xyentry[TabNum][i],TRUE);
	    gtk_editable_set_editable((GtkEditable *) xyentry[TabNum][i],TRUE);
	    gtk_widget_grab_focus(xyentry[TabNum][i]);
	    setxypressed[TabNum][i]=FALSE;
	    bpressed[TabNum][i]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setxybutton[TabNum][i]),FALSE);
	    lastpoints[TabNum][numlastpoints[TabNum]]=-(i+1);
	    numlastpoints[TabNum]++;

	    DrawMarker(drawing_area[TabNum], x, y, 0, colors);
	    break;
	}
    } else if (event->button == 3) {							/* Is the right button pressed ? */
	for (i=2;i<4;i++) if (!bpressed[TabNum][i]) {
	    axiscoords[TabNum][i][0]=x;
	    axiscoords[TabNum][i][1]=y;
	    for (j=0;j<4;j++) if (i!=j) gtk_widget_set_sensitive(setxybutton[TabNum][j],TRUE);
	    gtk_widget_set_sensitive(xyentry[TabNum][i],TRUE);
	    gtk_editable_set_editable((GtkEditable *) xyentry[TabNum][i],TRUE);
	    gtk_widget_grab_focus(xyentry[TabNum][i]);
	    setxypressed[TabNum][i]=FALSE;
	    bpressed[TabNum][i]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setxybutton[TabNum][i]),FALSE);
	    lastpoints[TabNum][numlastpoints[TabNum]]=-(i+1);
	    numlastpoints[TabNum]++;

	    DrawMarker(drawing_area[TabNum], x, y, 1, colors);
	    break;
	}
    }
    SetButtonSensitivity(TabNum);
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
/* This function is called when movement is detected in the	*/
/* drawing area, it captures the coordinates and zoom in om the */
/* position and plots it on the zoom area.			*/
/****************************************************************/
gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  gint x, y, TabNum;
  gchar buf[32];
  GdkModifierType 	state;
  static gboolean 	FirstTime = TRUE;
  static GdkGC 		*mngc;								/* Graphic context */
  static GdkPixbuf	*gpbzoomimage;
  struct PointValue	CalcVal;

    TabNum = GPOINTER_TO_INT(data);

    gdk_window_get_pointer (event->window, &x, &y, &state);				/* Grab mousepointers coordinates */
											/* on drawing area. */
    mngc = gdk_gc_new (zoom_area[TabNum]->window);					/* Create graphics context */

    if (FirstTime) {
	gpbzoomimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, ZOOMPIXSIZE, ZOOMPIXSIZE);
	FirstTime = FALSE;
    }

    if (x>=0 && y>=0 && x<XSize[TabNum] && y<YSize[TabNum]) {

	gdk_pixbuf_composite(gpbimage[TabNum], gpbzoomimage, 0, 0, ZOOMPIXSIZE, 
			     ZOOMPIXSIZE, -x*ZOOMFACTOR + ZOOMPIXSIZE/2, 
			     -y*ZOOMFACTOR + ZOOMPIXSIZE/2, 1.0*ZOOMFACTOR, 
			     1.0*ZOOMFACTOR, GDK_INTERP_BILINEAR, 255);
	gdk_draw_pixbuf(zoom_area[TabNum]->window,zoom_area[TabNum]->style->white_gc,gpbzoomimage,
			0,0,0,0,ZOOMPIXSIZE,ZOOMPIXSIZE,GDK_RGB_DITHER_NONE,0,0);

	DrawMarker(zoom_area[TabNum], ZOOMPIXSIZE/2, ZOOMPIXSIZE/2, 2, colors);		/* Then draw the square in the middle of the zoom area */

	if (valueset[TabNum][0] && valueset[TabNum][1] && valueset[TabNum][2] && valueset[TabNum][3]) {
	    CalcVal = CalcPointValue(x,y,TabNum);

	    sprintf(buf,"%16.10g",CalcVal.Xv);
	    gtk_entry_set_text(GTK_ENTRY(xc_entry[TabNum]),buf);			/* Put out coordinates in entries */
	    sprintf(buf,"%16.10g",CalcVal.Yv);
	    gtk_entry_set_text(GTK_ENTRY(yc_entry[TabNum]),buf);
	    sprintf(buf,"%16.10g",CalcVal.Xerr);
	    gtk_entry_set_text(GTK_ENTRY(xerr_entry[TabNum]),buf);			/* Put out coordinates in entries */
	    sprintf(buf,"%16.10g",CalcVal.Yerr);
	    gtk_entry_set_text(GTK_ENTRY(yerr_entry[TabNum]),buf);
	}
	else {
	    gtk_entry_set_text(GTK_ENTRY(xc_entry[TabNum]),"");				/* Else clear entries */
	    gtk_entry_set_text(GTK_ENTRY(yc_entry[TabNum]),"");
	    gtk_entry_set_text(GTK_ENTRY(xerr_entry[TabNum]),"");
	    gtk_entry_set_text(GTK_ENTRY(yerr_entry[TabNum]),"");
	}
    } else {
	gtk_entry_set_text(GTK_ENTRY(xc_entry[TabNum]),"");				/* Else clear entries */
	gtk_entry_set_text(GTK_ENTRY(yc_entry[TabNum]),"");
	gtk_entry_set_text(GTK_ENTRY(xerr_entry[TabNum]),"");
	gtk_entry_set_text(GTK_ENTRY(yerr_entry[TabNum]),"");
    }
    g_object_unref(mngc);								/* Kill graphics context */
    return TRUE;
}


/****************************************************************/
/* This function is called when the drawing area is exposed, it	*/
/* simply redraws the pixmap on it.				*/
/****************************************************************/
static gint expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  gint i, TabNum;

    TabNum = GPOINTER_TO_INT(data);

    gdk_draw_pixbuf(widget->window,widget->style->white_gc,gpbimage[TabNum],
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    min(event->area.width,XSize[TabNum]), min(event->area.height,YSize[TabNum]),
		    GDK_RGB_DITHER_NONE,0,0);

    for (i=0;i<4;i++) if (bpressed[TabNum][i]) DrawMarker(drawing_area[TabNum], axiscoords[TabNum][i][0], axiscoords[TabNum][i][1], i/2, colors);
    for (i=0;i<numpoints[TabNum];i++) DrawMarker(drawing_area[TabNum], points[TabNum][i][0], points[TabNum][i][1], 2, colors);

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

    if (GTK_TOGGLE_BUTTON (widget)->active) {						/* Is the button pressed on ? */
	setxypressed[ViewedTabNum][index]=TRUE;						/* The button is pressed down */
	for (i=0;i<4;i++) {
	    if (index != i) gtk_widget_set_sensitive(setxybutton[ViewedTabNum][i],FALSE);
	}
	if (bpressed[index]) {								/* If the x axis point is already set */
	    remthis=-(index+1);								/* remove the square */
	    remove_last(widget,NULL);
	}
	bpressed[ViewedTabNum][index]=FALSE;						/* Set x axis point 1 to unset */
    } else {										/* If button is trying to get unpressed */
	if (setxypressed[ViewedTabNum][index]) 
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE); 		/* Set button down */
    }
}


/****************************************************************/
/* Set type of ordering at output of data.			*/
/****************************************************************/
void SetOrdering(GtkWidget *widget, gpointer func_data)
{
    ordering[ViewedTabNum] = GPOINTER_TO_INT (func_data);				/* Set ordering control variable */
}


/****************************************************************/
/****************************************************************/
void SetAction(GtkWidget *widget, gpointer func_data)
{
    Action[ViewedTabNum] = GPOINTER_TO_INT (func_data);
    SetButtonSensitivity(ViewedTabNum);
}


/****************************************************************/
/* Set whether to use error evaluation and printing or not.	*/
/****************************************************************/
void UseErrCB(GtkWidget *widget, gpointer func_data)
{
    UseErrors[ViewedTabNum] = (GTK_TOGGLE_BUTTON (widget)->active);
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
    sscanf(xy_text,"%lf",&realcoords[ViewedTabNum][index]);				/* Convert string to double value and */
											/* store in realcoords[0]. */
    if (logxy[ViewedTabNum][index/2] && realcoords[ViewedTabNum][index] > 0) valueset[ViewedTabNum][index]=TRUE;
    else if (logxy[ViewedTabNum][index/2]) valueset[ViewedTabNum][index]=FALSE;
    else valueset[ViewedTabNum][index]= TRUE; 

    SetButtonSensitivity(ViewedTabNum);
}


/****************************************************************/
/* If all the axispoints has been put out, values for these	*/
/* have been assigned and at least one point has been set on	*/
/* the graph activate the write to file button.			*/
/****************************************************************/
void read_file_entry(GtkWidget *entry, gpointer func_data)
{

  gint TabNum;

    TabNum = GPOINTER_TO_INT (func_data);

    file_name[TabNum] = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
    file_name_length[TabNum] = strlen(file_name[TabNum]);			/* Get length of string */

    if (bpressed[TabNum][0] && bpressed[TabNum][1] && bpressed[TabNum][2] && 
	bpressed[TabNum][3] && valueset[TabNum][0] && valueset[TabNum][1] && 
	valueset[TabNum][2] && valueset[TabNum][3] && numpoints[TabNum] > 0 &&
	file_name_length[TabNum] > 0) {
	gtk_widget_set_sensitive(exportbutton[TabNum],TRUE);
    }
    else gtk_widget_set_sensitive(exportbutton[TabNum],FALSE);

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

    logxy[ViewedTabNum][index] = (GTK_TOGGLE_BUTTON (widget)->active); 			/* If checkbutton is pressed down */
											/* logxy = TRUE else FALSE. */
    if (logxy[ViewedTabNum][index]) {
	if (realcoords[ViewedTabNum][index*2] <= 0) {					/* If a negative value has been insert */
	    valueset[ViewedTabNum][index*2]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(xyentry[ViewedTabNum][index*2]),"");		/* Zero it */
	}
	if (realcoords[ViewedTabNum][index*2+1] <= 0) {					/* If a negative value has been insert */
	    valueset[ViewedTabNum][index*2+1]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(xyentry[ViewedTabNum][index*2+1]),"");		/* Zero it */
        }
    }
}


/****************************************************************/
/* This function removes the last inserted point or the point	*/
/* indexed by remthis (<0).					*/
/****************************************************************/
void remove_last(GtkWidget *widget, gpointer data) 
{
  gint i, j, TabNum;

    TabNum = GPOINTER_TO_INT(data);

/* First redraw the drawing_area with the original image, to clean it. */

    gdk_draw_pixbuf(drawing_area[TabNum]->window,widget->style->white_gc,gpbimage[TabNum],
		    0, 0, 0, 0, XSize[TabNum], YSize[TabNum],GDK_RGB_DITHER_NONE,0,0);

    if (numlastpoints[TabNum]>0) {							/* If points been put out, remove last one */
	if (remthis==0) {								/* If remthis is 0, ignore it.		*/
	    numlastpoints[TabNum]--;
	    for (i=0;i<4;i++) if (lastpoints[TabNum][numlastpoints[TabNum]]==-(i+1)) {	/* If point to be removed is axispoint 1-4 */
		bpressed[TabNum][i]=FALSE;						/* Mark it unpressed.			*/
		gtk_widget_set_sensitive(xyentry[TabNum][i],FALSE);			/* Inactivate entry for point.		*/
		break;
	    }
	    if (i==4) numpoints[TabNum]--;						/* If its none of the X/Y markers then	*/
	    SetNumPointsEntry(nump_entry[TabNum], numpoints[TabNum]);			/* its an ordinary marker, remove it.	 */
	}

	if (numlastpoints[TabNum]>0) {							/* If more than 0 points left, start to redraw */
	    for (i=0;i<numlastpoints[TabNum];i++) {					/* Loop over all remaining points.	*/
		for (j=0;j<4;j++) 
		    if (lastpoints[TabNum][i]==-(j+1) && remthis!=-(j+1)) 
			DrawMarker(drawing_area[TabNum], axiscoords[TabNum][j][0], 
				   axiscoords[TabNum][j][1], j/2, colors);
		if (lastpoints[TabNum][i]>=0) 
		    DrawMarker(drawing_area[TabNum], points[TabNum][lastpoints[TabNum][i]][0], 
			       points[TabNum][lastpoints[TabNum][i]][1], 2, colors);
	    }
	}
    }

    SetButtonSensitivity(TabNum);
    remthis = 0;									/* Reset remthis variable */
}


/****************************************************************/
/* This function sets the proper variables and then calls 	*/
/* remove_last, to remove all points except the axis points.	*/
/****************************************************************/
void remove_all(GtkWidget *widget, gpointer data) 
{
  gint i, j, index, TabNum;

    TabNum = GPOINTER_TO_INT(data);

    if (numlastpoints[TabNum]>0 && numpoints[TabNum]>0) {
	index = 0;
	for (i=0;i<numlastpoints[TabNum];i++) for (j=0;j<4;j++) {			/* Search for axispoints and store them in */
	    if (lastpoints[TabNum][i]==-(j+1)) {					/* lastpoints at the first positions.      */
		lastpoints[TabNum][index] = -(j+1);
		index++;
	    }
	}
	lastpoints[TabNum][index] = 0;

	numlastpoints[TabNum] = index+1;
	numpoints[TabNum] = 1;
	SetNumPointsEntry(nump_entry[TabNum], numpoints[TabNum]);

	remove_last(widget,data);							/* Call remove_last() */
    } else if (numlastpoints[TabNum]>0 && numpoints[TabNum]==0) {
	numlastpoints[TabNum] = 0;							/* Nullify amount of points */
	for (i=0;i<4;i++) {
	    valueset[TabNum][i] = FALSE;
	    bpressed[TabNum][i] = FALSE;
	    gtk_entry_set_text((GtkEntry *) xyentry[TabNum][i], "");
	}
	remove_last(widget,data);							/* Call remove_last() */
    }
    SetButtonSensitivity(TabNum);
}


/****************************************************************/
/* This function handles all of the keypresses done within the	*/
/* main window and handles the  appropriate measures.		*/
/****************************************************************/
gint key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer)
{
  GtkAdjustment *adjustment;
  gdouble adj_val;
  GdkCursor	*cursor;

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
    } else if (event->keyval==GDK_Control_L) {
	if (ViewedTabNum != -1) {
	    cursor = gdk_cursor_new (GDK_CIRCLE);
	    gdk_window_set_cursor (drawing_area[ViewedTabNum]->window, cursor);
	    MovePointMode = TRUE;
	}
    }

  return 0;
}


/****************************************************************/
/****************************************************************/
gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer pointer)
{
  GdkCursor	*cursor;

    if (event->keyval==GDK_Control_L) {
	if (ViewedTabNum != -1) {
	    cursor = gdk_cursor_new (GDK_CROSSHAIR);
	    gdk_window_set_cursor (drawing_area[ViewedTabNum]->window, cursor);
	    MovePointMode = FALSE;
	}
    }

  return 0;
}


/****************************************************************/
/* This function loads the image, and inserts it into the tab	*/
/* and sets up all of the different signals associated with it.	*/
/****************************************************************/
gint InsertImage(char *filename, gdouble Scale, gdouble maxX, gdouble maxY, gint TabNum) {

  gint 		newX, newY;
  gdouble 	mScale;
  GdkPixbuf	*loadgpbimage;
  GdkCursor	*cursor;
  GtkWidget	*dialog;
  gchar 	buf[256];								/* Text buffer for window title */

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
    XSize[TabNum] = gdk_pixbuf_get_width(loadgpbimage);					/* Get image width */
    YSize[TabNum] = gdk_pixbuf_get_height(loadgpbimage);				/* Get image height */

    sprintf(buf, Window_Title, filename);						/* Print window title in buffer */
    gtk_window_set_title (GTK_WINDOW (window), buf);					/* Set window title */

    mScale = -1;
    if (maxX != -1 && maxY != -1) {
	if (XSize[TabNum] > maxX) {
	    mScale = (double) maxX/XSize[TabNum];
	}
	if (YSize[TabNum] > maxY && (double) maxY/YSize[TabNum] < mScale) mScale = (double) maxY/YSize[TabNum];
    }

    if (Scale == -1 && mScale != -1) Scale = mScale;

    if (Scale != -1) {
	newX = XSize[TabNum]*Scale;
	newY = YSize[TabNum]*Scale;
	gpbimage[TabNum] = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, newX, newY);
	gdk_pixbuf_composite(loadgpbimage, gpbimage[TabNum], 0, 0, newX, newY,
			     0, 0, Scale, Scale, GDK_INTERP_BILINEAR, 255);
	g_object_unref(loadgpbimage);
    }
    else gpbimage[TabNum] = loadgpbimage;

    XSize[TabNum] = gdk_pixbuf_get_width(gpbimage[TabNum]);				/* Get image width */
    YSize[TabNum] = gdk_pixbuf_get_height(gpbimage[TabNum]);				/* Get image height */

    drawing_area[TabNum] = gtk_drawing_area_new ();					/* Create new drawing area */
    gtk_widget_set_size_request (drawing_area[TabNum], XSize[TabNum], YSize[TabNum]);

    g_signal_connect (G_OBJECT (drawing_area[TabNum]), "expose_event",			/* Connect drawing area to */
              G_CALLBACK (expose_event), GINT_TO_POINTER (TabNum));			/* expose_event. */

    g_signal_connect (G_OBJECT (drawing_area[TabNum]), "configure_event",		/* Connect drawing area to */
              G_CALLBACK (configure_event), GINT_TO_POINTER (TabNum));			/* configure_event. */

    g_signal_connect (G_OBJECT (drawing_area[TabNum]), "button_press_event",		/* Connect drawing area to */
              G_CALLBACK (button_press_event), GINT_TO_POINTER (TabNum));		/* button_press_event. */

    g_signal_connect (G_OBJECT (drawing_area[TabNum]), "button_release_event",		/* Connect drawing area to */
              G_CALLBACK (button_release_event), GINT_TO_POINTER (TabNum));		/* button_release_event */

    g_signal_connect (G_OBJECT (drawing_area[TabNum]), "motion_notify_event",		/* Connect drawing area to */
              G_CALLBACK (motion_notify_event), GINT_TO_POINTER (TabNum));		/* motion_notify_event. */

    gtk_widget_set_events (drawing_area[TabNum], GDK_EXPOSURE_MASK |			/* Set the events active */
			   GDK_BUTTON_PRESS_MASK | 
			   GDK_BUTTON_RELEASE_MASK |
			   GDK_POINTER_MOTION_MASK | 
			   GDK_POINTER_MOTION_HINT_MASK);

    gtk_container_add((GtkContainer *) drawing_area_alignment, drawing_area[TabNum]);

    gtk_widget_show(drawing_area[TabNum]);

    cursor = gdk_cursor_new (GDK_CROSSHAIR);
    gdk_window_set_cursor (drawing_area[TabNum]->window, cursor);
 
    return 0;
}


/****************************************************************/
/* This callback is called when the file - exit menuoptioned is */
/* selected.							*/
/****************************************************************/
GCallback menu_file_exit(void)
{
    close_application(NULL,NULL,NULL);

  return NULL;
}

/****************************************************************/
/* This callback sets up the thumbnail in the Fileopen dialog.	*/
/****************************************************************/
static void update_preview_cb (GtkFileChooser *file_chooser, gpointer data) {
  GtkWidget *preview;
  char *filename;
  GdkPixbuf *pixbuf;
  gboolean have_preview;

    preview = GTK_WIDGET (data);
    filename = gtk_file_chooser_get_preview_filename (file_chooser);

    pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 128, 128, NULL);
    have_preview = (pixbuf != NULL);
    g_free (filename);

    gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
    if (pixbuf) gdk_pixbuf_unref (pixbuf);

    gtk_file_chooser_set_preview_widget_active (file_chooser, have_preview);
}


/****************************************************************/
/* This function sets up a new tab, sets up all of the widgets 	*/
/* needed.							*/
/****************************************************************/
gint SetupNewTab(char *filename, gdouble Scale, gdouble maxX, gdouble maxY, gboolean UsePreSetCoords)
{
  GtkWidget 	*table;									/* GTK table/box variables for packing */
  GtkWidget	*tophbox, *bottomhbox;
  GtkWidget	*trvbox, *tlvbox, *brvbox, *blvbox, *subvbox;
  GtkWidget 	*xy_label[4];								/* Labels for texts in window */
  GtkWidget 	*logcheckb[2];								/* Logarithmic checkbuttons */
  GtkWidget 	*nump_label, *ScrollWindow;						/* Various widgets */
  GtkWidget	*APlabel, *PIlabel, *ZAlabel, *Llabel, *tab_label;
  GtkWidget 	*alignment, *fixed;
  GtkWidget 	*x_label, *y_label, *tmplabel;
  GtkWidget	*ordercheckb[3], *UseErrCheckB, *actioncheckb[2];
  GtkWidget	*Olabel, *Elabel, *Alabel;
  GSList 	*group;
  GtkWidget	*dialog;

  gchar 	buf[256], buf2[256];
  gint 		i, TabNum;
  gboolean	FileInCwd;
  static gint	NumberOfTabs=0;

    if (NumberOfTabs == MAXNUMTABS-1) {
	dialog = gtk_message_dialog_new (GTK_WINDOW(window),				/* Notify user of the error */
                                  GTK_DIALOG_DESTROY_WITH_PARENT,			/* with a dialog */
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Cannot open more tabs, maximum number reached (%d)",
                                  MAXNUMTABS);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (dialog);
	return -1;
    }

    NumberOfTabs++;

    strncpy(buf2,filename,256);
    if (strcmp(dirname(buf2),getcwd(buf,256)) == 0) {
	tab_label = gtk_label_new(basename(filename));
	FileInCwd = TRUE;
    } else {
	tab_label = gtk_label_new(filename);
	FileInCwd = FALSE;
    }

    table = gtk_table_new(1, 2 ,FALSE);							/* Create table */
    gtk_container_set_border_width (GTK_CONTAINER (table), WINDOW_BORDER);
    gtk_table_set_row_spacings(GTK_TABLE(table), SECT_SEP);				/* Set spacings */
    gtk_table_set_col_spacings(GTK_TABLE(table), 0);
    TabNum = gtk_notebook_append_page((GtkNotebook *) mainnotebook, 
				      table, tab_label);
    if (TabNum == -1) {
	return -1;
    }

/* Init datastructures */

    bpressed[TabNum][0] = FALSE;
    bpressed[TabNum][1] = FALSE;
    bpressed[TabNum][2] = FALSE;
    bpressed[TabNum][3] = FALSE;

    valueset[TabNum][0] = FALSE;
    valueset[TabNum][1] = FALSE;
    valueset[TabNum][2] = FALSE;
    valueset[TabNum][3] = FALSE;

    numpoints[TabNum] = 0;
    numlastpoints[TabNum] = 0;
    ordering[TabNum] = 0;

    lastpoints[TabNum] = (gint *) malloc(sizeof(gint) * (MaxPoints[TabNum]+4));
    if (lastpoints[TabNum]==NULL) {
	printf("Error allocating memory for lastpoints. Exiting.\n");
	return -1;
    }
    points[TabNum] = (void *) malloc(sizeof(gint *) * MaxPoints[TabNum]);
    if (points[TabNum]==NULL) {
	printf("Error allocating memory for points. Exiting.\n");
	return -1;
    }
    for (i=0;i<MaxPoints[TabNum];i++) {
	points[TabNum][i] = (gint *) malloc(sizeof(gint) * 2);
	if (points[TabNum][i]==NULL) {
	    printf("Error allocating memory for points[%d]. Exiting.\n",i);
	    return -1;
	}
    }

    tooltip = gtk_tooltips_new();

    for(i=0;i<4;i++) {
	xyentry[TabNum][i] = gtk_entry_new();  						/* Create text entry */
	gtk_entry_set_max_length (GTK_ENTRY (xyentry[TabNum][i]), 20);
	gtk_editable_set_editable((GtkEditable *) xyentry[TabNum][i],FALSE);
	gtk_widget_set_sensitive(xyentry[TabNum][i],FALSE);				/* Inactivate it */
	g_signal_connect (G_OBJECT (xyentry[TabNum][i]), "changed",			/* Init the entry to call */
			  G_CALLBACK (read_xy_entry), GINT_TO_POINTER (i));		/* read_x1_entry whenever */
	gtk_tooltips_set_tip (tooltip,xyentry[TabNum][i],entryxytt[i],entryxytt[i]);
    }

    x_label = gtk_label_new(x_string);
    y_label = gtk_label_new(y_string);
    xc_entry[TabNum] = gtk_entry_new();							/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (xc_entry[TabNum]), 16);
    gtk_editable_set_editable((GtkEditable *) xc_entry[TabNum],FALSE);
    yc_entry[TabNum] = gtk_entry_new();							/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (yc_entry[TabNum]), 16);
    gtk_editable_set_editable((GtkEditable *) yc_entry[TabNum],FALSE);

    pm_label = gtk_label_new(pm_string);
    pm_label2 = gtk_label_new(pm_string);
    xerr_entry[TabNum] = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (xerr_entry[TabNum]), 16);
    gtk_editable_set_editable((GtkEditable *) xerr_entry[TabNum],FALSE);
    yerr_entry[TabNum] = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (yerr_entry[TabNum]), 16);
    gtk_editable_set_editable((GtkEditable *) yerr_entry[TabNum],FALSE);

    nump_label = gtk_label_new(nump_string);
    nump_entry[TabNum] = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (nump_entry[TabNum]), 10);
    gtk_editable_set_editable((GtkEditable *) nump_entry[TabNum],FALSE);
    SetNumPointsEntry(nump_entry[TabNum], numpoints[TabNum]);

    zoom_area[TabNum] = gtk_drawing_area_new ();					/* Create new drawing area */
    gtk_widget_set_size_request (zoom_area[TabNum], ZOOMPIXSIZE, ZOOMPIXSIZE);

    setcolors(&colors);

    for (i=0;i<4;i++) {
	xy_label[i] = gtk_label_new(NULL);
	gtk_label_set_markup((GtkLabel *) xy_label[i], xy_label_text[i]);
    }

    for (i=0;i<4;i++) {
	tmplabel = gtk_label_new(NULL);
	gtk_label_set_markup_with_mnemonic((GtkLabel *) tmplabel, setxylabel[i]);
	setxybutton[TabNum][i] = gtk_toggle_button_new();				/* Create button */
	gtk_container_add((GtkContainer *) setxybutton[TabNum][i], tmplabel);
	g_signal_connect (G_OBJECT (setxybutton[TabNum][i]), "toggled",			/* Connect button */
			  G_CALLBACK (toggle_xy), GINT_TO_POINTER (i));
	gtk_tooltips_set_tip (tooltip,setxybutton[TabNum][i],setxytts[i],setxytt[i]);
    }

    remlastbutton[TabNum] = gtk_button_new_with_mnemonic (RemLastBLabel);		/* Create button */
    g_signal_connect (G_OBJECT (remlastbutton[TabNum]), "clicked",			/* Connect button */
                  G_CALLBACK (remove_last), GINT_TO_POINTER (TabNum));
    gtk_widget_set_sensitive(remlastbutton[TabNum],FALSE);
    gtk_tooltips_set_tip (tooltip,remlastbutton[TabNum],removeltt,removeltt);

    remallbutton[TabNum] = gtk_button_new_with_mnemonic (RemAllBLabel);			/* Create button */
    g_signal_connect (G_OBJECT (remallbutton[TabNum]), "clicked",			/* Connect button */
                  G_CALLBACK (remove_all), GINT_TO_POINTER (TabNum));
    gtk_widget_set_sensitive(remallbutton[TabNum],FALSE);
    gtk_tooltips_set_tip (tooltip,remallbutton[TabNum],removeatts,removeatt);

    for (i=0;i<2;i++) {
	logcheckb[i] = gtk_check_button_new_with_mnemonic(loglabel[i]);			/* Create check button */
	g_signal_connect (G_OBJECT (logcheckb[i]), "toggled",				/* Connect button */
			  G_CALLBACK (islogxy), GINT_TO_POINTER (i));
	gtk_tooltips_set_tip (tooltip,logcheckb[i],logxytt[i],logxytt[i]);
	gtk_toggle_button_set_active (( GtkToggleButton *) logcheckb[i], logxy[TabNum][i]);
    }

    tophbox = gtk_hbox_new (FALSE, SECT_SEP);
    alignment = gtk_alignment_new (0,0,0,0);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 5, 0, 0, 0);
    gtk_container_add((GtkContainer *) alignment, tophbox);

    bottomhbox = gtk_hbox_new (FALSE, SECT_SEP);
    alignment = gtk_alignment_new (0, 0, 1, 1);
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
	gtk_table_attach(GTK_TABLE(table), setxybutton[TabNum][i], 0, 1, i, i+1, 5, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), xy_label[i], 1, 2, i, i+1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), xyentry[TabNum][i], 2, 3, i, i+1, 0, 0, 0, 0);
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
    gtk_table_attach(GTK_TABLE(table), xc_entry[TabNum], 1, 2, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), pm_label, 2, 3, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), xerr_entry[TabNum], 3, 4, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), y_label, 0, 1, 1, 2, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), yc_entry[TabNum], 1, 2, 1, 2, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), pm_label2, 2, 3, 1, 2, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), yerr_entry[TabNum], 3, 4, 1, 2, 0, 0, 0, 0);

    table = gtk_table_new(3, 1 ,FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 6);
    gtk_table_set_col_spacings(GTK_TABLE(table), 6);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, table, FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (trvbox), fixed, FALSE, FALSE, 0);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, nump_label);
    gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table), nump_entry[TabNum], 1, 2, 0, 1, 0, 0, 0, 0);

    blvbox = gtk_vbox_new (FALSE, GROUP_SEP);
    gtk_box_pack_start (GTK_BOX (bottomhbox), blvbox, FALSE, FALSE, ELEM_SEP);

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), remlastbutton[TabNum], FALSE, FALSE, 0);	/* Pack button in vert. box */
    gtk_box_pack_start (GTK_BOX (subvbox), remallbutton[TabNum], FALSE, FALSE, 0);		/* Pack button in vert. box */

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    zoomareabox[TabNum] = subvbox;
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    ZAlabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (ZAlabel), ZAheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, ZAlabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, zoom_area[TabNum], FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    logbox[TabNum] = subvbox;
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    Llabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Llabel), Lheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Llabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<2;i++) {
	fixed = gtk_fixed_new ();
	gtk_fixed_put((GtkFixed *) fixed, logcheckb[i], FRAME_INDENT, 0);
	gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);			/* Pack checkbutton in vert. box */
    }

    group = NULL;
    for (i=0;i<ORDERBNUM;i++) {
	ordercheckb[i] = gtk_radio_button_new_with_label (group, orderlabel[i]);	/* Create radio button */
	g_signal_connect (G_OBJECT (ordercheckb[i]), "toggled",				/* Connect button */
			  G_CALLBACK (SetOrdering), GINT_TO_POINTER (i));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (ordercheckb[i]));		/* Get buttons group */
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ordercheckb[0]), TRUE);		/* Set no ordering button active */

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    oppropbox[TabNum] = subvbox;
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    Olabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Olabel), Oheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Olabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    for (i=0;i<ORDERBNUM;i++) {
	fixed = gtk_fixed_new ();
	gtk_fixed_put((GtkFixed *) fixed, ordercheckb[i], FRAME_INDENT, 0);
	gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);			/* Pack radiobutton in vert. box */
    }

    UseErrCheckB = gtk_check_button_new_with_mnemonic(PrintErrCBLabel);
    g_signal_connect (G_OBJECT (UseErrCheckB), "toggled",
		      G_CALLBACK (UseErrCB), GINT_TO_POINTER (TabNum));
    gtk_tooltips_set_tip (tooltip,UseErrCheckB,uetts,uett);
    gtk_toggle_button_set_active (( GtkToggleButton *) UseErrCheckB, UseErrors[TabNum]);

    Elabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (Elabel), Eheader);
    alignment = gtk_alignment_new (0, 1, 0, 0);
    gtk_container_add((GtkContainer *) alignment, Elabel);
    gtk_box_pack_start (GTK_BOX (subvbox), alignment, FALSE, FALSE, 0);
    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, UseErrCheckB, FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);

    subvbox = gtk_vbox_new (FALSE, ELEM_SEP);
    gtk_box_pack_start (GTK_BOX (blvbox), subvbox, FALSE, FALSE, 0);
    group = NULL;
    for (i=0;i<ACTIONBNUM;i++) {
	actioncheckb[i] = gtk_radio_button_new_with_label (group, actionlabel[i]);	/* Create radio button */
	g_signal_connect (G_OBJECT (actioncheckb[i]), "toggled",			/* Connect button */
			  G_CALLBACK (SetAction), GINT_TO_POINTER (i));
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (actioncheckb[i]));	/* Get buttons group */
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (actioncheckb[0]), TRUE);		/* Set no ordering button active */

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

    file_entry[TabNum] = gtk_entry_new();						/* Create text entry */
    gtk_entry_set_max_length (GTK_ENTRY (file_entry[TabNum]), 256);
    gtk_editable_set_editable((GtkEditable *) file_entry[TabNum],TRUE);
    g_signal_connect (G_OBJECT (file_entry[TabNum]), "changed",				/* Init the entry to call */
                    G_CALLBACK (read_file_entry), GINT_TO_POINTER (TabNum));
    gtk_tooltips_set_tip (tooltip,file_entry[TabNum],filenamett,filenamett);

    if (FileInCwd) {
	snprintf(buf2, 256, "%s.dat",basename(filename));
	strncpy(FileNames[TabNum], basename(filename), 256);
    } else {
	snprintf(buf2, 256, "%s.dat",filename);
	strncpy(FileNames[TabNum], filename, 256);
    }

    snprintf(buf, 256, Window_Title, FileNames[TabNum]);				/* Print window title in buffer */
    gtk_window_set_title (GTK_WINDOW (window), buf);                    		/* Set window title */

    fixed = gtk_fixed_new ();
    gtk_fixed_put((GtkFixed *) fixed, file_entry[TabNum], FRAME_INDENT, 0);
    gtk_box_pack_start (GTK_BOX (subvbox), fixed, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(file_entry[TabNum],FALSE);

    exportbutton[TabNum] = gtk_button_new_with_mnemonic (PrintBLabel);			/* Create button */

    gtk_box_pack_start (GTK_BOX (subvbox), exportbutton[TabNum], FALSE, FALSE, 0);
    gtk_widget_set_sensitive(exportbutton[TabNum],FALSE);

    g_signal_connect (G_OBJECT (exportbutton[TabNum]), "clicked",
                  G_CALLBACK (print_results), GINT_TO_POINTER(FALSE));
    gtk_tooltips_set_tip (tooltip,exportbutton[TabNum],printrestt,printrestt);

    brvbox = gtk_vbox_new (FALSE, GROUP_SEP);
    gtk_box_pack_start (GTK_BOX (bottomhbox), brvbox, TRUE, TRUE, 0);

    gtk_entry_set_text(GTK_ENTRY (file_entry[TabNum]), buf2);				/* Set text of text entry to filename */

    ScrollWindow = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy ((GtkScrolledWindow *) ScrollWindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    ViewPort = gtk_viewport_new(NULL,NULL);
    gtk_box_pack_start (GTK_BOX (brvbox), ScrollWindow, TRUE, TRUE, 0);
    drawing_area_alignment = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (ViewPort), drawing_area_alignment);
    gtk_container_add (GTK_CONTAINER (ScrollWindow), ViewPort);

    gtk_widget_show_all(window);


    gtk_notebook_set_current_page((GtkNotebook *) mainnotebook, TabNum);

    if (InsertImage(filename, Scale, maxX, maxY, TabNum) == -1) {
	gtk_notebook_remove_page((GtkNotebook *) mainnotebook, TabNum);
	return -1;
    }

    if (UsePreSetCoords) {
	axiscoords[TabNum][0][0] = 0;
	axiscoords[TabNum][0][1] = YSize[TabNum]-1;
	axiscoords[TabNum][1][0] = XSize[TabNum]-1;
	axiscoords[TabNum][1][1] = YSize[TabNum]-1;
	axiscoords[TabNum][2][0] = 0;
	axiscoords[TabNum][2][1] = YSize[TabNum]-1;
	axiscoords[TabNum][3][0] = 0;
	axiscoords[TabNum][3][1] = 0;
	for (i=0;i<4;i++) {
	    gtk_widget_set_sensitive(xyentry[TabNum][i],TRUE);
	    gtk_editable_set_editable((GtkEditable *) xyentry[TabNum][i],TRUE);
	    sprintf(buf,"%lf",realcoords[TabNum][i]);
	    gtk_entry_set_text((GtkEntry *) xyentry[TabNum][i], buf);
	    lastpoints[TabNum][numlastpoints[TabNum]]=-(i+1);
	    numlastpoints[TabNum]++;
	    valueset[TabNum][i] = TRUE;
	    bpressed[TabNum][i] = TRUE;
	    setxypressed[TabNum][i]=FALSE;
	}
	gtk_widget_set_sensitive(exportbutton[TabNum],TRUE);
    }

    gtk_action_group_set_sensitive(tab_action_group, TRUE);
    NoteBookNumPages++;

    if (HideZoomArea) for (i=0;i<MAXNUMTABS;i++) if (zoomareabox[i] != NULL) gtk_widget_hide(zoomareabox[i]);
    if (HideLog) for (i=0;i<MAXNUMTABS;i++) if (logbox[i] != NULL) gtk_widget_hide(logbox[i]);
    if (HideOpProp) for (i=0;i<MAXNUMTABS;i++) if (oppropbox[i] != NULL) gtk_widget_hide(oppropbox[i]);

  return 0;
}


/****************************************************************/
/****************************************************************/
void drag_data_received(GtkWidget *widget,
                              GdkDragContext *drag_context,
                              gint x, gint y,
                              GtkSelectionData *data,
                              guint info,
                              guint event_time,
                              gpointer user_data)
{
  gchar 	filename[256], *c;
  gint		i;
  GtkWidget	*dialog;

//printf("DnD received\n");

    switch (info) {
	case URI_LIST: {
//	    printf("Received uri : %s\n", (gchar *) data->data);
	    if ((c = strstr((gchar *) data->data, URI_IDENTIFIER)) == NULL) {
		dialog = gtk_message_dialog_new (GTK_WINDOW(window),			/* Notify user of the error */
                	                  	 GTK_DIALOG_DESTROY_WITH_PARENT,	/* with a dialog */
	                                  	 GTK_MESSAGE_ERROR,
	                                  	 GTK_BUTTONS_CLOSE,
	                                  	 "Cannot extract filename from uri '%s'",
       		                          	 filename);
 		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		break;
	    }
	    strncpy(filename,&(c[strlen(URI_IDENTIFIER)]),256);
	    for (i=0;i<strlen(filename);i++) if (filename[i] == '\n') filename[i] = '\0';
//printf("Opening file >%s<\n",filename);
	    SetupNewTab(filename, 1.0, -1, -1, FALSE);
	    break;
	}
	case JPEG_DATA:
	case PNG_DATA: {
	    printf("Received drag-and-drop jpeg_data or png_data\n");
            GError *error = NULL;
            GdkPixbufLoader *loader = gdk_pixbuf_loader_new_with_mime_type(gdk_atom_name(data->type), &error);
            if (loader) {
            	error = NULL;
            	if (gdk_pixbuf_loader_write( loader, data->data, data->length, &error)) {
                    GdkPixbuf *pbuf = gdk_pixbuf_loader_get_pixbuf(loader);
                    if ( pbuf ) {
                        int width = gdk_pixbuf_get_width(pbuf);
                        int height = gdk_pixbuf_get_height(pbuf);
                        printf("Received image of size %d x %d\n", width, height);  // Print debugging information
/*                        snprintf(tmp, sizeof(tmp), "%d", width);
                        newImage->setAttribute("width", tmp);
 
                        snprintf(tmp, sizeof(tmp), "%d", height);
                        newImage->setAttribute("height", tmp); */
                    }
                }
            }
	    break;
	}
	case APP_X_COLOR: {
	    printf("Received drag-and-drop app-x-color\n");
	    break;
	}
    }
    gtk_drag_finish (drag_context, TRUE, FALSE, event_time);
}


/****************************************************************/
/* This callback handles the file - open dialog.		*/
/****************************************************************/
GCallback menu_file_open(void)
{
  GtkWidget *dialog, *scalespinbutton, *hboxextra, *scalelabel;
  GtkImage *preview;
  GtkAdjustment *scaleadj;
  GtkFileFilter *filefilter;

    dialog = gtk_file_chooser_dialog_new ("Open File",
				          GTK_WINDOW (window),
				          GTK_FILE_CHOOSER_ACTION_OPEN,
				          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				          NULL);
    
// Set filtering of files to open to filetypes gdk_pixbuf can handle
    filefilter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(filefilter);
    gtk_file_chooser_set_filter((GtkFileChooser *) dialog, (GtkFileFilter *) filefilter);


    hboxextra = gtk_hbox_new(FALSE, ELEM_SEP);

    scalelabel = gtk_label_new(scale_string);

    scaleadj = (GtkAdjustment *) gtk_adjustment_new(1, 0.1, 100, 0.1, 0.1, 1);
    scalespinbutton = gtk_spin_button_new(scaleadj, 0.1, 1);

    gtk_box_pack_start (GTK_BOX (hboxextra), scalelabel, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hboxextra), scalespinbutton, FALSE, FALSE, 0);

    gtk_file_chooser_set_extra_widget((GtkFileChooser *) dialog, hboxextra);

    gtk_widget_show(hboxextra);
    gtk_widget_show(scalelabel);
    gtk_widget_show(scalespinbutton);

    preview = (GtkImage *) gtk_image_new ();
    gtk_file_chooser_set_preview_widget ((GtkFileChooser *) dialog, (GtkWidget *) preview);
    g_signal_connect (dialog, "update-preview",
		      G_CALLBACK (update_preview_cb), preview);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
	char *filename;

    	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	SetupNewTab(filename, gtk_spin_button_get_value((GtkSpinButton *) scalespinbutton), -1, -1, FALSE);

    	g_free (filename);
    }

    gtk_widget_destroy (dialog);

    return NULL;
}


/****************************************************************/
/* This function destroys a dialog.				*/
/****************************************************************/
void dialog_destroy( GtkWidget *widget, gpointer data)
{
    gtk_grab_remove(GTK_WIDGET(widget));
}


/****************************************************************/
/* This function closes a dialog.				*/
/****************************************************************/
void dialog_close( GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}

 
/****************************************************************/
/* This Callback generates the help - about dialog.		*/
/****************************************************************/
GCallback menu_help_about(void)
{
  gchar *authors[] = AUTHORS;

    gtk_show_about_dialog((GtkWindow *) window, 
	"authors", authors, 
	"comments", COMMENTS,
	"copyright", COPYRIGHT,
	"license", LICENSE,
	"name", PROGNAME,
	"version", VERSION,
	"website", HOMEPAGEURL,
	"website-label", HOMEPAGELABEL,
	NULL);

  return NULL;
}


/****************************************************************/
/* This function is called when a tab is closed. It removes the	*/
/* page from the notebook, all widgets within the page are	*/
/* destroyed.							*/
/****************************************************************/
GCallback menu_tab_close(void)
{
    gtk_notebook_remove_page((GtkNotebook *) mainnotebook, ViewedTabNum);		/* This appearently takes care of everything */

    logxy[ViewedTabNum][0] = FALSE;
    logxy[ViewedTabNum][1] = FALSE;

    zoomareabox[ViewedTabNum] = NULL;
    logbox[ViewedTabNum] = NULL;
    oppropbox[ViewedTabNum] = NULL;

    NoteBookNumPages--;

    if (NoteBookNumPages == 0) gtk_action_group_set_sensitive(tab_action_group, FALSE);

  return NULL;
}

/****************************************************************/
/* This callback handles the fullscreen toggling.		*/
/****************************************************************/
GCallback full_screen_action_callback(GtkWidget *widget, gpointer func_data)
{
    if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(widget))) {
	gtk_window_fullscreen(GTK_WINDOW (window));
	WinFullScreen = TRUE;
    } else {
	gtk_window_unfullscreen(GTK_WINDOW (window));
	WinFullScreen = FALSE;
    }
  return NULL;
}

/****************************************************************/
/* This callback handles the hide zoom area toggling.		*/
/****************************************************************/
GCallback hide_zoom_area_callback(GtkWidget *widget, gpointer func_data)
{
  int i;

    if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(widget))) {
        for (i=0;i<MAXNUMTABS;i++) if (zoomareabox[i] != NULL) gtk_widget_hide(zoomareabox[i]);
	HideZoomArea = TRUE;
    } else {
        for (i=0;i<MAXNUMTABS;i++) if (zoomareabox[i] != NULL) gtk_widget_show(zoomareabox[i]);
	HideZoomArea = FALSE;
    }
  return NULL;
}

/****************************************************************/
/* This callback handles the hide axis settings toggling.	*/
/****************************************************************/
GCallback hide_axis_settings_callback(GtkWidget *widget, gpointer func_data)
{
  int i;

    if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(widget))) {
        for (i=0;i<MAXNUMTABS;i++) if (logbox[i] != NULL) gtk_widget_hide(logbox[i]);
	HideLog = TRUE;
    } else {
        for (i=0;i<MAXNUMTABS;i++) if (logbox[i] != NULL) gtk_widget_show(logbox[i]);
	HideLog = FALSE;
    }
  return NULL;
}

/****************************************************************/
/* This callback handles the hide output properties toggling.	*/
/****************************************************************/
GCallback hide_output_prop_callback(GtkWidget *widget, gpointer func_data)
{
  int i;

    if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(widget))) {
        for (i=0;i<MAXNUMTABS;i++) if (oppropbox[i] != NULL) gtk_widget_hide(oppropbox[i]);
	HideOpProp = TRUE;
    } else {
        for (i=0;i<MAXNUMTABS;i++) if (oppropbox[i] != NULL) gtk_widget_show(oppropbox[i]);
	HideOpProp = FALSE;
    }
  return NULL;
}

/****************************************************************/
/* This callback is called when the notebook page is changed.	*/
/* It sets up the ViewedTabNum value as well as the title of	*/
/* the window to match the image currently viewed.		*/
/****************************************************************/
GCallback NoteBookTabChange(GtkNotebook *notebook, GtkNotebookPage *page, 
			    guint page_num, gpointer user_data)
{
  gchar		buf[256];

    ViewedTabNum = page_num;
    sprintf(buf, Window_Title, FileNames[ViewedTabNum]);                        	/* Print window title in buffer */
    gtk_window_set_title (GTK_WINDOW (window), buf);                    		/* Set window title */

  return NULL;
}


/****************************************************************/
/* This is the main function, this function gets called when	*/
/* the program is executed. It allocates the necessary work-	*/
/* spaces and initialized the main window and its widgets.	*/
/****************************************************************/
int main (int argc, char **argv)
{
  gint 		FileIndex[MAXNUMTABS], NumFiles = 0, i, maxX, maxY;
  gdouble 	Scale;
  gboolean	UsePreSetCoords, UseError, Uselogxy[2];
  gdouble	TempCoords[4];

  GtkWidget	*mainvbox;

  GtkWidget *menubar;
  GtkActionGroup *action_group;
  GtkUIManager *ui_manager;
  GtkAccelGroup *accel_group;
  GError *error;

#include "vardefs.h"

    gtk_init (&argc, &argv);								/* Init GTK */

    if (argc > 1) if (strcmp(argv[1],"-h")==0 ||					/* If no parameters given, -h or --help */
	strcmp(argv[1],"--help")==0) {
	printf("%s",HelpText);								/* Print help */
	exit(0);									/* and exit */
    }

    maxX = -1;
    maxY = -1;
    Scale = -1;
   UseError = FALSE;
    UsePreSetCoords = FALSE;
    Uselogxy[0] = FALSE;
    Uselogxy[1] = FALSE;
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
	    } else if (strcmp(argv[i],"-errors")==0) {
		UseError = TRUE;
	    } else if (strcmp(argv[i],"-lnx")==0) {
		Uselogxy[0] = TRUE;
	    } else if (strcmp(argv[i],"-lny")==0) {
		Uselogxy[1] = TRUE;
	    } else if (strcmp(argv[i],"-max")==0) {
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
	    } else if (strcmp(argv[i],"-coords")==0) {
		UsePreSetCoords = TRUE;
		if (argc-i < 5) {
		    printf("Too few parameters for -coords\n");
		    exit(0);
		}
		if (sscanf(argv[i+1],"%lf", &TempCoords[0])!=1) {
		    printf("-max first parameter in invalid form !\n");
		    exit(0);
		}
		if (sscanf(argv[i+2],"%lf", &TempCoords[1])!=1) {
		    printf("-max second parameter in invalid form !\n");
		    exit(0);
		} 
		if (sscanf(argv[i+3],"%lf", &TempCoords[2])!=1) {
		    printf("-max third parameter in invalid form !\n");
		    exit(0);
		} 
		if (sscanf(argv[i+4],"%lf", &TempCoords[3])!=1) {
		    printf("-max fourth parameter in invalid form !\n");
		    exit(0);
		}
		i+=4;
		if (i >= argc) break;
/*	    } else if (strcmp(argv[i],"-hidelog")==0) {
		HideLog = TRUE;
	    } else if (strcmp(argv[i],"-hideza")==0) {
		HideZoomArea = TRUE;
	    } else if (strcmp(argv[i],"-hideop")==0) {
		HideOpProp = TRUE; */
	    } else {
		printf("Unknown parameter : %s\n", argv[i]);
		exit(0);
	    }
	    continue;
	} else {
	    FileIndex[NumFiles] = i;
	    NumFiles++;
	}
    }


    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);					/* Create window */
    gtk_window_set_default_size((GtkWindow *) window, 640, 480);
    gtk_window_set_title(GTK_WINDOW (window), Window_Title_NoneOpen);			/* Set window title */
    gtk_window_set_policy(GTK_WINDOW (window), FALSE, FALSE, TRUE);
    gtk_window_set_resizable(GTK_WINDOW (window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER (window), 0);				/* Set borders in window */
    mainvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add( GTK_CONTAINER(window), mainvbox);

    g_signal_connect(G_OBJECT (window), "delete_event",					/* Init delete event of window */
                        G_CALLBACK (close_application), NULL);

    gtk_drag_dest_set(window, GTK_DEST_DEFAULT_ALL, ui_drop_target_entries, NUM_IMAGE_DATA, (GDK_ACTION_COPY | GDK_ACTION_MOVE));
    g_signal_connect(G_OBJECT (window), "drag-data-received",				/* Drag and drop catch */
                        G_CALLBACK (drag_data_received), NULL);

/* Create menues */
    action_group = gtk_action_group_new("MenuActions");
    gtk_action_group_add_actions(action_group, entries, G_N_ELEMENTS (entries), window);
    gtk_action_group_add_toggle_actions(action_group, toggle_entries, G_N_ELEMENTS (toggle_entries), window);
    tab_action_group = gtk_action_group_new("TabActions");
    gtk_action_group_add_actions(tab_action_group, closeaction, G_N_ELEMENTS (closeaction), window);
    gtk_action_group_set_sensitive(tab_action_group, FALSE);

    ui_manager = gtk_ui_manager_new();
    gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);
    gtk_ui_manager_insert_action_group(ui_manager, tab_action_group, 0);
 
    accel_group = gtk_ui_manager_get_accel_group(ui_manager);
    gtk_window_add_accel_group(GTK_WINDOW (window), accel_group);
 
    error = NULL;
    if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_description, -1, &error)) {
        g_message("building menus failed: %s", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
    }
 
    menubar = gtk_ui_manager_get_widget(ui_manager, "/MainMenu");
    gtk_box_pack_start(GTK_BOX (mainvbox), menubar, FALSE, FALSE, 0);

    mainnotebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX (mainvbox), mainnotebook, TRUE, TRUE, 0);

    g_signal_connect(G_OBJECT (mainnotebook), "switch-page",				/* Init switch-page event of notebook */
                        G_CALLBACK (NoteBookTabChange), NULL);

    if (NumFiles > 0) {
	for (i=0;i<NumFiles;i++) {
	    realcoords[i][0] = TempCoords[0];
	    realcoords[i][2] = TempCoords[1];
	    realcoords[i][1] = TempCoords[2];
	    realcoords[i][3] = TempCoords[3];
	    logxy[i][0] = Uselogxy[0];
	    logxy[i][1] = Uselogxy[1];
	    UseErrors[i] = UseError;
	    SetupNewTab(argv[FileIndex[i]], Scale, maxX, maxY, UsePreSetCoords);
	}
    }

    g_signal_connect_swapped (G_OBJECT (window), "key_press_event",
			          G_CALLBACK (key_press_event), NULL);
    g_signal_connect_swapped (G_OBJECT (window), "key_release_event",
			          G_CALLBACK (key_release_event), NULL);

    gtk_widget_show_all(window);							/* Show all widgets */

    gtk_main();										/* This is where it all starts */
              
    free(colors);									/* Deallocate memory */

    return(0);										/* Exit. */
}
