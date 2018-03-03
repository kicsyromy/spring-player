#include "genres_page.h"

#include <libspring_logger.h>

#include "utility.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::utility;

GenresPage::GenresPage(GtkBuilder *builder, const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    LOG_INFO("GenresPage({}): Creating...", void_p(this));

    get_widget_from_builder_simple(genres_page);
    get_widget_from_builder_simple(genres_content);
}

void GenresPage::activated() noexcept
{
    LOG_INFO("GenresPage({}): Activated", void_p(this));
}

GenresPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(genres_page_);
}
