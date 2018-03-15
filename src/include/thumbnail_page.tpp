#ifndef SPRING_PLAYER_THUMBNAIL_PAGE_H
#include "thumbnail_page.h"
#endif

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

template <typename ContentProvider>
ThumbnailPage<ContentProvider>::ThumbnailPage(std::weak_ptr<MusicLibrary> music_library,
                                              std::weak_ptr<PlaybackList> playback_list) noexcept
  : music_library_(music_library)
  , playback_list_(playback_list)
{
    LOG_INFO("ThumbnailPage({}): Creating...", void_p(this));

    auto builder = gtk_builder_new_from_resource(APPLICATION_PREFIX "/thumbnail_page.ui");
    page_ = gtk_cast<GtkScrolledWindow>(gtk_builder_get_object(builder, "page"));
    content_ = gtk_cast<GtkFlowBox>(gtk_builder_get_object(builder, "content"));
    loading_spinner_ = gtk_cast<GtkSpinner>(gtk_builder_get_object(builder, "loading_spinner"));
    g_object_unref(builder);

    gtk_flow_box_set_filter_func(content_, &filter, static_cast<void *>(this), nullptr);
    connect_g_signal(content_, "child-activated", &on_child_activated, this);
}

template <typename ContentProvider>
template <typename FetchFunction>
void ThumbnailPage<ContentProvider>::activated(FetchFunction &&f) noexcept
{
    LOG_INFO("ThumbnailPage({}): Activated", void_p(this));

    if (children_.empty())
    {
        gtk_spinner_start(loading_spinner_);

        async_queue::push_front_request(new async_queue::Request{
            "load_content", [f, this]() {
                /* TODOD: There is a crash at start-up if we start fetching   */
                /*        album data immediately so we add an artificial delay*/
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                LOG_INFO("ThumbnailPage({}): Loading content...", void_p(this));
                auto r = f();
                async_queue::post_response(new async_queue::Response{
                    "albums_ready", [this, r]() {
                        LOG_INFO("ThumbnailPage({}): Content ready, populating "
                                 "GtkFlowBox",
                                 void_p(this));

                        children_ = std::move(*r);
                        delete r;

                        for (auto &widget : children_)
                        {
                            gtk_flow_box_insert(content_, (*widget)(), -1);
                        }

                        gtk_spinner_stop(loading_spinner_);
                    } });
            } });
    }
}

template <typename ContentProvider>
void ThumbnailPage<ContentProvider>::search_string_changed(std::string &&text) noexcept
{
    LOG_INFO("ThumbnailPage({}): Search string changed, resetting filter", void_p(this));
    gtk_flow_box_invalidate_filter(content_);

    search_string_ = std::move(text);
}

template <typename ContentProvider> GtkWidget *ThumbnailPage<ContentProvider>::operator()() noexcept
{
    return gtk_cast<GtkWidget>(page_);
}

template <typename ContentProvider>
std::int32_t ThumbnailPage<ContentProvider>::filter(GtkFlowBoxChild *child, void *instance) noexcept
{
    auto self = static_cast<ThumbnailPage *>(instance);

    LOG_INFO("ThumbnailPage({}): Applying filter for string {}", void_p(self),
             self->search_string_);

    const auto main_title =
        self->children_.at(static_cast<std::size_t>(gtk_flow_box_child_get_index(child)))
            ->main_title();
    const auto secondary_title =
        self->children_.at(static_cast<std::size_t>(gtk_flow_box_child_get_index(child)))
            ->secondary_title();

    const bool is_near_title_match{ utility::fuzzy_match(self->search_string_, main_title, 2) };
    const bool is_near_artist_match{ utility::fuzzy_match(self->search_string_, secondary_title,
                                                          2) };

    return self->search_string_.empty() || is_near_title_match || is_near_artist_match;
}

template <typename ContentProvider>
void ThumbnailPage<ContentProvider>::on_child_activated(GtkFlowBox *,
                                                        GtkFlowBoxChild *element,
                                                        ThumbnailPage *self) noexcept
{
    auto index = static_cast<std::size_t>(gtk_flow_box_child_get_index(element));

    LOG_INFO("AlbumsPage({}): Activated album at index {}", void_p(self), index);

    self->children_.at(index)->activated();
}
