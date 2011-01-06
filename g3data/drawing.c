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
#include "drawing.h"

#define MARKERLENGTH 6					/* Axis marker length */
#define MARKERTHICKNESS 2				/* Axis marker thickness */

/****************************************************************/
/* This function draws the X-axis, Y-axis or point marker on	*/
/* the drawing_area depending on the value of the type		*/
/* parameter.							*/
/****************************************************************/
void DrawMarker(cairo_t *cr, gint x, gint y, gint type, GdkColor *color) 
{
    if (type == 0) {
        gdk_cairo_set_source_color(cr, &color[0]);
        cairo_move_to(cr, x - MARKERLENGTH, y);
        cairo_rel_line_to(cr, 2 * MARKERLENGTH, 0);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, -MARKERLENGTH);
        cairo_stroke(cr);
    } else if (type == 1) {
        gdk_cairo_set_source_color(cr, &color[1]);
        cairo_move_to(cr, x, y - MARKERLENGTH);
        cairo_rel_line_to(cr, 0, 2 * MARKERLENGTH);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, MARKERLENGTH, 0);
        cairo_stroke(cr);
    } else if (type == 2) {
        gdk_cairo_set_source_color(cr, &color[2]);
        cairo_rectangle(cr, x - MARKERLENGTH / 2, y - MARKERLENGTH / 2, MARKERLENGTH, MARKERLENGTH);
        cairo_stroke(cr);
    }
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
