#include "songs_page.h"

#include "utility.h"

using namespace spring;
using namespace spring::player;

SongsPage::SongsPage(GtkBuilder *builder,
                     const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    get_widget_from_builder_simple(songs_page);
    get_widget_from_builder_simple(songs_content);
}

void SongsPage::activated() noexcept
{
    g_warning("songs activated");
}

SongsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(songs_page_);
}
