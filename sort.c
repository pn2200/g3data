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

/****************************************************************/
/* The following sorting functions is a copy of the example in  */
/* K & R C programming 2nd ed. page 120. Some sort of quicksort.*/
/****************************************************************/
void swap3(struct PointValue *RealPos, gint i, gint j)
{
  struct PointValue tmp;

    tmp.Xv = RealPos[i].Xv;
    tmp.Yv = RealPos[i].Yv;
    tmp.Xerr = RealPos[i].Xerr;
    tmp.Yerr = RealPos[i].Yerr;
    RealPos[i].Xv = RealPos[j].Xv;
    RealPos[i].Yv = RealPos[j].Yv;
    RealPos[i].Xerr = RealPos[j].Xerr;
    RealPos[i].Yerr = RealPos[j].Yerr;
    RealPos[j].Xv = tmp.Xv;
    RealPos[j].Yv = tmp.Yv;
    RealPos[j].Xerr = tmp.Xerr;
    RealPos[j].Yerr = tmp.Yerr;
}


gint compare3(struct PointValue *RealPos, gint i, gint j, gint orderv)
{
    if (orderv == 1) {
	if (RealPos[i].Xv < RealPos[j].Xv) return (-1);
	else return (1);
    } else {
	if (RealPos[i].Yv < RealPos[j].Yv) return (-1);
	else return (1);
    }
}


void Order(struct PointValue *RealPos, gint left, gint right, gint ordering)
{
  gint i,last;

    if (left>=right) return;
 
    swap3(RealPos, left, (left+right)/2);
   
    last = left;
 
    for (i=left+1;i<=right;i++) if (compare3(RealPos,i,left,ordering)<0) swap3(RealPos, ++last, i);
          
    swap3(RealPos, left, last);
    Order(RealPos, left, last-1, ordering);
    Order(RealPos, last+1, right, ordering);
}
