#include "page_stack.h"

#include <libspring_logger.h>

#include "utility.h"

using namespace spring;
using namespace spring::player;

PageStack::PageStack(GtkBuilder *builder, MusicLibrary &&music_library) noexcept
{
    LOG_INFO("PageStack({}): Creating...", void_p(this));

    *music_library_ = std::move(music_library);

    get_widget_from_builder_simple(page_stack);

    auto switch_page = [this](Page page) {
        switch (page)
        {
            case Page::Albums:
                gtk_stack_set_visible_child(page_stack_, *albums_page_.get());
                albums_page_->activated();
                break;
            case Page::Artists:
                gtk_stack_set_visible_child(page_stack_, *artists_page_.get());
                artists_page_->activated();
                break;
            case Page::Genres:
                gtk_stack_set_visible_child(page_stack_, *genres_page_.get());
                genres_page_->activated();
                break;
            case Page::Songs:
                gtk_stack_set_visible_child(page_stack_, *songs_page_.get());
                songs_page_->activated();
                break;
            default:
                break;
        }
    };

    albums_page_ = std::make_unique<AlbumsPage>(builder, music_library_);
    /* weak_ptr instead of ref */
    artists_page_ = std::make_unique<ArtistsPage>(builder, *music_library_);
    genres_page_ = std::make_unique<GenresPage>(builder, *music_library_);
    songs_page_ = std::make_unique<SongsPage>(builder, *music_library_);

    switch_page(settings::get_current_page());

    page_stack_switcher_ =
        std::make_unique<PageStackSwitcher>(builder, switch_page);
}

PageStack::~PageStack() noexcept
{
    LOG_INFO("PageStack({}): Destroying...", void_p(this));
}
