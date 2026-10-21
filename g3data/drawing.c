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

#include <gtk/gtk.h>
#include "main.h"
#include "drawing.h"

#define MARKERLENGTH 6					/* Axis marker length */
#define MARKERTHICKNESS 2				/* Axis marker thickness */

/* Marker colors: X axis, Y axis, points (same values as the old
 * 16-bit GdkColor table, divided by 65535) */
static const GdkRGBA marker_colors[] = {
    {40000/65535.0, 20000/65535.0, 48000/65535.0, 1.0},
    {40000/65535.0, 48000/65535.0, 20000/65535.0, 1.0},
    {1.0, 0.0, 0.0, 1.0}
};

/****************************************************************/
/* This function draws the X-axis, Y-axis or point marker on	*/
/* the drawing_area depending on the value of the type		*/
/* parameter.							*/
/****************************************************************/
void DrawMarker(cairo_t *cr, gint x, gint y, gint type)
{
    if (type < 0 || type > 2)
        return;
    gdk_cairo_set_source_rgba(cr, &marker_colors[type]);
    if (type == 0) {
        cairo_move_to(cr, x - MARKERLENGTH, y);
        cairo_rel_line_to(cr, 2 * MARKERLENGTH, 0);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, -MARKERLENGTH);
        cairo_stroke(cr);
    } else if (type == 1) {
        cairo_move_to(cr, x, y - MARKERLENGTH);
        cairo_rel_line_to(cr, 0, 2 * MARKERLENGTH);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, MARKERLENGTH, 0);
        cairo_stroke(cr);
    } else {
        cairo_rectangle(cr, x - MARKERLENGTH / 2, y - MARKERLENGTH / 2, MARKERLENGTH, MARKERLENGTH);
        cairo_stroke(cr);
    }
}
