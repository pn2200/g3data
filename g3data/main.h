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

#define ZOOMPIXSIZE 200					/* Size of zoom in window */
#define ZOOMFACTOR 4					/* Zoom factor of zoom window */
#define MAXPOINTS 256					/* Maximum number of points per memory allocation */
#define MAXNUMTABS 128					/* Maximum number of open tabs */
#define MOVETRESHOLD 3
#define ACTIONBNUM 2
#define ORDERBNUM 3
#define LOGBNUM 2

#define URI_IDENTIFIER "file://"

struct PointValue {
  double Xv,Yv,Xerr,Yerr;
  double eXplus,eXminus,eYplus,eYminus;
};

typedef enum {
    URI_LIST,
    PNG_DATA,
    JPEG_DATA,
    APP_X_COLOR,
    NUM_IMAGE_DATA,
} UI_DROP_TARGET_INFO;
