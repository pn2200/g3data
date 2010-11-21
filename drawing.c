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

#include <stdlib.h>
#include <gtk/gtk.h>
#include "main.h"

/****************************************************************/
/* This function draws the X-axis, Y-axis or point marker on	*/
/* the drawing_area depending on the value of the type		*/
/* parameter.							*/
/****************************************************************/
void DrawMarker(GtkWidget *da, gint x, gint y, gint type, GdkColor *color) 
{
  GdkGC *bpegc;

    bpegc = gdk_gc_new (da->window);

    if (type==0) {
	gdk_gc_set_foreground (bpegc, &color[0]);
	gdk_draw_rectangle(da->window,bpegc,TRUE,
			   x-MARKERLENGTH,y-MARKERTHICKNESS/2,MARKERLENGTH*2+1,MARKERTHICKNESS+1);
	gdk_draw_rectangle(da->window,bpegc,TRUE,
			   x-MARKERTHICKNESS/2,y-MARKERLENGTH,MARKERTHICKNESS+1,MARKERLENGTH+1);
    } else if (type==1) {
	gdk_gc_set_foreground (bpegc, &color[1]);
	gdk_draw_rectangle(da->window,bpegc,TRUE,
			   x-MARKERTHICKNESS/2,y-MARKERLENGTH,MARKERTHICKNESS+1,MARKERLENGTH*2+1);
	gdk_draw_rectangle(da->window,bpegc,TRUE,
			   x,y-MARKERTHICKNESS/2,MARKERLENGTH+1,MARKERTHICKNESS+1);
    } else {
	gdk_gc_set_foreground (bpegc, &color[2]);
	gdk_draw_rectangle(da->window,bpegc,TRUE,
			   x-OUTERSIZE,y-OUTERSIZE,OUTERSIZE*2+1,OUTERSIZE*2+1);
	gdk_gc_set_foreground (bpegc, &color[3]);
	gdk_draw_rectangle(da->window,bpegc,TRUE,
			   x-INNERSIZE,y-INNERSIZE,INNERSIZE*2+1,INNERSIZE*2+1);
    }
    g_object_unref(bpegc);
}


/****************************************************************/
/* This function initializes the colors which are used when 	*/
/* drawing the axispoints and the graph points			*/
/****************************************************************/
gboolean setcolors(GdkColor **color)
{
  gboolean *success;
  gint i, ncolors;
  gushort xcolor[19][4] = {{40000,20000,48000},				/* x axis marker color */
			   {40000,48000,20000},				/* y axis marker color */
			   {65535,00000,00000},				/* outer square color */
			   {65535,65535,65535}};			/* inner square color */

    ncolors=4;								/* Number of colors to be initialized */
    *color = (GdkColor *) calloc (ncolors, sizeof(GdkColor));		/* Allocate memory for the colors */
    success = (gboolean *) calloc (ncolors, sizeof(gboolean));

    for(i=0;i<ncolors;i++) {						/* Transfer colorindexes into colors array */
	(*color)[i].red = xcolor[i][0];
        (*color)[i].green = xcolor[i][1];
        (*color)[i].blue = xcolor[i][2];
    }

/* Allocate the colors */
    gdk_colormap_alloc_colors(gdk_colormap_get_system(),*color, ncolors, FALSE, FALSE, success);
    free(success);

  return TRUE;
}
