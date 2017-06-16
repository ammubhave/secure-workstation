#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>

static void constructor(XfcePanelPlugin *plugin);
XFCE_PANEL_PLUGIN_REGISTER(constructor)

static void constructor(XfcePanelPlugin *plugin) {
    GtkWidget *hvbox;
    GtkWidget *label;
    GtkOrientation orientation;

    orientation = xfce_panel_plugin_get_orientation(plugin);
    hvbox = xfce_hvbox_new(orientation, FALSE, 2);
    gtk_widget_show(hvbox);

    gtk_container_add(GTK_CONTAINER(plugin), hvbox);

    label = gtk_label_new("Hello!");
    gtk_container_add(GTK_CONTAINER(hvbox), GTK_WIDGET(label));
    gtk_widget_show(label);
}
