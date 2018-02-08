#include "genres_page.h"

#include "common.h"

using namespace spring;
using namespace spring::player;

GenresPage::GenresPage(GtkBuilder *builder,
                       const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    get_widget_from_builder_simple(genres_page);
    get_widget_from_builder_simple(genres_content);
}

void GenresPage::activated() noexcept
{
    g_warning("genres activated");
}

GenresPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(genres_page_);
}
