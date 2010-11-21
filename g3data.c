/*
g3data : A program for grabbing data from scanned graphs
Copyright (C) 2000 Jonas Frantz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

My email : jonas.frantz@helsinki.fi
*/

#include <gtk/gtk.h>					/* Include gtk library */
#include <gdk-pixbuf/gdk-pixbuf.h>			/* Include gdk-pixbuf library */
#include <gdk/gdk.h>					/* Include gdk library */
#include <stdio.h>					/* Include stdio library */
#include <stdlib.h>					/* Include stdlib library */
#include <string.h>
#include <math.h>					/* Include math library */
#include "tooltips.h"					/* Include toolstips */

#define ZoomPixSize 200					/* Size of zoom in window */
#define ZoomFactor 4					/* Zoom factor of zoom window */
#define InnerSize 1					/* Size of white inner square */
#define OuterSize 2					/* Size of black outer square */
#define MarkerLength 6
#define MarkerThick 2
#define MaxPoints 1024					/* Maximum number of points that can be put out */
#define VERSION "1.08"					/* Version number */

/* Declaration of gtk, gdk and imlib variables */
GtkWidget	*window, *drawing_area, *zoom_area;		/* Windows and drawing areas */
GtkWidget	*x1_entry, *y1_entry, *x2_entry, *y2_entry;	/* Text entries */
GtkWidget	*quitbutton, *printbutton, *remlastbutton;	/* Various buttons */
GtkWidget	*setx1button, *setx2button, *sety1button;	/* More various buttons */
GtkWidget	*sety2button, *remallbutton, *fileoutputb;	/* Even more various buttons */
GdkPixmap	*pixmap;
GdkColor        *colors;					/* Pointer to colors */
GdkGC 		*gc;						/* Graphic context */
GtkWidget	*xc_entry,*yc_entry,*file_entry;		/* Coordinate and filename entries */

GdkPixbuf       *gpbimage;

/* Declaration of various variables */
gint		axiscoords[4][2];			/* X,Y coordinates of axispoints */
gint		points[MaxPoints][2];			/* Indexes of graphpoints and their coordinates */
gint		lastpoints[MaxPoints+4];		/* Indexes of last points put out */
gint		numpoints = 0, numlastpoints = 0;	/* Number of points on graph and last put out */
gint		XSize, YSize,remthis = 0,ordering = 0;	/* Various control variables */
gint		file_name_length;
gfloat		realcoords[4];				/* X,Y coords on graph */
gboolean	x1pressed, x2pressed, y1pressed,y2pressed;  /* Set point buttons pressed in ? */
gboolean	bpressed[4] = {FALSE,FALSE,FALSE,FALSE};/* What axispoints have been set out ? */
gboolean	valueset[4] = {FALSE,FALSE,FALSE,FALSE};
gboolean	logx = FALSE, logy = FALSE;		/* Logarithmic axises ? */
gchar 		*file_name;				/* Pointer to filename */
FILE		*FP;					/* File pointer */
gboolean	print2file;				/* Should we print to file ? */

/* Declaration of functions */
void remove_last(GtkWidget *widget, gpointer data);


/****************************************************************/
/* This function returns the number of the point, which resides	*/
/* on the position described by the variable 'index' ordered by	*/
/* the x-coordinate.						*/
/****************************************************************/
gint orderbyx(gint index) 
{
gint i, j, value, chosen;
gboolean sortedpoints[MaxPoints], valueused;

    for (i=0;i<numpoints;i++) {				/* Nullify checking table */
	sortedpoints[i]=TRUE;
    }

    for (i=0;i<index+1;i++) {				/* Go through it as many times as the index */
	valueused=TRUE;					/* of the wanted sorted number is */
	for (j=0;j<numpoints;j++) {			/* Go trough all points */
	    if (sortedpoints[j] && (points[j][0] < value || valueused)) { /* Is point coordinate less than value ? */
		value=points[j][0];			/* Set value to value of point */
		valueused=FALSE;			/* Value has been set */
		chosen=j;				/* This index might be the chosen one */
	    }
	}
    sortedpoints[chosen]=FALSE;				/* Mark this point used */
    }
    return chosen;					/* Return the chosen one */
}


/****************************************************************/
/* This function returns the number of the point, which resides	*/
/* on the position described by the variable 'index' ordered by	*/
/* the y-coordinate.						*/
/****************************************************************/
gint orderbyy(gint index) 
{
gint i, j, value, chosen;
gboolean sortedpoints[MaxPoints], valueused;

    for (i=0;i<numpoints;i++) {				/* Nullify checking table */
	sortedpoints[i]=TRUE;
    }

    for (i=0;i<index+1;i++) {				/* Go through it as many times as the index */
	valueused=TRUE;					/* of the wanted sorted number is */
	for (j=0;j<numpoints;j++) {			/* Go trough all points */
	    if (sortedpoints[j] && (points[j][1] > value || valueused)) {	/* Is point coordinate more than value ? */
		value=points[j][1];			/* Set value to value of point */
		valueused=FALSE;			/* Value has been set */
		chosen=j;				/* This index might be the chosen one */
	    }
	}
    sortedpoints[chosen]=FALSE;				/* Mark this point used */
    }
    return chosen;					/* Return the chosen one */
}


/****************************************************************/
/* This function closes the window when the application is 	*/
/* killed.							*/
/****************************************************************/
gint close_application( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    gtk_main_quit();					/* Quit gtk */
    return(FALSE);
}


