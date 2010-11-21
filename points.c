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

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include "main.h"

/* Extern functions */

extern	void	Order(struct PointValue *RealPos, gint left, gint right, gint ordering);

/* Extern variables */

extern	gint		axiscoords[MAXNUMTABS][4][2];
extern	gint		**points[MAXNUMTABS];
extern	gint		numpoints[MAXNUMTABS];
extern	gint		ordering[MAXNUMTABS];
extern	gdouble		realcoords[MAXNUMTABS][4];
extern	gboolean	UseErrors[MAXNUMTABS];
extern	gboolean	logxy[MAXNUMTABS][2];
extern	gint		Action[MAXNUMTABS];
extern  gchar		*file_name[MAXNUMTABS];
extern	gint		ViewedTabNum;

/****************************************************************/
/* This function sets the numpoints entry to numpoints variable	*/
/* value.							*/
/****************************************************************/
void SetNumPointsEntry(GtkWidget *np_entry, gint np)
{
  char buf[128];

    sprintf(buf,"%d",np);
    gtk_entry_set_text(GTK_ENTRY(np_entry),buf);
}


/****************************************************************/
/* This function returns the integer with the lesser value.	*/
/****************************************************************/
gint min(gint x, gint y)
{
    if (x < y) return x;
    else return y;
}


/****************************************************************/
/* This function calculates the true value of the point based	*/
/* on the coordinates of the point on the bitmap.		*/
/****************************************************************/
struct PointValue CalcPointValue(gint Xpos, gint Ypos, gint TabNum)
{
  double alpha,beta,x21,x43,y21,y43,rlc[4];				/* Declare help variables */
  struct PointValue PV;

    x21=(double) axiscoords[TabNum][1][0]-axiscoords[TabNum][0][0];			/* Calculate deltax of x axis points */
    y21=(double) axiscoords[TabNum][1][1]-axiscoords[TabNum][0][1];			/* Calculate deltay of x axis points */
    x43=(double) axiscoords[TabNum][3][0]-axiscoords[TabNum][2][0];			/* Calculate deltax of y axis points */
    y43=(double) axiscoords[TabNum][3][1]-axiscoords[TabNum][2][1];			/* Calculate deltay of y axis points */

    if (logxy[TabNum][0]) {							/* If x axis is logarithmic, store	*/
	rlc[0]=log(realcoords[TabNum][0]);					/* recalculated values in rlc.		*/ 
	rlc[1]=log(realcoords[TabNum][1]);
    }
    else {
	rlc[0]=realcoords[TabNum][0];						/* Else store old values in rlc.	*/
	rlc[1]=realcoords[TabNum][1];
    }

    if (logxy[TabNum][1]) {
	rlc[2]=log(realcoords[TabNum][2]);					/* If y axis is logarithmic, store      */
	rlc[3]=log(realcoords[TabNum][3]);					/* recalculated values in rlc.          */
    }
    else {
	rlc[2]=realcoords[TabNum][2];						/* Else store old values in rlc.        */
	rlc[3]=realcoords[TabNum][3];
    }

	alpha=((axiscoords[TabNum][0][0]-(double) Xpos)
              -(axiscoords[TabNum][0][1]-(double)  Ypos)*(x43/y43))/(x21-((y21*x43)/y43));
        beta=((axiscoords[TabNum][2][1]-(double) Ypos)
              -(axiscoords[TabNum][2][0]-(double) Xpos)*(y21/x21))/(y43-((x43*y21)/x21));

	if (logxy[TabNum][0]) PV.Xv = exp(-alpha*(rlc[1]-rlc[0])+rlc[0]);
	else PV.Xv = -alpha*(rlc[1]-rlc[0])+rlc[0];

	if (logxy[TabNum][1]) PV.Yv = exp(-beta*(rlc[3]-rlc[2])+rlc[2]);
	else PV.Yv = -beta*(rlc[3]-rlc[2])+rlc[2];

	alpha=((axiscoords[TabNum][0][0]-(double) (Xpos+1))
              -(axiscoords[TabNum][0][1]-(double) (Ypos+1))*(x43/y43))/(x21-((y21*x43)/y43));
        beta=((axiscoords[TabNum][2][1]-(double) (Ypos+1))
              -(axiscoords[TabNum][2][0]-(double) (Xpos+1))*(y21/x21))/(y43-((x43*y21)/x21));

