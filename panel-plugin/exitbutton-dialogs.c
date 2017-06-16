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

#include <string.h>
#include <gtk/gtk.h>

#include <exo/exo.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <xfconf/xfconf.h>

#include "exitbutton.h"
#include "exitbutton-dialogs.h"

/* the website url */
#define PLUGIN_WEBSITE "http://goodies.xfce.org/projects/panel-plugins/xfce4-exitbutton-plugin"

static XfcePanelImage *image;

static XfconfChannel*
xfsm_open_config(void)
{
  static XfconfChannel *channel = NULL;

  if (G_UNLIKELY (channel == NULL))
    channel = xfconf_channel_get("xfce4-session");
  return channel;
}

static void
on_check_show_hibernate_clicked(GtkButton *button)
{
  gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button) );
  XfconfChannel *channel = xfsm_open_config();
  xfconf_channel_set_bool(channel, "/shutdown/ShowHibernate", active);
}

static void
on_check_show_suspend_clicked(GtkButton *button)
{
  gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button) );
  XfconfChannel *channel = xfsm_open_config();
  xfconf_channel_set_bool(channel, "/shutdown/ShowSuspend", active);
}

static void
exitbutton_configure_response(GtkWidget *dialog, gint response, ExitbuttonPlugin *exitbutton)
{
  gboolean result;

  if (response == GTK_RESPONSE_HELP)
    {
      /* show help */
      result = g_spawn_command_line_async("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL );

      if (G_UNLIKELY (result == FALSE))
        g_warning(_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  else
    {
      /* remove the dialog data from the plugin */
      g_object_set_data(G_OBJECT(exitbutton->plugin), "dialog", NULL );

      /* unlock the panel menu */
      xfce_panel_plugin_unblock_menu(exitbutton->plugin);

      /* save the plugin */
      exitbutton_save(exitbutton->plugin, exitbutton);

      image = NULL;
      /* destroy the properties dialog */
      gtk_widget_destroy(dialog);
    }
}

static void
applications_menu_plugin_configure_plugin_icon_chooser(GtkWidget *button, ExitbuttonPlugin *exitbutton)
{
  GtkWidget *chooser;
  gchar *icon_name;

  //panel_return_if_fail (XFCE_IS_APPLICATIONS_MENU_PLUGIN (plugin));
  //panel_return_if_fail (XFCE_IS_PANEL_IMAGE (plugin->dialog_icon));

  chooser = exo_icon_chooser_dialog_new(_("Select An Icon"), GTK_WINDOW (gtk_widget_get_toplevel (button)), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL );
  gtk_dialog_set_default_response(GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);
  gtk_dialog_set_alternative_button_order(GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT, GTK_RESPONSE_CANCEL, -1);

  exo_icon_chooser_dialog_set_icon(EXO_ICON_CHOOSER_DIALOG (chooser), exo_str_is_empty (exitbutton->icon_name) ? DEFAULT_ICON_NAME : exitbutton->icon_name);

  if (gtk_dialog_run(GTK_DIALOG (chooser) ) == GTK_RESPONSE_ACCEPT)
    {
      icon_name = exo_icon_chooser_dialog_get_icon(EXO_ICON_CHOOSER_DIALOG (chooser) );
      //g_object_set (G_OBJECT (plugin), "button-icon", icon, NULL);

      if (! exo_str_is_empty(icon_name)) {
        xfce_panel_image_set_from_source (XFCE_PANEL_IMAGE(image), icon_name);
        exitbutton->icon_name = g_strdup(icon_name);
      } else {

      }
      g_free(icon_name);
    }

  gtk_widget_destroy(chooser);
}

void
exitbutton_configure(XfcePanelPlugin *parent, ExitbuttonPlugin *exitbutton)
{
  GtkWidget *dialog;
  XfconfChannel *channel;
  GError *error = NULL;

  if (G_UNLIKELY(!xfconf_init(&error)))
    {
      xfce_dialog_show_error(NULL, error, _("Unable to contact settings server"));
      g_error_free(error);
    }

  /* load xfconf settings */
  channel = xfsm_open_config();

  /* block the plugin menu */
  xfce_panel_plugin_block_menu(parent);

  /* create the dialog */
  dialog = xfce_titled_dialog_new_with_buttons(_("ExitButton Plugin"), GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (parent))), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_HELP, GTK_RESPONSE_HELP, GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL );

  /* center dialog on the screen */
  gtk_window_set_position(GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  /* set dialog icon */
  gtk_window_set_icon_name(GTK_WINDOW (dialog), "xfce4-settings");

  /* link the dialog to the plugin, so we can destroy it when the plugin
   * is closed, but the dialog is still open */
  g_object_set_data(G_OBJECT (parent), "dialog", dialog);

  /* connect the reponse signal to the dialog */
  g_signal_connect(G_OBJECT (dialog), "response", G_CALLBACK(exitbutton_configure_response), exitbutton);

  /* show the entire dialog */
  gtk_widget_show(dialog);

  /* Sets the border width of the window. */
  gtk_container_set_border_width(GTK_CONTAINER (dialog), 10);

  GtkWidget* vbox1 = GTK_DIALOG (dialog) ->vbox;
  //gtk_widget_show (vbox1);
  //gtk_container_add (GTK_CONTAINER (), vbox1);

  gchar* lblapp = g_strdup_printf("<b>%s</b>", _("Appearance"));
  GtkWidget* framelabel1 = gtk_label_new(lblapp);
  gtk_label_set_use_markup(GTK_LABEL (framelabel1), TRUE);
  gtk_widget_show(framelabel1);
  g_free(lblapp);

  GtkWidget* frame1 = gtk_frame_new(NULL );
  gtk_frame_set_label_widget(GTK_FRAME (frame1), framelabel1);
  gtk_widget_show(frame1);
  gtk_container_add(GTK_CONTAINER (vbox1), frame1);

  GtkWidget* alignment1 = gtk_alignment_new(0.50, 0.50, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT (alignment1), 12, 12, 12, 12);
  gtk_widget_show(alignment1);
  gtk_container_add(GTK_CONTAINER (frame1), alignment1);

  GtkWidget* vbox2 = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox2);
  gtk_container_add(GTK_CONTAINER (alignment1), vbox2);

  GtkWidget* hbox1 = gtk_hbox_new(FALSE, 6);
  gtk_widget_show(hbox1);
  gtk_box_pack_start(GTK_BOX (vbox2), hbox1, FALSE, FALSE, 0);

  GtkWidget* iconlabel1 = gtk_label_new(_("Icon:"));
  gtk_label_set_use_markup(GTK_LABEL (iconlabel1), TRUE);
  gtk_widget_show(iconlabel1);
  gtk_box_pack_start(GTK_BOX (hbox1), iconlabel1, FALSE, FALSE, 0);

  GtkWidget* iconbutton1 = gtk_button_new();
  gtk_box_pack_start(GTK_BOX (hbox1), iconbutton1, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT (iconbutton1), "clicked", G_CALLBACK (applications_menu_plugin_configure_plugin_icon_chooser), exitbutton);
  gtk_widget_show(iconbutton1);

  GtkWidget *icon = xfce_panel_image_new_from_source(exo_str_is_empty (exitbutton->icon_name) ? DEFAULT_ICON_NAME : exitbutton->icon_name);
  image = XFCE_PANEL_IMAGE (icon);
  xfce_panel_image_set_size (XFCE_PANEL_IMAGE (icon), 48);
  gtk_widget_show(icon);
  gtk_container_add(GTK_CONTAINER (iconbutton1), icon);

  gchar* lblsess = g_strdup_printf("<b>%s</b>", _("Session"));
  GtkWidget* framelabel2 = gtk_label_new(lblsess);
  gtk_label_set_use_markup(GTK_LABEL (framelabel2), TRUE);
  gtk_widget_show(framelabel2);
  g_free(lblsess);

  GtkWidget* frame2 = gtk_frame_new(NULL );
  gtk_frame_set_label_widget(GTK_FRAME (frame2), framelabel2);
  gtk_widget_show(frame2);
  gtk_container_add(GTK_CONTAINER (vbox1), frame2);

  GtkWidget* alignment2 = gtk_alignment_new(0.50, 0.50, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT (alignment2), 12, 12, 12, 12);
  gtk_widget_show(alignment2);
  gtk_container_add(GTK_CONTAINER (frame2), alignment2);

  GtkWidget* vbox3 = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox3);
  gtk_container_add(GTK_CONTAINER (alignment2), vbox3);

  GtkWidget* notelabel = gtk_label_new(_("NOTE:\nThe following parameters are relative to xfce4-session and not the exitbutton plugin itself.\nSo changing them will have a global effect on the xfce4-session behavior."));
  //gtk_label_set_use_markup ( GTK_LABEL (notelabel), TRUE);
  gtk_widget_show(notelabel);
  gtk_box_pack_start(GTK_BOX (vbox3), notelabel, FALSE, FALSE, 0);

  GtkWidget* check_show_hibernate = gtk_check_button_new_with_label(_("Show Hibernate"));
  gtk_box_pack_start(GTK_BOX (vbox3), check_show_hibernate, FALSE, FALSE, 10);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_show_hibernate), xfconf_channel_get_bool(channel, "/shutdown/ShowHibernate", TRUE));
  g_signal_connect(check_show_hibernate, "clicked", G_CALLBACK(on_check_show_hibernate_clicked), NULL);
  gtk_widget_show(check_show_hibernate);

  GtkWidget* check_show_suspend = gtk_check_button_new_with_label(_("Show Suspend"));
  gtk_box_pack_start(GTK_BOX (vbox3), check_show_suspend, FALSE, FALSE, 10);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_show_suspend), xfconf_channel_get_bool(channel, "/shutdown/ShowSuspend", TRUE));
  g_signal_connect(check_show_suspend, "clicked", G_CALLBACK(on_check_show_suspend_clicked), NULL);
  gtk_widget_show(check_show_suspend);

}

void
exitbutton_about(XfcePanelPlugin *plugin)
{
  GdkPixbuf *icon;
  const gchar *auth[] =
    { "Alessio Piccoli <alepic@geckoblu.net>", NULL };

  icon = xfce_panel_pixbuf_from_source("system-shutdown", NULL, 32);

  gtk_show_about_dialog(NULL, "logo", icon, "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL), "version", PACKAGE_VERSION, "program-name", PACKAGE_NAME, "comments", _("Allows to exit from the system."), "website", "http://goodies.xfce.org/projects/panel-plugins/xfce4-exitbutton-plugin", "copyright",
      _("Copyright (c) 2013\n"), "authors", auth, NULL );
  // TODO: add translators.

  if (icon)
    g_object_unref(G_OBJECT(icon) );
}