/****************************************************************/
/* When a button is pressed inside the drawing area this 	*/
/* function is called, it handles axispoints and graphpoints	*/
/* and paints a square in that position.			*/
/****************************************************************/
gint button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
gint x,y;
GdkModifierType state;

    gc = gdk_gc_new (drawing_area->window);		/* Create graphics context */

    gdk_window_get_pointer (event->window, &x, &y, &state); /* Get pointer state */
    if (event->button == 1) {				/* If button 1 (leftmost) is pressed */

/* If none of the set axispoint buttons been pressed */
	if (!x1pressed && !x2pressed && !y1pressed && !y2pressed) {
	    if (numpoints>=MaxPoints) {
		printf("Limit for points (MaxPoints) on graph reached!\n");
	    }
	    else {
		points[numpoints][0]=x;			/* Save x coordinate */
		points[numpoints][1]=y;			/* Save x coordinate */
		lastpoints[numlastpoints]=numpoints;	/* Save index of point */
		numlastpoints++;				/* Increase lastpoint index */
		numpoints++;				/* Increase point counter */
		gtk_widget_set_sensitive(remlastbutton,TRUE);	/* Activate "Remove last" button */
		gtk_widget_set_sensitive(remallbutton,TRUE);	/* Activate "Remove all" button */

		gdk_gc_set_foreground (gc, &colors[2]);		/* Set color of graphics context */
		gdk_draw_rectangle(drawing_area->window,gc,TRUE,
				x-OuterSize,y-OuterSize,OuterSize*2+1,OuterSize*2+1);
		gdk_gc_set_foreground (gc, &colors[3]);		/* Set color of graphics context */
		gdk_draw_rectangle(drawing_area->window,gc,TRUE,
				x-InnerSize,y-InnerSize,InnerSize*2+1,InnerSize*2+1);
	    }
	}
	else if (x1pressed || x2pressed) {
	    if (x1pressed) {				/* If the "Set point 1 on x axis" button is pressed */
	    axiscoords[0][0]=x;				/* Save coordinates */
	    axiscoords[0][1]=y;
	    gtk_widget_set_sensitive(setx2button,TRUE);	/* Restore the other "Set axis */
	    gtk_widget_set_sensitive(sety1button,TRUE);	/* point" buttons */
	    gtk_widget_set_sensitive(sety2button,TRUE);
	    gtk_widget_set_sensitive(x1_entry,TRUE);	/* Sensitize the entry */
	    gtk_entry_set_editable(GTK_ENTRY(x1_entry),TRUE); /* and set it to editable */
	    gtk_widget_grab_focus(x1_entry);		/* Focus on entry */
	    x1pressed=FALSE;				/* Mark the button as not pressed */
	    bpressed[0]=TRUE;				/* Mark that axis point's been set */
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setx1button),FALSE); /* Pop up the button */
	    lastpoints[numlastpoints]=-1;		/* Remember that the points been put out */
	    numlastpoints++;				/* Increase index of lastpoints */
	    gtk_widget_set_sensitive(remlastbutton,TRUE);	/* Activate "Remove last" button */
	    gtk_widget_set_sensitive(remallbutton,TRUE);	/* Activate "Remove all" button */
	}
	else if (x2pressed) {				/* If the "Set point 2 on x axis" button is pressed */
	    axiscoords[1][0]=x;				/* otherwise the same as above */
	    axiscoords[1][1]=y;
	    gtk_widget_set_sensitive(setx1button,TRUE);
	    gtk_widget_set_sensitive(sety1button,TRUE);
	    gtk_widget_set_sensitive(sety2button,TRUE);
	    gtk_widget_set_sensitive(x2_entry,TRUE);
	    gtk_entry_set_editable(GTK_ENTRY(x2_entry),TRUE);
	    gtk_widget_grab_focus(x2_entry);
	    x2pressed=FALSE;
	    bpressed[1]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setx2button),FALSE);
	    lastpoints[numlastpoints]=-2;
	    numlastpoints++;
	    gtk_widget_set_sensitive(remlastbutton,TRUE);
	    gtk_widget_set_sensitive(remallbutton,TRUE);
	}
    	gdk_gc_set_foreground (gc, &colors[0]);		/* Set color of graphics context */
	gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x-MarkerLength,y-MarkerThick/2,MarkerLength*2+1,MarkerThick+1);
	gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x-MarkerThick/2,y-MarkerLength,MarkerThick+1,MarkerLength+1);
	}
	else {
	if (y1pressed) {				/* If the "Set point 1 on y axis" button is pressed */
	    axiscoords[2][0]=x;				/* otherwise the same as above */
	    axiscoords[2][1]=y;
	    gtk_widget_set_sensitive(setx1button,TRUE);
	    gtk_widget_set_sensitive(setx2button,TRUE);
	    gtk_widget_set_sensitive(sety2button,TRUE);
	    gtk_widget_set_sensitive(y1_entry,TRUE);
	    gtk_entry_set_editable(GTK_ENTRY(y1_entry),TRUE);
	    gtk_widget_grab_focus(y1_entry);
	    y1pressed=FALSE;
	    bpressed[2]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sety1button),FALSE);
	    lastpoints[numlastpoints]=-3;
	    numlastpoints++;
	    gtk_widget_set_sensitive(remlastbutton,TRUE);
	    gtk_widget_set_sensitive(remallbutton,TRUE);
	    }
	else if (y2pressed) {				/* If the "Set point 2 on y axis" button is pressed */
	    axiscoords[3][0]=x;				/* otherwise the same as above */
	    axiscoords[3][1]=y;
	    gtk_widget_set_sensitive(setx1button,TRUE);
	    gtk_widget_set_sensitive(setx2button,TRUE);
	    gtk_widget_set_sensitive(sety1button,TRUE);
	    gtk_widget_set_sensitive(y2_entry,TRUE);
	    gtk_entry_set_editable(GTK_ENTRY(y2_entry),TRUE);
	    gtk_widget_grab_focus(y2_entry);
	    y2pressed=FALSE;
	    bpressed[3]=TRUE;
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sety2button),FALSE);
	    lastpoints[numlastpoints]=-4;
	    numlastpoints++;
	    gtk_widget_set_sensitive(remlastbutton,TRUE);
	    gtk_widget_set_sensitive(remallbutton,TRUE);
	    }

	gdk_gc_set_foreground (gc, &colors[1]);		/* Set color of graphics context */
	gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x-MarkerThick/2,y-MarkerLength,MarkerThick+1,MarkerLength*2+1);
	gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x,y-MarkerThick/2,MarkerLength+1,MarkerThick+1);
	}
	if (bpressed[0] && bpressed[1] && bpressed[2] && bpressed[3] && valueset[0] && valueset[1] && valueset[2] &&
	    valueset[3] && numpoints > 0) {
	    gtk_widget_set_sensitive(printbutton,TRUE);	/* If all axispoints been set and */
							/* there are more than one graph- */
							/* point, activate the "Print	  */ 
							/* data" button.		  */
	    if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE);
	}
    }
    else if (event->button == 2) {			/* Is middle button pressed ? */
	if (!bpressed[0] || !bpressed[1]) {		/* Is neither of x axis points put out ? */
	    if (!bpressed[0]) {				/* Is x axispoint 1 not put out ? */
		axiscoords[0][0]=x;			/* Same procedure as above */
		axiscoords[0][1]=y;
		gtk_widget_set_sensitive(setx2button,TRUE);
		gtk_widget_set_sensitive(sety1button,TRUE);
		gtk_widget_set_sensitive(sety2button,TRUE);
		gtk_widget_set_sensitive(x1_entry,TRUE);
		gtk_entry_set_editable(GTK_ENTRY(x1_entry),TRUE);
		gtk_widget_grab_focus(x1_entry);
		x1pressed=FALSE;
		bpressed[0]=TRUE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setx1button),FALSE);
		lastpoints[numlastpoints]=-1;
		numlastpoints++;
		gtk_widget_set_sensitive(remlastbutton,TRUE);
		gtk_widget_set_sensitive(remallbutton,TRUE);
	    } 
	    else if (!bpressed[1]) {			/* Is x axispoint 2 not put out ? */
		axiscoords[1][0]=x;			/* Same procedure as above */
		axiscoords[1][1]=y;
		gtk_widget_set_sensitive(setx1button,TRUE);
		gtk_widget_set_sensitive(sety1button,TRUE);
		gtk_widget_set_sensitive(sety2button,TRUE);
		gtk_widget_set_sensitive(x2_entry,TRUE);
		gtk_entry_set_editable(GTK_ENTRY(x2_entry),TRUE);
		gtk_widget_grab_focus(x2_entry);
		x2pressed=FALSE;
		bpressed[1]=TRUE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setx2button),FALSE);
		lastpoints[numlastpoints]=-2;
		numlastpoints++;
		gtk_widget_set_sensitive(remlastbutton,TRUE);
		gtk_widget_set_sensitive(remallbutton,TRUE);
	    }
    	    gdk_gc_set_foreground (gc, &colors[0]);
	    gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x-MarkerLength,y-MarkerThick/2,MarkerLength*2+1,MarkerThick+1);
	    gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x-MarkerThick/2,y-MarkerLength,MarkerThick+1,MarkerLength+1);
	}
    }
    else if (event->button == 3) {			/* Is the right button pressed ?*/
	if (!bpressed[2] || !bpressed[3]) {		/* Is neither of the y axispoints put out ?*/
	    if (!bpressed[2]) {				/* Is y axispoint 1 not put out ? */
		axiscoords[2][0]=x;			/* Same procedure as above */
		axiscoords[2][1]=y;
		gtk_widget_set_sensitive(setx1button,TRUE);
		gtk_widget_set_sensitive(setx2button,TRUE);
		gtk_widget_set_sensitive(sety2button,TRUE);
		gtk_widget_set_sensitive(y1_entry,TRUE);
		gtk_entry_set_editable(GTK_ENTRY(y1_entry),TRUE);
		gtk_widget_grab_focus(y1_entry);
		y1pressed=FALSE;
		bpressed[2]=TRUE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sety1button),FALSE);
		lastpoints[numlastpoints]=-3;
	 	numlastpoints++;
	 	gtk_widget_set_sensitive(remlastbutton,TRUE);
	 	gtk_widget_set_sensitive(remallbutton,TRUE);
	    } 
	    else if (!bpressed[3]) {			/* Is y axispoint 2 not put out ? */
		axiscoords[3][0]=x;			/* Same procedure as above */
		axiscoords[3][1]=y;
		gtk_widget_set_sensitive(setx1button,TRUE);
		gtk_widget_set_sensitive(setx2button,TRUE);
		gtk_widget_set_sensitive(sety1button,TRUE);
		gtk_widget_set_sensitive(y2_entry,TRUE);
		gtk_entry_set_editable(GTK_ENTRY(y2_entry),TRUE);
		gtk_widget_grab_focus(y2_entry);
		y2pressed=FALSE;
		bpressed[3]=TRUE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sety2button),FALSE);
		lastpoints[numlastpoints]=-4;
		numlastpoints++;
		gtk_widget_set_sensitive(remlastbutton,TRUE);
		gtk_widget_set_sensitive(remallbutton,TRUE);
	    }
	    gdk_gc_set_foreground (gc, &colors[1]);
	    gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x-MarkerThick/2,y-MarkerLength,MarkerThick+1,MarkerLength*2+1);
	    gdk_draw_rectangle(drawing_area->window,gc,TRUE,
			   x,y-MarkerThick/2,MarkerLength+1,MarkerThick+1);
	}
    }
    gdk_gc_unref(gc);					/* Kill graphics context */
    return TRUE;
}


