#include "artists_page.h"

#include <libspring_logger.h>

#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

ArtistsPage::ArtistsPage(GtkBuilder *builder,
                         const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    LOG_INFO("ArtistsPage({}): Creating...", void_p(this));

    get_widget_from_builder_simple(artists_page);
    get_widget_from_builder_simple(artists_content);
}

void ArtistsPage::activated() noexcept
{
    LOG_INFO("ArtistsPage({}): Activated", void_p(this));
}

ArtistsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(artists_page_);
}
