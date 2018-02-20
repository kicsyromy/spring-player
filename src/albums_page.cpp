#include "albums_page.h"

#include "async_queue.h"
#include "common.h"

using namespace spring;
using namespace spring::player;

AlbumsPage::AlbumsPage(GtkBuilder *builder,
                       const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    get_widget_from_builder_simple(albums_page);
    get_widget_from_builder_simple(albums_content);
    get_widget_from_builder_simple(albums_loading_spinner);
    get_widget_from_builder_simple(search_entry);

    gtk_flow_box_set_filter_func(albums_content_, &AlbumsPage::filter,
                                 reinterpret_cast<void *>(this),
                                 &AlbumsPage::destroy_function);

    g_signal_connect(albums_content_, "child-activated",
                     G_CALLBACK(&on_child_activated), this);

    g_signal_connect(search_entry_, "search-changed",
                     G_CALLBACK(&on_search_changed), this);
}

void AlbumsPage::activated() noexcept
{
    if (albums_.empty())
    {
        gtk_spinner_start(albums_loading_spinner_);

        async_queue::post_request(new async_queue::Request{
            "load_albums", [this]() {
                auto album_widgets =
                    new std::vector<std::unique_ptr<AlbumWidget>>{};
                auto albums = music_library_.albums();

                for (auto &album : albums)
                {
                    album_widgets->push_back(
                        std::make_unique<AlbumWidget>(std::move(album)));
                }

                async_queue::post_response(new async_queue::Response{
                    "albums_ready", [this, album_widgets]() {
                        albums_ = std::move(*album_widgets);
                        delete album_widgets;

                        for (auto &album : albums_)
                        {
                            gtk_flow_box_insert(albums_content_, *album, -1);
                        }

                        gtk_spinner_stop(albums_loading_spinner_);
                    } });
            } });
    }
}

gboolean AlbumsPage::filter(GtkFlowBoxChild *element, void *instance) noexcept
{
    auto self = static_cast<AlbumsPage *>(instance);
    auto searched_text = std::string{ gtk_entry_get_text(
        gtk_cast<GtkEntry>(self->search_entry_)) };
    return searched_text.empty() ||
           self->albums_
                   .at(static_cast<std::size_t>(
                       gtk_flow_box_child_get_index(element)))
                   ->title() == searched_text;
}

void AlbumsPage::destroy_function(void *) noexcept
{
}

void AlbumsPage::on_search_changed(GtkSearchEntry *entry,
                                   AlbumsPage *self) noexcept
{
    if (gtk_widget_get_visible(gtk_cast<GtkWidget>(self->albums_page_)))
    {
        gtk_flow_box_invalidate_filter(self->albums_content_);
    }
}

void AlbumsPage::on_child_activated(GtkFlowBox *,
                                    GtkFlowBoxChild *element,
                                    AlbumsPage *self) noexcept
{
    self->albums_
        .at(static_cast<std::size_t>(gtk_flow_box_child_get_index(element)))
        ->activated();
}

AlbumsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(albums_page_);
}