/****************************************************************/
/* This function is called when a button is released on the	*/
/* drawing area, currently this function does not perform any	*/
/* task.							*/
/****************************************************************/
gint button_release_event(GtkWidget *widget, GdkEventButton *event,gpointer data)
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
gint motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
gint xmin, ymin, xmax, ymax;
gint xsize, ysize, xoffset, yoffset;
gint x,y;
GdkModifierType state;
double x21, x43, y21, y43;
double rlcx1, rlcx2, rlcy1, rlcy2, alpha, beta;
gchar xbuf[30],ybuf[30];
GdkGC  *mngc;							/* Graphic context */

static GdkPixbuf	*gpbzoomimage;
static gboolean FirstTime=TRUE;

    gdk_window_get_pointer (event->window, &x, &y, &state);	/* Grab mousepointers coordinates */
								/* on drawing area. */
    if (FirstTime) {
	gpbzoomimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, ZoomPixSize, ZoomPixSize);
	FirstTime = FALSE;
    }

    if (x>0 && y>0 && x<XSize && y<YSize) {
	mngc = gdk_gc_new (zoom_area->window);			/* Create graphics context */

	xmin = MAX(x-ZoomPixSize/(2*ZoomFactor),0);
	xoffset = 0;
	if( x-ZoomPixSize/(2*ZoomFactor) < 0 ) {
	    xoffset = -(x*ZoomFactor-ZoomPixSize/2);
	}
	xmax = MIN(x+ZoomPixSize/(2*ZoomFactor),XSize);
	ymin = MAX(y-ZoomPixSize/(2*ZoomFactor),0);
	yoffset = 0;
	if( y-ZoomPixSize/(2*ZoomFactor) < 0 ) {
	    yoffset = -(y*ZoomFactor-ZoomPixSize/2);
	}
	ymax = MIN(y+ZoomPixSize/(2*ZoomFactor),YSize);
	xsize = (xmax-xmin)*ZoomFactor;
	ysize = (ymax-ymin)*ZoomFactor;

/********************************************************************************/
/* Change GDK_INTERP_BILINEAR to GDK_INTERP_HYPER in the next call if you want	*/
/* observe the memory leak !							*/
/********************************************************************************/

	gdk_pixbuf_composite(gpbimage, gpbzoomimage, 0, 0, ZoomPixSize, 
			     ZoomPixSize, -x*ZoomFactor + ZoomPixSize/2, -y*ZoomFactor + ZoomPixSize/2, 
			     1.0*ZoomFactor, 1.0*ZoomFactor, GDK_INTERP_BILINEAR, 255);

	gdk_pixbuf_render_to_drawable(gpbzoomimage,zoom_area->window,zoom_area->style->white_gc,
				      0,0,0,0,ZoomPixSize,ZoomPixSize,GDK_RGB_DITHER_NONE,0,0);

/* Then draw the square in the middle of the zoom area */

	gdk_gc_set_foreground (mngc, &colors[2]);			/* Set color of graphics context */
	gdk_draw_rectangle(zoom_area->window,mngc,
			   TRUE,(ZoomPixSize/2)-OuterSize,(ZoomPixSize/2)-OuterSize,
			   OuterSize*2+1,OuterSize*2+1);
	gdk_gc_set_foreground (mngc, &colors[3]);			/* Set color of graphics context */
	gdk_draw_rectangle(zoom_area->window,mngc,
			   TRUE,(ZoomPixSize/2)-InnerSize,(ZoomPixSize/2)-InnerSize,
			   InnerSize*2+1,InnerSize*2+1);

	if (valueset[0] && valueset[1] && valueset[2] && valueset[3]) {
	    x21=(double) axiscoords[1][0]-axiscoords[0][0];	/* Calculate deltax of x axis points */
	    y21=(double) axiscoords[1][1]-axiscoords[0][1];	/* Calculate deltay of x axis points */
	    x43=(double) axiscoords[3][0]-axiscoords[2][0];	/* Calculate deltax of y axis points */
	    y43=(double) axiscoords[3][1]-axiscoords[2][1];	/* Calculate deltay of y axis points */

	    if (logx) {					/* If x axis is logarithmic, store */
		rlcx1=log(realcoords[0]);			/* recalculated values in rlc. */ 
		rlcx2=log(realcoords[1]);
	    }
	    else {
		rlcx1=realcoords[0];			/* Else store old values in rlc. */
		rlcx2=realcoords[1];
	    }

	    if (logy) {
		rlcy1=log(realcoords[2]);			/* If y axis is logarithmic, store */
		rlcy2=log(realcoords[3]);			/* recalculated values in rlc.*/
	    }
	    else {
		rlcy1=realcoords[2];			/* Else store old values in rlc.*/
		rlcy2=realcoords[3];
	    }

	    alpha=((axiscoords[0][0]-(double) x) - (axiscoords[0][1]-(double) y)*(x43/y43))/(x21-((y21*x43)/y43));
	    beta=((axiscoords[2][1]-(double) y) - (axiscoords[2][0]-(double) x)*(y21/x21))/(y43-((x43*y21)/x21));

	    if (logx) {
		sprintf(xbuf,"%g",exp(-alpha*(rlcx2-rlcx1)+rlcx1));
	    }
	    else {
		sprintf(xbuf,"%f",-alpha*(rlcx2-rlcx1)+rlcx1);
	    }
	    if (logy) {
		sprintf(ybuf,"%g",exp(-beta*(rlcy2-rlcy1)+rlcy1));
	    }
	    else {
		sprintf(ybuf,"%f",-beta*(rlcy2-rlcy1)+rlcy1);
	    }
	    gtk_entry_set_text(GTK_ENTRY(xc_entry),xbuf);	/* Put out coordinates in entries */
	    gtk_entry_set_text(GTK_ENTRY(yc_entry),ybuf);
	}
	else {
	    gtk_entry_set_text(GTK_ENTRY(xc_entry),"");	/* Else clear entries */
	    gtk_entry_set_text(GTK_ENTRY(yc_entry),"");
	}
    gdk_gc_unref(mngc);					/* Kill graphics context */
    }
    return TRUE;
}


/****************************************************************/
/* This function is called when the drawing area is exposed, it	*/
/* simply redraws the pixmap on it.				*/
/****************************************************************/
static gint expose_event (GtkWidget *widget, GdkEventExpose *event,gpointer data)
{
gint i;
static GdkGC *eegc;
static gboolean FirstTime;

    gdk_draw_pixmap(widget->window,widget->style->white_gc,pixmap,
          event->area.x, event->area.y,
          event->area.x, event->area.y,
          event->area.width, event->area.height);

    if (FirstTime) {
	eegc = gdk_gc_new (zoom_area->window);			/* Create graphics context */
	FirstTime = FALSE;
    }

    if (bpressed[0]) {
    	gdk_gc_set_foreground (eegc, &colors[0]);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[0][0]-MarkerLength,
			   axiscoords[0][1]-MarkerThick/2,MarkerLength*2+1,MarkerThick+1);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[0][0]-MarkerThick/2,
			   axiscoords[0][1]-MarkerLength,MarkerThick+1,MarkerLength+1);
    }
    if (bpressed[1]) {
    	gdk_gc_set_foreground (eegc, &colors[0]);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[1][0]-MarkerLength,
			   axiscoords[1][1]-MarkerThick/2,MarkerLength*2+1,MarkerThick+1);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[1][0]-MarkerThick/2,
			   axiscoords[1][1]-MarkerLength,MarkerThick+1,MarkerLength+1);
    }
    if (bpressed[2]) {
	gdk_gc_set_foreground (eegc, &colors[1]);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[2][0]-MarkerThick/2,
			   axiscoords[2][1]-MarkerLength,MarkerThick+1,MarkerLength*2+1);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[2][0],
			   axiscoords[2][1]-MarkerThick/2,MarkerLength+1,MarkerThick+1);
    }
    if (bpressed[3]) {
	gdk_gc_set_foreground (eegc, &colors[1]);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[3][0]-MarkerThick/2,
			   axiscoords[3][1]-MarkerLength,MarkerThick+1,MarkerLength*2+1);
	gdk_draw_rectangle(drawing_area->window,eegc, TRUE, axiscoords[3][0],
			   axiscoords[3][1]-MarkerThick/2,MarkerLength+1,MarkerThick+1);
    }

    for (i=0;i<numpoints;i++) {
    	gdk_gc_set_foreground (eegc, &colors[2]);		/* Set color of graphics context */
	gdk_draw_rectangle(drawing_area->window,eegc,
			   TRUE,points[i][0]-OuterSize,points[i][1]-OuterSize,
			   OuterSize*2+1,OuterSize*2+1);
    	gdk_gc_set_foreground (eegc, &colors[3]);		/* Set color of graphics context */
	gdk_draw_rectangle(drawing_area->window,eegc,
			   TRUE,points[i][0]-InnerSize,points[i][1]-InnerSize,
			   InnerSize*2+1,InnerSize*2+1);
    }

