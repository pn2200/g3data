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
        "    <menu action='ViewMenu'>"
        "      <menuitem action='Zoom area'/>"
        "      <menuitem action='Axis settings'/>"
        "      <menuitem action='Output properties'/>"
	"      <separator />"
        "      <menuitem action='FullScreen'/>"
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
