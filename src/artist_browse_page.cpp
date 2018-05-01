#include <gtk/gtk.h>

#include "artist_browse_page.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ArtistBrowsePage::ArtistBrowsePage() noexcept
{
    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/artist_browse_page.ui");
    get_guarded_widget_from_builder(root_container);
    get_widget_from_builder_simple(artist_thumbnail);
    get_widget_from_builder_simple(right_pane_container);
    get_widget_from_builder_simple(artist_name);
    get_widget_from_builder_simple(popular_tracks_listbox);
    get_widget_from_builder_simple(album_list_content);
    get_widget_from_builder_simple(album_list_loading_spinner);
    g_object_unref(builder);
}

ArtistBrowsePage::~ArtistBrowsePage() noexcept
{
}

GtkWidget *ArtistBrowsePage::operator ()() noexcept
{
    return gtk_cast<GtkWidget>(root_container_);
}