//    gdk_gc_unref(gc);					/* Kill the graphics context */
    return FALSE;
}   


/****************************************************************/
/* This function is called when the drawing area is configured	*/
/* for the first time, currently this function does not perform	*/
/* any task.							*/
/****************************************************************/
gint configure_event (GtkWidget *widget, GdkEventConfigure *event,gpointer data)
{
    return TRUE;
}


/****************************************************************/
/* This function is called when the "Set point 1 on x axis"	*/
/* button is pressed. It inactivates the other "Set" buttons	*/
/* and makes sure the button stays down even when pressed on.	*/
/****************************************************************/
void toggle_x1 (GtkWidget *widget, gpointer data)
{
    if (GTK_TOGGLE_BUTTON (widget)->active) {		/* Is the button pressed on ? */
	x1pressed=TRUE;					/* The button is pressed down */
	gtk_widget_set_sensitive(setx2button,FALSE);	/* Inactivate other "Set" buttons */
	gtk_widget_set_sensitive(sety1button,FALSE);
	gtk_widget_set_sensitive(sety2button,FALSE);
	if (bpressed[0]) {				/* If the x axis point is already set */
	    remthis=-1;					/* remove the square */
	    remove_last(widget,NULL);
	}
	bpressed[0]=FALSE;				/* Set x axis point 1 to unset */
    } else {						/* If button is trying to get unpressed */
	if (x1pressed) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE); /* Set button down */
    }
}


/****************************************************************/
/* This function is called when the "Set point 2 on x axis"	*/
/* button is pressed. It inactivates the other "Set" buttons	*/
/* and makes sure the button stays down even when pressed on.	*/
/* This function work exactly the same way as the above and is	*/
/* therefore not explicitly commented.				*/
/****************************************************************/
void toggle_x2 (GtkWidget *widget, gpointer data)
{
    if (GTK_TOGGLE_BUTTON (widget)->active) {
	x2pressed=TRUE;        
	gtk_widget_set_sensitive(setx1button,FALSE);
	gtk_widget_set_sensitive(sety1button,FALSE);
	gtk_widget_set_sensitive(sety2button,FALSE);
	if (bpressed[1]) {
	    remthis=-2;
	    remove_last(widget,NULL);
	}
	bpressed[1]=FALSE;
    } else {
	if (x2pressed) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
    }
}


/****************************************************************/
/* This function is called when the "Set point 1 on y axis"	*/
/* button is pressed. It inactivates the other "Set" buttons	*/
/* and makes sure the button stays down even when pressed on.	*/
/* This function work exactly the same way as the above and is	*/
/* therefore not explicitly commented.				*/
/****************************************************************/
void toggle_y1 (GtkWidget *widget, gpointer data)
{
    if (GTK_TOGGLE_BUTTON (widget)->active) {
	y1pressed=TRUE;        
	gtk_widget_set_sensitive(setx1button,FALSE);
	gtk_widget_set_sensitive(setx2button,FALSE);
	gtk_widget_set_sensitive(sety2button,FALSE);
	if (bpressed[2]) {
	    remthis=-3;
	    remove_last(widget,NULL);
	}
	bpressed[2]=FALSE;
    } else {
	if (y1pressed) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
    }
}


/****************************************************************/
/* This function is called when the "Set point 2 on y axis"	*/
/* button is pressed. It inactivates the other "Set" buttons	*/
/* and makes sure the button stays down even when pressed on.	*/
/* This function work exactly the same way as the above and is	*/
/* therefore not explicitly commented.				*/
/****************************************************************/
void toggle_y2 (GtkWidget *widget, gpointer data)
{
    if (GTK_TOGGLE_BUTTON (widget)->active) {
	y2pressed=TRUE;        
	gtk_widget_set_sensitive(setx1button,FALSE);
	gtk_widget_set_sensitive(setx2button,FALSE);
	gtk_widget_set_sensitive(sety1button,FALSE);
	if (bpressed[3]) {
	    remthis=-4;
	    remove_last(widget,NULL);
	}
	bpressed[3]=FALSE;
    } else {
	if (y2pressed) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
    }
}


/****************************************************************/
/* When the no-order radio button is pressed down this function	*/
/* gets called.							*/
/****************************************************************/
void ordernot(GtkWidget *widget, gpointer data)
{
    ordering=0;						/* Set ordering control variable */
}


/****************************************************************/
/* When the order by x radio button is pressed down this 	*/
/* function gets called.					*/
/****************************************************************/
void orderx(GtkWidget *widget, gpointer data)
{
    ordering=1;						/* Set ordering control variable */
}


/****************************************************************/
/* When the order by y radio button is pressed down this	*/
/* function gets called.					*/
/****************************************************************/
void ordery(GtkWidget *widget, gpointer data)
{
    ordering=2;						/* Set ordering control variable */
}


/****************************************************************/
/* When the value of the entry of the first x axis point is	*/
/* changed, this function gets called.				*/
/****************************************************************/
void read_x1_entry( GtkWidget *widget, GtkWidget *entry )
{
gchar *x1_text;
    x1_text = gtk_entry_get_text(GTK_ENTRY(entry));	/* Get pointer to entry string */
    sscanf(x1_text,"%f",&realcoords[0]);		/* Convert string to float value and */
							/* store in realcoords[0]. */
    if (logx && realcoords[0] > 0) valueset[0]=TRUE;
    else if (logx) valueset[0]=FALSE;
    else valueset[0]= TRUE; 

    if (bpressed[0] && bpressed[1] && bpressed[2] && 	/* Are all buttons pressed and all values set ? */
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0) {
	gtk_widget_set_sensitive(printbutton,TRUE);	/* Activate printoutbutton */
	if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE); /* If filename entered activate printouttofilebutton */
    }
    else {
	gtk_widget_set_sensitive(printbutton,FALSE);	/* Else deactivate buttons */
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
}


/****************************************************************/
/* When the value of the entry of the second x axis point is	*/
/* changed, this function gets called.				*/
/****************************************************************/
void read_x2_entry( GtkWidget *widget, GtkWidget *entry )
{
gchar *x2_text;
    x2_text = gtk_entry_get_text(GTK_ENTRY(entry));	/* Get pointer to entry string */
    sscanf(x2_text,"%f",&realcoords[1]);		/* Convert string to float value and */
							/* store in realcoords[1]. */
    if (logx && realcoords[1] > 0) valueset[1]=TRUE;
    else if (logx) valueset[1]=FALSE;
    else valueset[1]= TRUE;

    if (bpressed[0] && bpressed[1] && bpressed[2] && 
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0) {
	gtk_widget_set_sensitive(printbutton,TRUE);
	if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE);
    }
    else {
	gtk_widget_set_sensitive(printbutton,FALSE);
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
}


/****************************************************************/
/* When the value of the entry of the first y axis point is	*/
/* changed, this function gets called.				*/
/****************************************************************/
void read_y1_entry( GtkWidget *widget, GtkWidget *entry )
{
gchar *y1_text;
    y1_text = gtk_entry_get_text(GTK_ENTRY(entry));	/* Get pointer to entry string */
    sscanf(y1_text,"%f",&realcoords[2]);		/* Convert string to float value and */
							/* store in realcoords[2]. */
    if (logy && realcoords[2] > 0) valueset[2]=TRUE;
    else if (logy) valueset[2]=FALSE;
    else valueset[2]= TRUE; 

    if (bpressed[0] && bpressed[1] && bpressed[2] && 
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0) {
	gtk_widget_set_sensitive(printbutton,TRUE);
	if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE);
    }
    else {
	gtk_widget_set_sensitive(printbutton,FALSE);
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
}


