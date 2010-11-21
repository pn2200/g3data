/************************************************************************************************/
/* 					COMMANDLINE HELP 					*/
/************************************************************************************************/

gchar HelpText[] =	"\n"
                        "g3data version "VERSION", Copyright (C) 2000 Jonas Frantz\n"
                        " g3data comes with ABSOLUTELY NO WARRANTY; for details\n"
                        " check out the documentation.  This is free software, and\n"
                        " you are welcome to redistribute it under all conditions.\n"
                        "\n"
                        " Usage : g3data [options] file.ext\n"
                        "\n"
                        "  Options :\n"
                        "      -max <x> <y>                    : If picture is larger x-wise than x or y-wise than y,\n"
                        "                                        the picture is scaled down accordingly otherwise this\n"
                        "                                        parameter has no effect.\n"
                        "      -scale <factor>                 : Scales the picture size according to the factor. This\n"
                        "                                        parameter nullifies effect of the -max parameter.\n"
                        "      -scroll                         : Picture is viewed through a viewport with scrollbars,\n"
                        "                                        useful for very large pictures.\n"
                        "      -coords <llx> <lly> <rux> <ruy> : Preset the coordinates of the left lower (ll) corner\n"
                        "                                        and the right upper (ru) corner.\n"
                        "      -errors                         : Print out the error of the x value in column 3 and the\n"
                        "                                        error of y value in column 4 of the output.\n"
                        "      -lnx                            : Use logarithmic scale for x coordinates.\n"
                        "      -lny                            : Use logarithmic scale for y coordinates.\n"
                        "      -h or --help                    : Print this help.\n"
                        "\n"
                        "  Supported image formats : JPEG, GIF, PPM, PGM, XPM, PNG, TIFF, EIM\n"
                        "\n"
			"  To access fullscreen mode use the F10 key.\n"
			"\n"
                        "  Hint for increased accuracy : Use -scale with a large parameter to achieve\n"
                        "                                finer granularity and thus smaller errors.\n"
                        "\n"
                        " Report bugs to jonas.frantz@helsinki.fi\n"
                        "\n";
                                                                                                                                                                    
/************************************************************************************************/
/* 					BUTTON/CHECKBUTTON/LABELS 				*/
/************************************************************************************************/

gchar Dialog_Title[] = "g3data Export data";

gchar Window_Title[] = "%s - g3data";
                                                                                                                                                                    
gchar setxylabel[4][40] = {"Set point X<sub>1</sub> on X axis (_1)",
                           "Set point X<sub>2</sub> on X axis (_2)",
                           "Set point Y<sub>1</sub> on Y axis (_3)",
                           "Set point Y<sub>2</sub> on Y axis (_4)"};
                                                                                                                                                                    
gchar orderlabel[ORDERBNUM][40] = {"Do not order output data",
                                   "Order output data according to X value",
                                   "Order output data according to Y value"};

gchar actionlabel[ACTIONBNUM][32] = {"Export data to stdout",
			             "Export data to file"};
                                                                                                                                                                    
gchar loglabel[LOGBNUM][32] = {"_X axis is logarithmic",
                               "_Y axis is logarithmic"};

gchar xy_label_text[4][32] = {"X<sub>1</sub> value : ",
                              "X<sub>2</sub> value : ",
                              "Y<sub>1</sub> value : ",
                              "Y<sub>2</sub> value : "};

gchar PrintBLabel[] = 	"Export point _data";

gchar PrintToStdoutLabel[] = "Print data to _stdout";

gchar RemLastBLabel[] = "_Remove last point";

gchar RemAllBLabel[] = 	"Remove _all points";

gchar PrintToFileBLabel[] = "Print data to _file";

gchar PrintErrCBLabel[] = "Print _errors with data printout";

gchar pm_string[] = 	" ± ";

gchar x_string[] = " X : ";
gchar y_string[] = " Y : ";

gchar filen_string[] = "Filename : ";

gchar nump_string[] = "Number of points : ";

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

gchar entryx1tt[] =	"This entry should contain the value of the first x-axis point.";

gchar entryx2tt[] =	"This entry should contain the value of the second x-axis point.";

gchar entryy1tt[] =	"This entry should contain the value of the first y-axis point.";

gchar entryy2tt[] =	"This entry should contain the value of the second y-axis point.";

gchar *entryxytt[] = 	{&entryx1tt[0], &entryx2tt[0], &entryy1tt[0], &entryy2tt[0]};

gchar removeltt[] =	"This button removes the last put out point.";

gchar removeatts[] =	"Remove all points";
gchar removeatt[] =	"This button removes all put out points except the axispoints. If all "
			"points are already removed a second click will also remove the axispoints.";

gchar printrestt[] =	"Print data to stdout.";

gchar printfilett[] =	"Print data to file with name defined in entrybox.";

gchar filenamett[] =	"This entry should contain the name of the file to print data to.";

gchar logxtt[] =	"If x-axis is logarithmic check this button.";

gchar logytt[] =	"If y-axis is logarithmic check this button.";

gchar *logxytt[] = 	{&logxtt[0], &logytt[0]};

gchar uetts[] = 	"Show the errors of the x and y values at printout.";

gchar uett[] =		"If you want the errors of the x value to be printed in column 3 and the error "
			"of the y value to be printed in column 4 check this button.";
