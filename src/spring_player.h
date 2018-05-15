#ifndef SPRING_PLAYER_H
#define SPRING_PLAYER_H

#include <gtk/gtk.h>

#include <libspring_plex_media_server.h>

#define SPRING_PLAYER_TYPE (spring_player_get_type())
G_DECLARE_FINAL_TYPE(SpringPlayer, spring_player, SPRING_PLAYER, APPLICATION, GtkApplication)

SpringPlayer *spring_player_new();

#endif // !SPRING_PLAYER_H