/****************************************************************/
/* When the value of the entry of the second y axis point is	*/
/* changed, this function gets called.				*/
/****************************************************************/
void read_y2_entry( GtkWidget *widget, GtkWidget *entry )
{
gchar *y2_text;
    y2_text = gtk_entry_get_text(GTK_ENTRY(entry));	/* Get pointer to entry string */
    sscanf(y2_text,"%f",&realcoords[3]);		/* Convert string to float value and */
							/* store in realcoords[3]. */
    if (logy && realcoords[3] > 0) valueset[3]=TRUE;
    else if (logy) valueset[3]=FALSE;
    else valueset[3]= TRUE; 

    if (bpressed[0] && bpressed[1] && bpressed[2] && 
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0) {
	gtk_widget_set_sensitive(printbutton,TRUE);
	if (file_name_length>0) gtk_widget_set_sensitive(fileoutputb,TRUE);

    }
    else {
	gtk_widget_set_sensitive(printbutton,FALSE);
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
}


/****************************************************************/
/* If all the axispoints has been put out, values for these	*/
/* have been assigned and at least one point has been set on	*/
/* the graph activate the write to file button.			*/
/****************************************************************/
void read_file_entry( GtkWidget *widget, GtkWidget *entry )
{
    file_name = gtk_entry_get_text(GTK_ENTRY(entry));	/* Get pointer to entry string */
    file_name_length = strlen(file_name);		/* Get length of string */

    if (bpressed[0] && bpressed[1] && bpressed[2] && 
	bpressed[3] && valueset[0] && valueset[1] && 
	valueset[2] && valueset[3] && numpoints > 0 &&
	file_name_length>0) {
	gtk_widget_set_sensitive(fileoutputb,TRUE);
    }
    else gtk_widget_set_sensitive(fileoutputb,FALSE);
}


/****************************************************************/
/* If the "X axis is logarithmic" check button is toggled this	*/
/* function gets called. It sets the logx variable to its	*/
/* correct value corresponding to the buttons state.		*/
/****************************************************************/
void islogx( GtkWidget *widget, gpointer data)
{
    logx = (GTK_TOGGLE_BUTTON (widget)->active); 	/* If checkbutton is pressed down */
							/* logx = TRUE else FALSE. */
    if (logx) {
	if (realcoords[0] <= 0) {			/* If a negative value has been insert */
	    valueset[0]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(x1_entry),"");	/* Zero it */
	}
	if (realcoords[1] <= 0) {			/* If a negative value has been insert */
	    valueset[1]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(x2_entry),"");	/* Zero it */
        }
    }
}


/****************************************************************/
/* If the "Y axis is logarithmic" check button is toggled this	*/
/* function gets called. It sets the logy variable to its	*/
/* correct value corresponding to the buttons state.		*/
/****************************************************************/
void islogy( GtkWidget *widget, gpointer data)
{
    logy = (GTK_TOGGLE_BUTTON (widget)->active);	/* If checkbutton is pressed down */
							/* logx = TRUE else FALSE. */
    if (logy) {
	if (realcoords[2] <= 0) {			/* If a negative value has been insert */
	    valueset[2]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(y1_entry),""); /* Zero it */
	}
	if (realcoords[3] <= 0) {			/* If a negative value has been insert */
	    valueset[3]=FALSE;
	    gtk_entry_set_text(GTK_ENTRY(y2_entry),""); /* Zero it */
        }
    }
}


/****************************************************************/
/* This function is called when the "Print results" button is	*/
/* pressed, it calculate the values of the datapoints and 	*/
/* prints them through stdout.					*/
/****************************************************************/
void print_results(GtkWidget *widget, gpointer data) 
{
double alpha,beta,x21,x43,y21,y43,rlc[4];		/* Declare help variables */
gint i, j;						/* Declare index variable */

    x21=(double) axiscoords[1][0]-axiscoords[0][0];	/* Calculate deltax of x axis points */
    y21=(double) axiscoords[1][1]-axiscoords[0][1];	/* Calculate deltay of x axis points */
    x43=(double) axiscoords[3][0]-axiscoords[2][0];	/* Calculate deltax of y axis points */
    y43=(double) axiscoords[3][1]-axiscoords[2][1];	/* Calculate deltay of y axis points */

    if (logx) {						/* If x axis is logarithmic, store	*/
	rlc[0]=log(realcoords[0]);			/* recalculated values in rlc.		*/ 
	rlc[1]=log(realcoords[1]);
    }
    else {
	rlc[0]=realcoords[0];				/* Else store old values in rlc.	*/
	rlc[1]=realcoords[1];
    }

    if (logy) {
	rlc[2]=log(realcoords[2]);			/* If y axis is logarithmic, store      */
	rlc[3]=log(realcoords[3]);			/* recalculated values in rlc.          */
    }
    else {
	rlc[2]=realcoords[2];				/* Else store old values in rlc.        */
	rlc[3]=realcoords[3];
    }

/* Next up is recalculating the positions of the points by solving a 2*2 matrix */
    for (i=0;i<numpoints;i++) {
	if (ordering == 1) j = orderbyx(i);
	else if (ordering == 2) j = orderbyy(i);
	else j = i;

	alpha=((axiscoords[0][0]-(double) points[j][0]) 
	      -(axiscoords[0][1]-(double) points[j][1])*(x43/y43))/(x21-((y21*x43)/y43));
	beta=((axiscoords[2][1]-(double) points[j][1]) 
              -(axiscoords[2][0]-(double) points[j][0])*(y21/x21))/(y43-((x43*y21)/x21));

	if (logx) {
	    if (print2file) fprintf(FP,"%g\t",exp(-alpha*(rlc[1]-rlc[0])+rlc[0]));
	    else printf("%g\t",exp(-alpha*(rlc[1]-rlc[0])+rlc[0]));
	}
	else {
	    if (print2file) fprintf(FP,"%f\t",-alpha*(rlc[1]-rlc[0])+rlc[0]);
	    else printf("%f\t",-alpha*(rlc[1]-rlc[0])+rlc[0]);	
	}
	if (logy) {					/* If logarithmic recalculate before */
	    if (print2file) fprintf(FP,"%g\n",exp(-beta*(rlc[3]-rlc[2])+rlc[2]));
	    else printf("%g\n",exp(-beta*(rlc[3]-rlc[2])+rlc[2]));
	}
	else {
	    if (print2file) fprintf(FP,"%f\n",-beta*(rlc[3]-rlc[2])+rlc[2]);
	    else printf("%f\n",-beta*(rlc[3]-rlc[2])+rlc[2]);
	}
    }
    print2file=FALSE;
}


