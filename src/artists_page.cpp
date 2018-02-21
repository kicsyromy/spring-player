#include "artists_page.h"

#include "utility.h"

using namespace spring;
using namespace spring::player;

ArtistsPage::ArtistsPage(GtkBuilder *builder,
                         const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    get_widget_from_builder_simple(artists_page);
    get_widget_from_builder_simple(artists_content);
}

void ArtistsPage::activated() noexcept
{
    g_warning("artists activated");
}

ArtistsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(artists_page_);
}
