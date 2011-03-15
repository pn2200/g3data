/*
g3data : A program for grabbing data from scanned graphs
Copyright (C) 2011 Paul Novak

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

Authors email : pnovak@alumni.caltech.edu
*/

#include "g3data-about.h"


void g3data_about (GtkWidget *window)
{
    const gchar *authors[] = {"Jonas Frantz", "Paul Novak", NULL};
    const gchar *program_name = "g3data";
    const gchar *version = "1.5.4";
    const gchar *copyright = "Copyright (C) 2000 Jonas Frantz";
    const gchar *comments = "Grab graph data, a program for extracting data from graphs";
    const gchar *license =
    "g3data is free software; you can redistribute it and/or "
    "modify it under the terms of the GNU General Public License as "
    "published by the Free Software Foundation; either version 2 of "
    "the License, or (at your option) any later version.\n"
    "\n"
    "g3data is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
    "General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License "
    "along with this program; if not, write to the Free Software "
    "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA "
    "02111-1307, USA.\n";
    const gchar *website = "http://www.frantz.fi/software/g3data.php";
    const gchar *website_label = "g3data homepage";

    gtk_show_about_dialog (GTK_WINDOW (window), 
                           "authors", authors, 
                           "comments", comments,
                           "copyright", copyright,
                           "license", license,
                           "program-name", program_name,
                           "version", version,
                           "website", website,
                           "website-label", website_label,
                           "wrap-license", TRUE,
                           NULL);
}