/****************************************************************/
/* This function removes the last inserted point or the point	*/
/* indexed by remthis (<0).					*/
/****************************************************************/
void remove_last(GtkWidget *widget, gpointer data) 
{
gint x,y,i;

    gc = gdk_gc_new (drawing_area->window);				/* Create graphics context */

/* First redraw the drawing_area with the original image, to clean it. */
    gdk_draw_pixmap(drawing_area->window,widget->style->white_gc,
		    pixmap,0,0,0,0,XSize,YSize);

    if (numlastpoints>0) {				/* If points been put out, remove last one */
	if (remthis==0) {				/* If remthis is 0, ignore it.		*/
	numlastpoints--;
	if (lastpoints[numlastpoints]==-1) {		/* If point to be removed is axispoint 1 */
	    bpressed[0]=FALSE;				/* Mark it unpressed.			*/
	    gtk_widget_set_sensitive(x1_entry,FALSE);	/* Inactivate entry for point.		*/
	}
	else if (lastpoints[numlastpoints]==-2) {	/* If point to be removed is axispoint 2 */
	    bpressed[1]=FALSE;				/* Mark it unpressed.                   */
	    gtk_widget_set_sensitive(x2_entry,FALSE);	/* Inactivate entry for point.          */
	}
	else if (lastpoints[numlastpoints]==-3) {	/* If point to be removed is axispoint 3 */
	    bpressed[2]=FALSE;				/* Mark it unpressed.                   */
	    gtk_widget_set_sensitive(y1_entry,FALSE);	/* Inactivate entry for point.          */
	}
	else if (lastpoints[numlastpoints]==-4) {	/* If point to be removed is axispoint 4 */
	    bpressed[3]=FALSE;				/* Mark it unpressed.                   */
	    gtk_widget_set_sensitive(y2_entry,FALSE);	/* Inactivate entry for point.          */
	}
	else numpoints--;				/* If ordinary point, decrease count.	*/
	}

	if (numlastpoints>0) {				/* If more than 0 points left, start to redraw */
	    for (i=0;i<numlastpoints;i++) {		/* Loop over all remaining points.	*/
		if (lastpoints[i]==-1 && remthis!=-1) { /* If x axis point 1 isn't removed, draw it. */
		    gdk_gc_set_foreground (gc, &colors[0]);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[0][0]-MarkerLength,
					axiscoords[0][1]-MarkerThick/2,MarkerLength*2+1,MarkerThick+1);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[0][0]-MarkerThick/2,
					axiscoords[0][1]-MarkerLength,MarkerThick+1,MarkerLength+1);
		}
		else if (lastpoints[i]==-2 && remthis!=-2) {	/* If x axis point 2 isn't removed, draw it. */
		    gdk_gc_set_foreground (gc, &colors[0]);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[1][0]-MarkerLength,
					axiscoords[1][1]-MarkerThick/2,MarkerLength*2+1,MarkerThick+1);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[1][0]-MarkerThick/2,
					axiscoords[1][1]-MarkerLength,MarkerThick+1,MarkerLength+1);
		}
		else if (lastpoints[i]==-3 && remthis!=-3) {	/* If x axis point 3 isn't removed, draw it. */
		    gdk_gc_set_foreground (gc, &colors[1]);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[2][0]-MarkerThick/2,
					axiscoords[2][1]-MarkerLength,MarkerThick+1,MarkerLength*2+1);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[2][0],
					axiscoords[2][1]-MarkerThick/2,MarkerLength+1,MarkerThick+1);
		}
		else if (lastpoints[i]==-4 && remthis!=-4) {	/* If x axis point 4 isn't removed, draw it. */
		    gdk_gc_set_foreground (gc, &colors[1]);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[3][0]-MarkerThick/2,
					axiscoords[3][1]-MarkerLength,MarkerThick+1,MarkerLength*2+1);
		    gdk_draw_rectangle(drawing_area->window,gc, TRUE, axiscoords[3][0],
					axiscoords[3][1]-MarkerThick/2,MarkerLength+1,MarkerThick+1);
		}
		else if (lastpoints[i]>=0) {		/* Else draw the graphpoints */
		    x=points[lastpoints[i]][0];		/* Get coordinates */
		    y=points[lastpoints[i]][1];

/* Redraw square */
	    	    gdk_gc_set_foreground (gc, &colors[2]);	/* Set color of graphics context */
		    gdk_draw_rectangle(drawing_area->window,gc,
					TRUE,x-OuterSize,y-OuterSize,OuterSize*2+1,
					OuterSize*2+1);
		    gdk_gc_set_foreground (gc, &colors[3]);	/* Set color of graphics context */
		    gdk_draw_rectangle(drawing_area->window,gc,
					TRUE,x-InnerSize,y-InnerSize,InnerSize*2+1,
					InnerSize*2+1);
		}
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
	gtk_widget_set_sensitive(printbutton,FALSE);
	gtk_widget_set_sensitive(fileoutputb,FALSE);
    }
    remthis=0;						/* Reset remthis variable */
    gdk_gc_unref(gc);					/* Kill graphics context */
}


/****************************************************************/
/* This function sets the proper variables and then calls 	*/
/* remove_last, to remove all points except the axis points.	*/
/****************************************************************/
void remove_all(GtkWidget *widget, gpointer data) 
{
gint i, index;

    if (numlastpoints>0 && numpoints>0) {
	index=0;
	for (i=0;i<numlastpoints;i++) {		/* Search for axispoints and store them in */
	    if (lastpoints[i]==-1) {		/* lastpoints at the first positions.	   */
		lastpoints[index]=-1;
		index++;
	    }
	    if (lastpoints[i]==-2) {
		lastpoints[index]=-2;
		index++;
	    }
	    if (lastpoints[i]==-3) {
		lastpoints[index]=-3;
		index++;
	    }
	    if (lastpoints[i]==-4) {
		lastpoints[index]=-4;
		index++;
	    }
	}
	lastpoints[index]=0;

	numlastpoints=index+1;
	numpoints=1;

	remove_last(widget,data);		/* Call remove_last() */
    }
    else if (numlastpoints>0 && numpoints==0) {
	numlastpoints=0;			/* Nullify amount of points */
	remove_last(widget,data);		/* Call remove_last() */
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
    FP = fopen(file_name,"w");			/* Open file for writing */
    if (FP==NULL) printf("Could not open %s for writing\n",file_name); /* If unable to open print error */
    else {					/* If open successfull */
	print2file=TRUE;			/* Mark printing to file */
	print_results(widget,data);		/* Call print_results() */
	fclose(FP);				/* Close the file */
    }
}


/****************************************************************/
/* This function initializes the colors which are used when 	*/
/* drawing the axispoints and the graph points			*/
/****************************************************************/
gboolean setcolors()
{
gboolean *success;
gint i, ncolors;

gushort xcolor[19][4] = {{40000,20000,48000},		/* x axis marker color */
			 {40000,48000,20000},		/* y axis marker color */
			 {65535,00000,00000},		/* outer square color */
			 {65535,65535,65535}};		/* inner square color */

    ncolors=4;						/* Number of colors to be initialized */
    colors = (GdkColor *) calloc (ncolors, sizeof(GdkColor));	/* Allocate memory for the colors */
    success = (gboolean *) calloc (ncolors, sizeof(gboolean));

    for(i=0;i<ncolors;i++) {				/* Transfer colorindexes into colors array */
	(colors)[i].red = xcolor[i][0];
        (colors)[i].green = xcolor[i][1];
        (colors)[i].blue = xcolor[i][2];
    }

/* Allocate the colors */
    gdk_colormap_alloc_colors(gdk_colormap_get_system(),colors, ncolors, FALSE, FALSE, success);
    free(success);

return TRUE;
}


/****************************************************************/
/* This is the main function, this function gets called when	*/
/* the program is executed. It allocates the necessary work-	*/
/* spaces and initialized the main window and its widgets.	*/
/****************************************************************/
int main (int argc, char *argv[] )
{
char buf[100];								/* Text buffer for window title */
GtkWidget *hbox,*vbox,*hbox2,*vbox2,*table,*table2;			/* GTK box variables for packing */
GtkWidget *x1_label,*x2_label,*y1_label,*y2_label;			/* Labels for texts in window */
GtkWidget *orderchecknot,*ordercheckx,*orderchecky;			/* Radiobuttons for ordering */
GtkWidget *logxcheck,*logycheck,*frame;					/* Logarithmic checkbuttons and frame */
GtkWidget *x_label,*y_label,*file_label;
GtkTooltips *tooltip;
GSList *group;								/* Group for the radiobuttons */
GdkBitmap *mask;
gint FileIndex, i, maxX, maxY, newX, newY;
double Scale, mScale;
GdkPixbuf       *loadgpbimage;

    if (argc<2) {							/* If program gets less then two arguments */
	printf("\n g3data version "VERSION", Copyright (C) 2000 Jonas Frantz\n");
	printf(" g3data comes with ABSOLUTELY NO WARRANTY; for details\n");
	printf(" check out the documentation.  This is free software, and\n");
	printf(" you are welcome to redistribute it under all conditions.\n\n\n");
	printf("  Usage : g3data [options] file.ext\n\n");
	printf("  Options :\n");
	printf("      -max x y      : If picture is larger x-wise than x or y-wise than y,\n");
	printf("                      the picture is scaled down accordingly.\n");
	printf("      -scale factor : Scales the picture size by factor. This parameter\n");
	printf("                      nullifies effect of the -max parameter.\n\n");
	printf("  Supported image formats : JPEG, GIF, PPM, PGM, XPM, PNG, TIFF, EIM\n\n");
	printf(" Report bugs to jonas.frantz@helsinki.fi\n\n");
	exit(0);
    }
	
    maxX = -1;
    maxY = -1;
    Scale = -1;
    FileIndex = -1;
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

    gtk_init (&argc, &argv);						/* Init GTK */
    gdk_init(&argc,&argv);						/* Init GDK */

    sprintf(buf,"g3data "VERSION" : %s",argv[FileIndex]);		/* Print window title in buffer */

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);			/* Create window */
    gtk_window_set_title (GTK_WINDOW (window), buf);			/* Set window title */
    gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, TRUE);	/* Init window */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);	/* Set borders in window */

    gtk_signal_connect (GTK_OBJECT (window), "delete_event",		/* Init delete event of window */
                        GTK_SIGNAL_FUNC (close_application), NULL);

    table = gtk_table_new(2, 2 ,FALSE);					/* Create table */
    gtk_table_set_row_spacings(GTK_TABLE(table),5);			/* Set spacings */
    gtk_table_set_col_spacings(GTK_TABLE(table),5);
    table2 = gtk_table_new(2, 2 ,FALSE);				/* Create table */
    gtk_table_set_row_spacings(GTK_TABLE(table2),5);			/* Set spacings */
    gtk_table_set_col_spacings(GTK_TABLE(table2),5);

    hbox = gtk_hbox_new (FALSE, 10);					/* Create horisontal box */
    vbox = gtk_vbox_new (FALSE, 5);					/* Create vertical box */

    x1_entry = gtk_entry_new_with_max_length(20);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(x1_entry), FALSE);			/* Make it editable */
    gtk_widget_set_sensitive(x1_entry,FALSE);				/* Inactivate it */
    gtk_signal_connect_object (GTK_OBJECT (x1_entry), "changed",	/* Init the entry to call */
                    GTK_SIGNAL_FUNC (read_x1_entry),NULL);		/* read_x1_entry whenever */
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,x1_entry,entryx1tt,NULL);
	
