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
        { "FileMenu", NULL, "_File", NULL, NULL, NULL },
        { "ViewMenu", NULL, "_View", NULL, NULL, NULL },
        { "HelpMenu", NULL, "_Help", NULL, NULL, NULL },
        { "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open an image in a new tab", G_CALLBACK( menu_file_open ) },
        { "Save As", GTK_STOCK_SAVE_AS, "Save _As...", "<control><shift>S", "Save data", G_CALLBACK( file_save_as_dialog ) },
        { "Close", GTK_STOCK_CLOSE, "_Close", "<control>W", "Close window", G_CALLBACK( close_application ) },
        { "About", GTK_STOCK_HELP, "_About", "", "About g3data", G_CALLBACK( menu_help_about ) }
  };

  GtkToggleActionEntry toggle_entries[] = {
	{ "Zoom area", NULL, "Zoom area", "F5", "Zoom area", G_CALLBACK( hide_zoom_area_callback ), TRUE },
	{ "Axis settings", NULL, "Axis settings", "F6", "Axis settings", G_CALLBACK( hide_axis_settings_callback ), TRUE },
	{ "Output properties", NULL, "Output properties", "F7", "Output properties", G_CALLBACK( hide_output_prop_callback ), TRUE }
  };

  GtkToggleActionEntry full_screen[] = {
	{ "FullScreen", NULL, "_Full Screen", "F11", "Switch between full screen and windowed mode", G_CALLBACK( full_screen_action_callback ), FALSE }
  };
/* Menu definitions */
const gchar *ui_description =
        "<ui>"
        "  <menubar name='MainMenu'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='Open'/>"
        "      <menuitem action='Save As'/>"
	"      <separator />"
        "      <menuitem action='Close'/>"
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

const GtkTargetEntry ui_drop_target_entries[] = {
  {"text/uri-list", 0, URI_LIST},
  {"image/png",     0, PNG_DATA},
  {"image/jpeg",    0, JPEG_DATA},
  {"application/x-color",    0, APP_X_COLOR}
};
