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

/* Actions definitions */
  GtkActionEntry entries[] = {
        { "FileMenu", NULL, "_File" },
        { "ViewMenu", NULL, "_View" },
        { "HelpMenu", NULL, "_Help" },
        { "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open an image in a new tab", (GCallback) menu_file_open },
        { "Quit", GTK_STOCK_QUIT, "_Quit", "<control>Q", "Quit program", (GCallback) menu_file_exit },
        { "About", GTK_STOCK_HELP, "_About", "<control>H", "About g3data", (GCallback) menu_help_about }
  };

  GtkActionEntry closeaction[] = {
        { "Close", GTK_STOCK_CLOSE, "_Close", "<control>C", "Close current tab", (GCallback) menu_tab_close }
  };

  GtkToggleActionEntry toggle_entries[] = {
	{ "Zoom area", NULL, "Zoom area", "F5", "Hide zoom area", (GCallback) hide_zoom_area_callback, FALSE },
	{ "Axis settings", NULL, "Axis settings", "F6", "Hide axis settings", (GCallback) hide_axis_settings_callback, FALSE },
	{ "Output properties", NULL, "Output properties", "F7", "Hide output properties", (GCallback) hide_output_prop_callback, FALSE },
	{ "FullScreen", NULL, "_Full Screen", "F11", "Switch between full screen and windowed mode", (GCallback) full_screen_action_callback, FALSE }
  };

/* Menu definitions */
  char *ui_description =
        "<ui>"
        "  <menubar name='MainMenu'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='Open'/>"
        "      <menuitem action='Close'/>"
	"      <separator />"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu action='ViewMenu'>"
        "      <menuitem action='Zoom area'/>"
        "      <menuitem action='Axis settings'/>"
        "      <menuitem action='Output properties'/>"
	"      <separator />"
        "      <menuitem action='FullScreen'/>"
        "    </menu>"
        "    <menu action='HelpMenu'>"
        "      <menuitem action='About'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";


/* Drag and drop definitions */

static GtkTargetEntry ui_drop_target_entries [NUM_IMAGE_DATA] = {
  {"text/uri-list", 0, URI_LIST},
  {"image/png",     0, PNG_DATA},
  {"image/jpeg",    0, JPEG_DATA},
  {"application/x-color",    0, APP_X_COLOR}
};
