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

/************************************************************************************************/
/* 					ABOUT INFO 						*/
/************************************************************************************************/

#define PROGNAME "g3data"
#define COPYRIGHT "Copyright (C) 2000 Jonas Frantz"
#define COMMENTS "Grab graph data, a program for extracting data from graphs"
#define LICENSE "g3data is free software; you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation; either version 2 of the License, or\n(at your option) any later version.\n\ng3data is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
#define HOMEPAGEURL "http://www.frantz.fi/software/g3data.php"
#define HOMEPAGELABEL "g3data homepage"
#define AUTHORS {"Jonas Frantz", NULL}

/************************************************************************************************/
/* 					COMMANDLINE HELP 					*/
/************************************************************************************************/

gchar HelpText[] =	"\n"
                        "g3data version "VERSION", Copyright (C) 2000 Jonas Frantz\n"
                        " g3data comes with ABSOLUTELY NO WARRANTY; for details\n"
                        " check out the documentation.  This is free software, and\n"
                        " you are welcome to redistribute it under all conditions.\n"
                        "\n"
                        " Usage : g3data [options] [file1.ext [file2.ext [...]]]\n"
                        "\n"
                        "  Options :\n"
                        "      -max <x> <y>                    : If picture is larger x-wise than x or y-wise than y,\n"
                        "                                        the picture is scaled down accordingly otherwise this\n"
                        "                                        parameter has no effect.\n"
                        "      -scale <factor>                 : Scales the picture size according to the factor. This\n"
                        "                                        parameter nullifies effect of the -max parameter.\n"
                        "      -coords <llx> <lly> <rux> <ruy> : Preset the coordinates of the left lower (ll) corner\n"
                        "                                        and the right upper (ru) corner.\n"
                        "      -errors                         : Print out the error of the x value in column 3 and the\n"
                        "                                        error of y value in column 4 of the output.\n"
                        "      -lnx                            : Use logarithmic scale for x coordinates.\n"
                        "      -lny                            : Use logarithmic scale for y coordinates.\n"
                        "      -h or --help                    : Print this help.\n"
                        "\n"
                        "  Supported image formats : PNG, XPM, JPEG, TIFF, PNM, RAS, BMP and GIF\n"
			"\n"
			" Hints and remarks for usage :\n"
			"  - The options given on the commandline only is applied to the images\n"
			"    opened from the commandline.\n"
			"  - To access fullscreen mode use the F11 key.\n"
                        "  - For increased data accuracy use the -scale option with a large parameter\n"
			"    to achieve finer granularity and thus smaller errors.\n"
                        "\n"
                        " Report bugs to jonas.frantz@welho.com\n"
                        "\n";
                                                                                                                                                                    
/************************************************************************************************/
/* 					BUTTON/CHECKBUTTON/LABELS 				*/
/************************************************************************************************/

gchar Dialog_Title[] = "g3data Export data";

gchar Window_Title_NoneOpen[] = "g3data - No open files";

gchar Window_Title[] = "%s - g3data";
                                                                                                                                                                    
gchar setxylabel[4][40] = {"Set point X<sub>1</sub> on X axis (_1)",
                           "Set point X<sub>2</sub> on X axis (_2)",
                           "Set point Y<sub>1</sub> on Y axis (_3)",
                           "Set point Y<sub>2</sub> on Y axis (_4)"};
                                                                                                                                                                    
gchar orderlabel[ORDERBNUM][40] = {"No ordering",
                                   "Based on X value",
                                   "Based on Y value"};

gchar actionlabel[ACTIONBNUM][32] = {"Export data to stdout",
			             "Export data to file"};
                                                                                                                                                                    
gchar loglabel[LOGBNUM][32] = {"_X axis is logarithmic",
                               "_Y axis is logarithmic"};

gchar xy_label_text[4][32] = {"X<sub>1</sub> value : ",
                              "X<sub>2</sub> value : ",
                              "Y<sub>1</sub> value : ",
                              "Y<sub>2</sub> value : "};

gchar PrintBLabel[] = 	"Export point _data";

gchar RemLastBLabel[] = "_Remove last point";

gchar RemAllBLabel[] = 	"Remove _all points";

gchar PrintErrCBLabel[] = "Include _errors";

gchar pm_string[] = 	" ± ";

gchar x_string[] = " X : ";
gchar y_string[] = " Y : ";

gchar filen_string[] = "Filename : ";

gchar nump_string[] = "Number of points : ";

gchar scale_string[] = "Scale image : ";

gchar APheader[] = "<b>Axis points</b>";
gchar PIheader[] = "<b>Processing information</b>";
gchar ZAheader[] = "<b>Zoom area</b>";
gchar Oheader[]  = "<b>Point ordering</b>";
gchar Eheader[]  = "<b>Value errors</b>";
gchar Lheader[]  = "<b>Logarithmic scales</b>";
gchar Aheader[]  = "<b>Action</b>";

/************************************************************************************************/
/* 					TOOLTIPS 						*/
/************************************************************************************************/

gchar setx1tts[] =	"Put out first point on x axis";
gchar setx1tt[] =	"To set out first point on x axis press this button and then press on the "
			"image x-axis (with left mousebutton). This procedure can be replaced by "
			"pressing the middle button on the image.";

gchar setx2tts[] =	"Put out second point on x axis";
gchar setx2tt[] =	"To set out second point on x axis press this button and then press on the "
			"image x-axis (with left mousebutton). This procedure can be replaced by "
			"pressing the middle button on the image.";

gchar sety1tts[] =	"Put out first point on y axis";
gchar sety1tt[] =	"To set out first point on y axis press this button and then press on the "
			"image y-axis (with left mousebutton). This procedure can be replaced by "
			"pressing the right button on the image.";

gchar sety2tts[] =	"Put out second point on y axis";
gchar sety2tt[] =	"To set out second point on y axis press this button and then press on the "
			"image y-axis (with left mousebutton). This procedure can be replaced by "
			"pressing the right button on the image.";

gchar *setxytts[] = 	{&setx1tts[0], &setx2tts[0], &sety1tts[0], &sety2tts[0]};
gchar *setxytt[] = 	{&setx1tt[0], &setx2tt[0], &sety1tt[0], &sety2tt[0]};

gchar entryx1tt[] =	"This entry should contain the value of the first x-axis point";

gchar entryx2tt[] =	"This entry should contain the value of the second x-axis point";

gchar entryy1tt[] =	"This entry should contain the value of the first y-axis point";

gchar entryy2tt[] =	"This entry should contain the value of the second y-axis point";

gchar *entryxytt[] = 	{&entryx1tt[0], &entryx2tt[0], &entryy1tt[0], &entryy2tt[0]};

gchar removeltt[] =	"This button removes the last put out point";

gchar removeatts[] =	"Remove all points";
gchar removeatt[] =	"This button removes all put out points except the axispoints. If all "
			"points are already removed a second click will also remove the axispoints.";

gchar printrestt[] =	"Print data to stdout";

gchar printfilett[] =	"Print data to file %s";

gchar filenamett[] =	"This entry should contain the name of the file to print data to";

gchar logxtt[] =	"If x-axis is logarithmic check this button";

gchar logytt[] =	"If y-axis is logarithmic check this button";

gchar *logxytt[] = 	{&logxtt[0], &logytt[0]};

gchar uetts[] = 	"Show the errors of the x and y values at printout";

gchar uett[] =		"If you want the errors of the x value to be printed in column 3 and the error "
			"of the y value to be printed in column 4 check this button.";
