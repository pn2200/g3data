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
#include <stdlib.h>
#include "main.h"

/**********************************/
/* Sort points by x- or y-values  */
/**********************************/
gint compare_x(const void *a, const void *b)
{
    const struct PointValue *da = (const struct PointValue *) a;
    const struct PointValue *db = (const struct PointValue *) b;
    
    return (da->Xv > db->Xv) - (da->Xv < db->Xv);
}


gint compare_y(const void *a, const void *b)
{
    const struct PointValue *da = (const struct PointValue *) a;
    const struct PointValue *db = (const struct PointValue *) b;
    
    return (da->Yv > db->Yv) - (da->Yv < db->Yv);
}


void Order(struct PointValue *RealPos, gint count, gint ordering)
{
    /* Sort by x-values */
    if (ordering == 1) {
        qsort(RealPos, count, sizeof (struct PointValue), compare_x);
    } else {
    /* Sort by y-values */
        qsort(RealPos, count, sizeof (struct PointValue), compare_y);
    }
}
