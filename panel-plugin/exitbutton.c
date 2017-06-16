/*  
 *  Copyright (C) 2013 Alessio Piccoli <alepic@geckoblu.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <exo/exo.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "exitbutton.h"
#include "exitbutton-dialogs.h"

#ifdef LIBXFCE4PANEL_CHECK_VERSION
  #if LIBXFCE4PANEL_CHECK_VERSION(4,9,0)
    #define HAS_PANEL_49
  #endif
#endif

static void exitbutton_plugin_button_spawn_command(const gchar *command)
{
  GError *error = NULL;

  if (g_getenv ("SESSION_MANAGER") == NULL)
    {
      /* TRANSLATORS: no session manager is launched, so avoid any
       * problems and ask the user to quit the panel so users without
       * xfce4-session can still close the xserver */
      if (xfce_dialog_confirm (NULL, GTK_STOCK_QUIT, NULL,
          _("You have started X without session manager. Clicking Quit will close the X server."),
          _("Are you sure you want to quit the panel?")))
         command = "xfce4-panel --quit";
       else
         return;
    }

  if (!g_spawn_command_line_async (command, &error))
    {
      xfce_dialog_show_error (NULL, error, _("Failed to execute command \"%s\""), command);
      g_error_free (error);
    }
}



static void button_clicked(GtkWidget *button, ExitbuttonPlugin *exitbutton)
{
    exitbutton_plugin_button_spawn_command("xfce4-session-logout");
}



void
exitbutton_save(XfcePanelPlugin *plugin, ExitbuttonPlugin *exitbutton) {

  if (exo_str_is_empty(exitbutton->icon_name))
    {
      return;
    }

  // Update the icon
  xfce_panel_image_set_from_source(XFCE_PANEL_IMAGE(exitbutton->icon), exitbutton->icon_name);

  /********************************************************
   * Save the properties
   ********************************************************/
  XfceRc *rc;
  gchar *file;

  /* get the config file location */
  file = xfce_panel_plugin_save_location(plugin, TRUE);

  if (G_UNLIKELY (file == NULL))
    {
      DBG("Failed to open config file");
      return;
    }

  /* open the config file, read/write */
  rc = xfce_rc_simple_open(file, FALSE);
  g_free(file);

  if (G_LIKELY (rc != NULL))
    {
      /* save the settings */
      DBG(".");
      if (exitbutton->icon_name)
        xfce_rc_write_entry(rc, "icon_name", exitbutton->icon_name);

      /* close the rc file */
      xfce_rc_close(rc);
    }
}



static void
exitbutton_read(ExitbuttonPlugin *exitbutton)
{
  XfceRc *rc;
  gchar *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location(exitbutton->plugin, TRUE);

  if (G_LIKELY (file != NULL))
    {
      /* open the config file, readonly */
      rc = xfce_rc_simple_open(file, TRUE);

      /* cleanup */
      g_free(file);

      if (G_LIKELY (rc != NULL))
        {
          /* read the settings */
          value = xfce_rc_read_entry(rc, "icon_name", DEFAULT_ICON_NAME);
          exitbutton->icon_name = g_strdup(value);

          /* cleanup */
          xfce_rc_close(rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG("Applying default settings");

  exitbutton->icon_name = g_strdup(DEFAULT_ICON_NAME);
}



static ExitbuttonPlugin *
exitbutton_new(XfcePanelPlugin *plugin)
{
  /* allocate memory for the plugin structure */
  ExitbuttonPlugin *exitbutton = g_slice_new0(ExitbuttonPlugin);

  /* pointer to plugin */
  exitbutton->plugin = plugin;

  /* read the user settings */
  exitbutton_read(exitbutton);

  /* some panel widgets */
  exitbutton->button = xfce_panel_create_button();
  gtk_widget_show(exitbutton->button);
  g_signal_connect(G_OBJECT(exitbutton->button), "clicked", G_CALLBACK(button_clicked), plugin);

  exitbutton->icon = xfce_panel_image_new_from_source(exo_str_is_empty (exitbutton->icon_name) ? DEFAULT_ICON_NAME : exitbutton->icon_name);
  gtk_widget_show(exitbutton->icon);
  gtk_container_add(GTK_CONTAINER(exitbutton->button), exitbutton->icon);

  return exitbutton;
}



static void exitbutton_free(XfcePanelPlugin *plugin, ExitbuttonPlugin *exitbutton)
{
  /* check if the dialog is still open. if so, destroy it */
  GtkWidget *dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY(dialog != NULL))
    gtk_widget_destroy(dialog);

  /* destroy the panel widgets */
  gtk_widget_destroy(exitbutton->button);
  gtk_widget_destroy(exitbutton->icon);

  /* free the plugin structure */
  g_slice_free(ExitbuttonPlugin, exitbutton);
}



static void exitbutton_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation   orientation, ExitbuttonPlugin *exitbutton)
{
  /* change the orientation of the box */
  //xfce_hvbox_set_orientation (XFCE_HVBOX (exitbutton->hvbox), orientation);
}



static gboolean exitbutton_size_changed(XfcePanelPlugin *plugin, gint size, ExitbuttonPlugin *exitbutton)
{
   /* shrink the gtk button's image to new size -*/
#ifdef HAS_PANEL_49
   size /= xfce_panel_plugin_get_nrows (plugin);
#endif
   gtk_widget_set_size_request(GTK_WIDGET(exitbutton->button), size, size);

  /* we handled the change */
  return TRUE;
}



static void exitbutton_construct(XfcePanelPlugin *plugin)
{
  ExitbuttonPlugin *exitbutton;

  /* setup translation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  exitbutton = exitbutton_new(plugin);

  /* add the button to the panel */
  gtk_container_add(GTK_CONTAINER (plugin), exitbutton->button);

  /* show the panel's right-click menu on this bitton */
  xfce_panel_plugin_add_action_widget(plugin, exitbutton->button);

  /* connect plugin signals */
  g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(exitbutton_free), exitbutton);

  g_signal_connect(G_OBJECT(plugin), "save", G_CALLBACK(exitbutton_save), exitbutton);

  g_signal_connect(G_OBJECT(plugin), "size-changed", G_CALLBACK(exitbutton_size_changed), exitbutton);

  g_signal_connect(G_OBJECT(plugin), "orientation-changed", G_CALLBACK(exitbutton_orientation_changed), exitbutton);

  /* show the configure menu item and connect signal */
  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT(plugin), "configure-plugin", G_CALLBACK(exitbutton_configure), exitbutton);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT(plugin), "about", G_CALLBACK(exitbutton_about), NULL);
}



/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER(exitbutton_construct);

