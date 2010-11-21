#define G_DISABLE_DEPRECATED
#define GDK_DISABLE_DEPRECATED
#define GDK_PIXBUF_DISABLE_DEPRECATED
#define GTK_DISABLE_DEPRECATED

#define ZOOMPIXSIZE 200					/* Size of zoom in window */
#define ZOOMFACTOR 4					/* Zoom factor of zoom window */
#define INNERSIZE 1					/* Size of point marker white inner square */
#define OUTERSIZE 3					/* Size of point marker red outer square */
#define MARKERLENGTH 6					/* Axis marker length */
#define MARKERTHICKNESS 2				/* Axis marker thickness */
#define MAXPOINTS 256					/* Maximum number of points per memory allocation */
#define MAXNUMTABS 128					/* Maximum number of open tabs */
#define VERSION "1.5.0"					/* Version number */


#define ACTIONBNUM 2
#define ORDERBNUM 3
#define LOGBNUM 2

struct PointValue {
  gdouble 	Xv,Yv,Xerr,Yerr;
};

enum ACTION {
  PRINT2STDOUT = 0,
  PRINT2FILE
};

