/*
 * Copyright (C) 2013 Collabora Ltd.
 *
 * Author: Emilio Pozuelo Monfort <emilio.pozuelo@collabora.co.uk>
 */

#include "config.h"

#include "favorites.h"

#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>

#include "app-icon.h"

enum {
  PROP_0,
};

struct WestonGtkFavoritesPrivate {
  GSettings *settings;
};

G_DEFINE_TYPE(WestonGtkFavorites, weston_gtk_favorites, GTK_TYPE_BOX)

static void
favorite_clicked (GtkButton *button,
    WestonGtkFavorites *self)
{
  GAppInfo *info = g_object_get_data (G_OBJECT(button), "info");
  GError *error = NULL;

  g_app_info_launch (info, NULL, NULL, &error);
  if (error)
    {
      g_warning ("Could not launch app %s: %s",
          g_app_info_get_name (info),
          error->message);
      g_clear_error (&error);
    }
}

static void
add_favorite (WestonGtkFavorites *self,
    const gchar *favorite)
{
  GDesktopAppInfo *info;
  GtkWidget *button, *image;
  GIcon *icon;

  info = g_desktop_app_info_new (favorite);

  if (!info)
    return;

  icon = g_app_info_get_icon (G_APP_INFO (info));

  button = weston_gtk_app_icon_new_from_gicon (icon);

  g_object_set_data_full (G_OBJECT (button), "info", info, g_object_unref);

  g_signal_connect (button, "clicked", G_CALLBACK (favorite_clicked), self);

  gtk_box_pack_end (GTK_BOX (self), button, FALSE, FALSE, 0);
}

static void
remove_favorite (GtkWidget *favorite,
    gpointer user_data)
{
  gtk_widget_destroy (favorite);
}

static void
favorites_changed (GSettings *settings,
    const gchar *key,
    WestonGtkFavorites *self)
{
  gchar **favorites = g_settings_get_strv (settings, key);
  gint i;

  /* Remove all favorites first */
  gtk_container_foreach (GTK_CONTAINER (self), remove_favorite, NULL);

  for (i = 0; i < g_strv_length (favorites); i++)
    {
      gchar *fav = favorites[i];

      add_favorite (self, fav);
    }

  g_strfreev (favorites);
}

static void
weston_gtk_favorites_dispose (GObject *object)
{
  WestonGtkFavorites *self = WESTON_GTK_FAVORITES (object);

  g_clear_object (&self->priv->settings);

  G_OBJECT_CLASS (weston_gtk_favorites_parent_class)->dispose (object);
}

static void
weston_gtk_favorites_init (WestonGtkFavorites *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            WESTON_GTK_TYPE_FAVORITES,
                                            WestonGtkFavoritesPrivate);

  self->priv->settings = g_settings_new ("org.raspberrypi.maynard");
  g_signal_connect (self->priv->settings, "changed::favorites",
                    G_CALLBACK (favorites_changed), self);
  favorites_changed (self->priv->settings, "favorites", self);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
}

static void
weston_gtk_favorites_class_init (WestonGtkFavoritesClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->dispose = weston_gtk_favorites_dispose;

  g_type_class_add_private (object_class, sizeof (WestonGtkFavoritesPrivate));
}

GtkWidget *
weston_gtk_favorites_new (void)
{
  return g_object_new (WESTON_GTK_TYPE_FAVORITES,
      "orientation", GTK_ORIENTATION_VERTICAL,
      NULL);
}
