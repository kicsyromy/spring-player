#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "thumbnail_page.h"
#include "playback_list.h"

#include "utility/fuzzy_search.h"
#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ThumbnailPage::ThumbnailPage(std::weak_ptr<MusicLibrary> music_library, std::weak_ptr<PlaybackList> playback_list) noexcept
  : music_library_(music_library)
  , playback_list_(playback_list)
{
    LOG_INFO("ThumbnailPage({}): Creating...", void_p(this));

    gtk_flow_box_set_filter_func(content_, &filter, static_cast<void *>(this), [](void *) {});
    connect_g_signal(content_, "child-activated", &on_child_activated, this);
}

GtkWidget *ThumbnailPage::operator()() noexcept
{
    return gtk_cast<GtkWidget>(page_);
}

std::int32_t ThumbnailPage::filter(GtkFlowBoxChild */*child*/, void *instance) noexcept
{
    auto self = static_cast<ThumbnailPage *>(instance);

    const auto searched_text = std::string{};
//std::string{ gtk_entry_get_text(gtk_cast<GtkEntry>(self->search_entry_)) };

    LOG_INFO("AlbumsPage({}): Applying filter for string {}", void_p(self), searched_text);

//const auto &album_title =
//    self->albums_.at(static_cast<std::size_t>(gtk_flow_box_child_get_index(child)))->title();
//const auto &artist_name =
//    self->albums_.at(static_cast<std::size_t>(gtk_flow_box_child_get_index(child)))->artist();

//const bool is_near_title_match{ utility::fuzzy_match(searched_text, album_title, 2) };
//const bool is_near_artist_match{ utility::fuzzy_match(searched_text, artist_name, 2) };

//return searched_text.empty() || is_near_title_match || is_near_artist_match;
    return true;
}

void ThumbnailPage::on_search_changed(GtkSearchEntry *element, ThumbnailPage *self) noexcept
{
    LOG_INFO("AlbumsPage({}): Search string changed, resetting filter", void_p(self));

//    if (gtk_widget_get_visible(gtk_cast<GtkWidget>(self->albums_page_)))
//    {
//        gtk_flow_box_invalidate_filter(self->albums_content_);
//    }
}

void ThumbnailPage::on_child_activated(GtkFlowBox *, GtkFlowBoxChild *element, ThumbnailPage *self) noexcept
{
}
