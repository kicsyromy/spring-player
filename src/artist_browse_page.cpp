#include <gtk/gtk.h>

#include <fmt/format.h>

#include "artist_browse_page.h"

#include "utility/pixbuf_loader.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ArtistBrowsePage::TrackListEntry::TrackListEntry(std::size_t index,
                                                 const string_view &title,
                                                 music::Track::Milliseconds duration) noexcept
{
    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/artist_browse_page.ui");
    get_guarded_widget_from_builder(track_list_entry);
    get_widget_from_builder_simple(track_number);
    get_widget_from_builder_simple(track_title);
    get_widget_from_builder_simple(track_duration);
    g_object_unref(builder);

    gtk_label_set_text(track_number_, fmt::format("{}", index).c_str());
    gtk_label_set_text(track_title_, title.data());
    gtk_label_set_text(track_duration_, fmt::format("{}", duration.count()).c_str());
}

ArtistBrowsePage::TrackListEntry::~TrackListEntry() noexcept
{
}

GtkWidget *ArtistBrowsePage::TrackListEntry::operator()() noexcept
{
    return gtk_cast<GtkWidget>(track_list_entry_);
}

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

void ArtistBrowsePage::set_artist(const music::Artist &artist) noexcept
{
    auto artwork = load_pixbuf_from_data_scaled<200, 200>(artist.artwork());
    gtk_image_set_from_pixbuf(artist_thumbnail_, artwork);
    g_object_unref(artwork);

    gtk_label_set_text(artist_name_, artist.name().c_str());

    auto tracks = artist.popularTracks(5);
    std::size_t index = 0;
    for (const auto &track : tracks)
    {
        TrackListEntry e{ index++, track.title(), track.duration() };
        gtk_container_add(gtk_cast<GtkContainer>(popular_tracks_listbox_), e());
    }
}

GtkWidget *ArtistBrowsePage::operator()() noexcept
{
    return gtk_cast<GtkWidget>(root_container_);
}
