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
#define MAXPOINTS 256					/* Maximum number of points that can be put out */
#define VERSION "1.4.0.1"				/* Version number */

#if GTK_MAJOR_VERSION == 2
  #if GTK_MINOR_VERSION >= 2
    #define GTK22X
  #else
    #define GTK20X
  #endif
#else
  #error "You need to have gtk+ version 2.0.0 or higher installed to compile and run g3data "VERSION"."
#endif

struct PointValue {
  gdouble 	Xv,Yv,Xerr,Yerr;
};

#define ACTIONBNUM 2
#define ORDERBNUM 3
#define LOGBNUM 2

enum ACTION {
  PRINT2STDOUT = 0,
  PRINT2FILE
};