/* its value is changed. */
    y1_entry = gtk_entry_new_with_max_length(20);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(y1_entry), FALSE);			/* Make it editable */
    gtk_widget_set_sensitive(y1_entry,FALSE);				/* Inactivate it */
    gtk_signal_connect_object (GTK_OBJECT (y1_entry), "changed",	/* Init the entry to call */
                    GTK_SIGNAL_FUNC (read_y1_entry),NULL);		/* read_y1_entry whenever */
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,y1_entry,entryy1tt,NULL);
									/* its value is changed.  */
    x2_entry = gtk_entry_new_with_max_length(20);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(x2_entry), FALSE);			/* Make it editable */
    gtk_widget_set_sensitive(x2_entry,FALSE);				/* Inactivate it */
    gtk_signal_connect_object (GTK_OBJECT (x2_entry), "changed",	/* Init the entry to call */
                    GTK_SIGNAL_FUNC (read_x2_entry),NULL);		/* read_x2_entry whenever */
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,x2_entry,entryx2tt,NULL);
									/* its value is changed.  */
    y2_entry = gtk_entry_new_with_max_length(20);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(y2_entry), FALSE);			/* Make it editable */
    gtk_widget_set_sensitive(y2_entry,FALSE);				/* Inactivate it */
    gtk_signal_connect_object (GTK_OBJECT (y2_entry), "changed",	/* Init the entry to call */
                    GTK_SIGNAL_FUNC (read_y2_entry),NULL);		/* read_y2_entry whenever */
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,y2_entry,entryy2tt,NULL);
									/* its value is changed.  */

    x_label = gtk_label_new(" X : ");
    y_label = gtk_label_new(" Y : ");
    xc_entry = gtk_entry_new_with_max_length(20);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(xc_entry), FALSE);			/* Make it not editable */
    yc_entry = gtk_entry_new_with_max_length(20);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(yc_entry), FALSE);			/* Make it not editable */
    
    file_label = gtk_label_new(" Filename : ");
    file_entry = gtk_entry_new_with_max_length(60);			/* Create text entry */
    gtk_entry_set_editable(GTK_ENTRY(file_entry), TRUE);		/* Make it not editable */
    gtk_signal_connect_object (GTK_OBJECT (file_entry), "changed",	/* Init the entry to call */
                    GTK_SIGNAL_FUNC (read_file_entry),NULL);		/* read_y2_entry whenever */
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,file_entry,filenamett,NULL);

    loadgpbimage = gdk_pixbuf_new_from_file(argv[FileIndex]);		/* Load image */
    if (loadgpbimage==NULL) {						/* If unable to load image */
	printf("Error : Unable to load file : %s\n",argv[FileIndex]);	/* Print error message and */
	gtk_exit(0);							/* exit */
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
			     0, 0, Scale, Scale, GDK_INTERP_HYPER, 255);
	gdk_pixbuf_unref(loadgpbimage);
    }
    else gpbimage = loadgpbimage;

    XSize = gdk_pixbuf_get_width(gpbimage);				/* Get image width */
    YSize = gdk_pixbuf_get_height(gpbimage);				/* Get image height */

    gdk_pixbuf_render_pixmap_and_mask(gpbimage, &pixmap, &mask, 255);

    drawing_area = gtk_drawing_area_new ();				/* Create new drawing area */
    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area),
			   XSize,YSize); 				/* Set drawing area size */

    zoom_area = gtk_drawing_area_new ();				/* Create new drawing area */
    gtk_drawing_area_size (GTK_DRAWING_AREA (zoom_area),
			   ZoomPixSize,ZoomPixSize); 			/* Set its size */

    setcolors();

    gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",	/* Connect drawing area to */
              (GtkSignalFunc) expose_event, NULL);			/* expose_event. */
    
    gtk_signal_connect (GTK_OBJECT (drawing_area), "configure_event",	/* Connect drawing area to */
              (GtkSignalFunc) configure_event, NULL);			/* configure_event. */

    gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",/* Connect drawing area to */
              (GtkSignalFunc) button_press_event, NULL);		/* button_press_event. */

    gtk_signal_connect (GTK_OBJECT (drawing_area), "button_release_event",/* Connect drawing area to */
              (GtkSignalFunc) button_release_event, NULL);		/* button_release_event */

    gtk_signal_connect (GTK_OBJECT (drawing_area), "motion_notify_event",/* Connect drawing area to */
              (GtkSignalFunc) motion_notify_event, NULL);		/* motion_notify_event. */

    gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK |		/* Set the events active */
			   GDK_BUTTON_PRESS_MASK | 
			   GDK_BUTTON_RELEASE_MASK |
			   GDK_POINTER_MOTION_MASK | 
			   GDK_POINTER_MOTION_HINT_MASK);

    x1_label = gtk_label_new(" X value for point 1 : ");		/* Set label */
    x2_label = gtk_label_new(" X value for point 2 : ");		/* Set label */
    y1_label = gtk_label_new(" Y value for point 1 : ");		/* Set label */
    y2_label = gtk_label_new(" Y value for point 2 : ");		/* Set label */

    setx1button = gtk_toggle_button_new_with_label("Set point 1 on X axis"); /* Create button */
    gtk_signal_connect (GTK_OBJECT (setx1button), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (toggle_x1), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,setx1button,setx1tt,NULL);

    setx2button = gtk_toggle_button_new_with_label("Set point 2 on X axis"); /* Create button */
    gtk_signal_connect (GTK_OBJECT (setx2button), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (toggle_x2), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,setx2button,setx2tt,NULL);

    sety1button = gtk_toggle_button_new_with_label("Set point 1 on Y axis"); /* Create button */
    gtk_signal_connect (GTK_OBJECT (sety1button), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (toggle_y1), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,sety1button,sety1tt,NULL);

    sety2button = gtk_toggle_button_new_with_label("Set point 2 on Y axis"); /* Create button */
    gtk_signal_connect (GTK_OBJECT (sety2button), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (toggle_y2), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,sety2button,sety2tt,NULL);

    quitbutton = gtk_button_new_with_label ("Quit");			/* Create button */
    gtk_signal_connect (GTK_OBJECT (quitbutton), "clicked",		/* Connect button */
                  GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

    printbutton = gtk_button_new_with_label ("Print results");		/* Create button */
    gtk_signal_connect (GTK_OBJECT (printbutton), "clicked",		/* Connect button */
                  GTK_SIGNAL_FUNC (print_results), NULL);
    gtk_widget_set_sensitive(printbutton,FALSE);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,printbutton,printrestt,NULL);

    remlastbutton = gtk_button_new_with_label ("Remove last point");	/* Create button */
    gtk_signal_connect (GTK_OBJECT (remlastbutton), "clicked",		/* Connect button */
                  GTK_SIGNAL_FUNC (remove_last), NULL);
    gtk_widget_set_sensitive(remlastbutton,FALSE);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,remlastbutton,removeltt,NULL);

    remallbutton = gtk_button_new_with_label ("Remove all points");	/* Create button */
    gtk_signal_connect (GTK_OBJECT (remallbutton), "clicked",		/* Connect button */
                  GTK_SIGNAL_FUNC (remove_all), NULL);
    gtk_widget_set_sensitive(remallbutton,FALSE);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,remallbutton,removeatt,NULL);

    fileoutputb = gtk_button_new_with_label ("Print data to file");	/* Create button */
    gtk_signal_connect (GTK_OBJECT (fileoutputb), "clicked",		/* Connect button */
                  GTK_SIGNAL_FUNC (fileoutput), NULL);
    gtk_widget_set_sensitive(fileoutputb,FALSE);

    orderchecknot = gtk_radio_button_new_with_label (NULL, 
					"Do not order output data"); 	/* Create radio button */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (orderchecknot), 
				  TRUE);				/* Set button active */
    gtk_signal_connect (GTK_OBJECT (orderchecknot), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (ordernot), NULL);
    group = gtk_radio_button_group (GTK_RADIO_BUTTON (orderchecknot));	/* Get buttons group */

    ordercheckx = gtk_radio_button_new_with_label (group, 
				"Order output data according to X");	/* Create radio button */
    gtk_signal_connect (GTK_OBJECT (ordercheckx), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (orderx), NULL);
    group = gtk_radio_button_group (GTK_RADIO_BUTTON (ordercheckx));	/* Get buttons group */

    orderchecky = gtk_radio_button_new_with_label(group, 
				"Order output data according to Y"); 	/* Create radio button */
    gtk_signal_connect (GTK_OBJECT (orderchecky), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (ordery), NULL);

    logxcheck = gtk_check_button_new_with_label("X axis is logaritmic");/* Create check button */
    gtk_signal_connect (GTK_OBJECT (logxcheck), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (islogx), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,logxcheck,logxtt,NULL);

    logycheck = gtk_check_button_new_with_label("Y axis is logaritmic");/* Create check button */
    gtk_signal_connect (GTK_OBJECT (logycheck), "toggled",		/* Connect button */
                  GTK_SIGNAL_FUNC (islogy), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip (tooltip,logycheck,logytt,NULL);

    gtk_container_add( GTK_CONTAINER(window),table);
    gtk_table_attach_defaults(GTK_TABLE(table), vbox, 0, 1, 0, 1);

    gtk_box_pack_start (GTK_BOX (vbox), setx1button, TRUE, FALSE, 0);	/* Pack button in vert. box */
    hbox2 = gtk_hbox_new (FALSE, 5);					/* Create new horisontal box */
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);		/* Pack hor. box in vert. box */
    gtk_box_pack_start (GTK_BOX (hbox2), x1_label, TRUE, TRUE, 0);	/* Pack label in hor. box */
    gtk_box_pack_start (GTK_BOX (hbox2), x1_entry, TRUE, TRUE, 0);	/* Pack entry in hor. box */
    gtk_box_pack_start (GTK_BOX (vbox), setx2button, TRUE, FALSE, 0);	/* Pack button in vert. box */
    gtk_widget_show(hbox2);						/* Show horisontal box */
    hbox2 = gtk_hbox_new (FALSE, 5);					/* Create new horisontal box */
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);		/* Pack hor. box in vert. box */
    gtk_box_pack_start (GTK_BOX (hbox2), x2_label, TRUE, TRUE, 0);	/* Pack label in hor. box */
    gtk_box_pack_start (GTK_BOX (hbox2), x2_entry, TRUE, TRUE, 0);	/* Pack entry in hor. box */
    gtk_box_pack_start (GTK_BOX (vbox), sety1button, TRUE, FALSE, 0);	/* Pack button in vert. box */
    gtk_widget_show(hbox2);						/* Show horisontal box */
    hbox2 = gtk_hbox_new (FALSE, 5);					/* Create new horisontal box */
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);		/* Pack hor. box in vert. box */
    gtk_box_pack_start (GTK_BOX (hbox2), y1_label, TRUE, TRUE, 0);	/* Pack label in hor. box */
    gtk_box_pack_start (GTK_BOX (hbox2), y1_entry, TRUE, TRUE, 0);	/* Pack entry in hor. box */
    gtk_box_pack_start (GTK_BOX (vbox), sety2button, TRUE, FALSE, 0);	/* Pack button in vert. box */
    gtk_widget_show(hbox2);						/* Show horisontal box */
    hbox2 = gtk_hbox_new (FALSE, 5);					/* Create new horisontal box */
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);		/* Pack hor. box in vert. box */
    gtk_box_pack_start (GTK_BOX (hbox2), y2_label, TRUE, TRUE, 0);	/* Pack label in hor. box */
    gtk_box_pack_start (GTK_BOX (hbox2), y2_entry, TRUE, TRUE, 0);	/* Pack entry in hor. box */
    gtk_widget_show(hbox2);						/* Show horisontal box */
    frame = gtk_frame_new("Zoom in");					/* Create frame */
    hbox2 = gtk_hbox_new (FALSE, 10);					/* Create new horisontal box */
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 10);		/* Pack frame in vert. box */
    gtk_container_add (GTK_CONTAINER (frame), hbox2);			/* Pack hor. box in frame */
    gtk_box_pack_start (GTK_BOX (hbox2), zoom_area, FALSE, FALSE, 35);	/* Pack zoom area in hor. box */
    gtk_widget_show(hbox2);						/* Show horisontal box */
    gtk_box_pack_start (GTK_BOX (vbox), orderchecknot, TRUE, FALSE, 0);	/* Pack radiobutton in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox), ordercheckx, TRUE, FALSE, 0);	/* Pack radiobutton in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox), orderchecky, TRUE, FALSE, 0);	/* Pack radiobutton in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox), logxcheck, TRUE, FALSE, 0);	/* Pack checkbutton in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox), logycheck, TRUE, FALSE, 0);	/* Pack checkbutton in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox), remlastbutton, TRUE, FALSE, 0);	/* Pack button in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox), remallbutton, TRUE, FALSE, 0);	/* Pack button in vert. box */
    vbox2 = gtk_vbox_new (FALSE, 10);
    gtk_table_attach_defaults(GTK_TABLE(table), vbox2, 0, 1, 1, 2);
    gtk_box_pack_start (GTK_BOX (vbox2), printbutton, FALSE, FALSE, 0);	/* Pack button in vert. box */
    gtk_box_pack_start (GTK_BOX (vbox2), quitbutton, FALSE, FALSE, 0);	/* Pack button in vert. box */
    gtk_widget_show(vbox2);						/* Show horisontal box */

    gtk_table_attach_defaults(GTK_TABLE(table), drawing_area, 1, 2, 0, 1);

    gtk_table_attach_defaults(GTK_TABLE(table), table2, 1, 2, 1, 2);
    hbox2 = gtk_hbox_new (FALSE, 10);
    gtk_table_attach_defaults(GTK_TABLE(table2), x_label, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table2), hbox2, 1, 2, 0, 1);
    gtk_box_pack_start (GTK_BOX (hbox2), xc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox2), y_label, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox2), yc_entry, TRUE, TRUE, 0);
    gtk_widget_show(hbox2);						/* Show horisontal box */
    hbox2 = gtk_hbox_new (FALSE, 10);
    gtk_table_attach_defaults(GTK_TABLE(table2), file_label, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table2), hbox2, 1, 2, 1, 2);
    gtk_box_pack_start (GTK_BOX (hbox2), file_entry, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox2), fileoutputb, TRUE, TRUE, 0);
    gtk_widget_show(hbox2);						/* Show horisontal box */

