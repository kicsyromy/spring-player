#include <gtk/gtk.h>

#include <libspring_logger.h>

#include "ui/songs_page.h"

#include "utility/global.h"
#include "utility/gtk_helpers.h"

using namespace spring;
using namespace spring::player;
using namespace spring::player::ui;
using namespace spring::player::utility;

SongsPage::SongsPage(GtkBuilder *builder, const MusicLibrary &music_library) noexcept
  : music_library_(music_library)
{
    LOG_INFO("SongsPage({}): Creating...", void_p(this));

    get_widget_from_builder_simple(songs_page);
    get_widget_from_builder_simple(songs_content);
}

SongsPage::~SongsPage() noexcept
{
    LOG_INFO("SongsPage({}): Destroying...", void_p(this));
}

void SongsPage::activated() noexcept
{
    LOG_INFO("SongsPage({}): Activated", void_p(this));
}

SongsPage::operator GtkWidget *() noexcept
{
    return gtk_cast<GtkWidget>(songs_page_);
}