	if (logxy[TabNum][0]) PV.Xerr = exp(-alpha*(rlc[1]-rlc[0])+rlc[0]);
	else PV.Xerr = -alpha*(rlc[1]-rlc[0])+rlc[0];

	if (logxy[TabNum][1]) PV.Yerr = exp(-beta*(rlc[3]-rlc[2])+rlc[2]);
	else PV.Yerr = -beta*(rlc[3]-rlc[2])+rlc[2];

	alpha=((axiscoords[TabNum][0][0]-(double) (Xpos-1))
              -(axiscoords[TabNum][0][1]-(double) (Ypos-1))*(x43/y43))/(x21-((y21*x43)/y43));
        beta=((axiscoords[TabNum][2][1]-(double) (Ypos-1))
              -(axiscoords[TabNum][2][0]-(double) (Xpos-1))*(y21/x21))/(y43-((x43*y21)/x21));

	if (logxy[TabNum][0]) PV.Xerr -= exp(-alpha*(rlc[1]-rlc[0])+rlc[0]);
	else PV.Xerr -= -alpha*(rlc[1]-rlc[0])+rlc[0];

	if (logxy[TabNum][1]) PV.Yerr -= exp(-beta*(rlc[3]-rlc[2])+rlc[2]);
	else PV.Yerr -= -beta*(rlc[3]-rlc[2])+rlc[2];

	PV.Xerr = fabs(PV.Xerr/4.0);
	PV.Yerr = fabs(PV.Yerr/4.0);

    return PV;
}


/****************************************************************/
/* This function is called when the "Print results" button is	*/
/* pressed, it calculate the values of the datapoints and 	*/
/* prints them through stdout.					*/
/****************************************************************/
void print_results(GtkWidget *widget, gpointer func_data) 
{
  gint i;								/* Declare index variable */
  gboolean print2file;
  FILE *FP;

  struct PointValue *RealPos, CalcVal;

    print2file = Action[ViewedTabNum];

    if (print2file == PRINT2FILE) {
	FP = fopen(file_name[ViewedTabNum],"w");					/* Open file for writing */
	if (FP==NULL) {
	    printf("Could not open %s for writing\n",file_name[ViewedTabNum]);	/* If unable to open print error */
	    return;
	}
    }

    RealPos = (struct PointValue *) malloc(sizeof(struct PointValue) * numpoints[ViewedTabNum]);

/* Next up is recalculating the positions of the points by solving a 2*2 matrix */

    for (i=0;i<numpoints[ViewedTabNum];i++) {
        CalcVal = CalcPointValue(points[ViewedTabNum][i][0], points[ViewedTabNum][i][1], ViewedTabNum);
	RealPos[i].Xv = CalcVal.Xv;
	RealPos[i].Yv = CalcVal.Yv;
	RealPos[i].Xerr = CalcVal.Xerr;
	RealPos[i].Yerr = CalcVal.Yerr;
    }

    if (ordering[ViewedTabNum] != 0) {
	Order(RealPos, 0, numpoints[ViewedTabNum]-1, ordering[ViewedTabNum]);
    }

/* Print results to stdout or file */

    for (i=0;i<numpoints[ViewedTabNum];i++) {
	if (print2file == PRINT2FILE) {
	    fprintf(FP,"%.12g  %.12g", RealPos[i].Xv, RealPos[i].Yv);
	    if (UseErrors[ViewedTabNum]) {
		fprintf(FP,"\t%.12g  %.12g\n", RealPos[i].Xerr, RealPos[i].Yerr);
	    } else fprintf(FP,"\n");
	}
	else {
	    printf("%.12g  %.12g",RealPos[i].Xv, RealPos[i].Yv);
	    if (UseErrors[ViewedTabNum]) {
		printf("\t%.12g  %.12g\n", RealPos[i].Xerr, RealPos[i].Yerr);
	    } else printf("\n");
	}
    }
    free(RealPos);

    if (print2file == PRINT2FILE) fclose(FP);
}