/* Show all widgets not shown already */
    gtk_widget_show(setx1button);
    gtk_widget_show(setx2button);
    gtk_widget_show(sety1button);
    gtk_widget_show(sety2button);
    gtk_widget_show(orderchecknot);
    gtk_widget_show(ordercheckx);
    gtk_widget_show(orderchecky);
    gtk_widget_show(logxcheck);
    gtk_widget_show(logycheck);
    gtk_widget_show(remlastbutton);
    gtk_widget_show(remallbutton);
    gtk_widget_show(printbutton);
    gtk_widget_show(fileoutputb);
    gtk_widget_show(quitbutton);
    gtk_widget_show(x1_label);
    gtk_widget_show(x2_label);
    gtk_widget_show(y1_label);
    gtk_widget_show(y2_label);
    gtk_widget_show(x1_entry);
    gtk_widget_show(y1_entry);
    gtk_widget_show(x2_entry);
    gtk_widget_show(y2_entry);
    gtk_widget_show(frame);
    gtk_widget_show(zoom_area);
    gtk_widget_show(drawing_area);
    gtk_widget_show(xc_entry);
    gtk_widget_show(yc_entry);
    gtk_widget_show(x_label);
    gtk_widget_show(y_label);
    gtk_widget_show(file_entry);
    gtk_widget_show(file_label);
    gtk_widget_show(hbox);
    gtk_widget_show(vbox);
    gtk_widget_show(table);
    gtk_widget_show(table2);
    gtk_widget_show(window);

    gtk_main();				/* This is where it all starts */
              
    free(colors);			/* Deallocate memory */

    return(0);				/* Exit. */
}
