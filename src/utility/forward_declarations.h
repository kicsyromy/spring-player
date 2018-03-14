#ifndef SPRING_PLAYER_UTILITY_FORWARD_DECLARATIONS_H
#define SPRING_PLAYER_UTILITY_FORWARD_DECLARATIONS_H

#include <cstdint>

#ifndef __GDK_H__
using GdkInterpType = std::int32_t;
#endif

#ifndef __GLIB_GOBJECT_H__
using GConnectFlags = std::int32_t;
#endif

#define declare_g_type(name) using name = struct _##name;

declare_g_type(GObject);

declare_g_type(GError);

declare_g_type(GtkWidget);
declare_g_type(GtkBox);
declare_g_type(GtkListBox);
declare_g_type(GtkListBoxRow);
declare_g_type(GtkPopover);
declare_g_type(GtkImage);
declare_g_type(GtkLabel);
declare_g_type(GtkSpinner);
declare_g_type(GtkScrolledWindow);
declare_g_type(GtkFlowBox);
declare_g_type(GtkFlowBoxChild);
declare_g_type(GtkSpinner);
declare_g_type(GtkSearchEntry);
declare_g_type(GtkButtonBox);
declare_g_type(GtkButton);
declare_g_type(GtkToggleButton);
declare_g_type(GtkScale);
declare_g_type(GtkAdjustment);
declare_g_type(GtkHeaderBar);
declare_g_type(GtkPaned);
declare_g_type(GtkRevealer);
declare_g_type(GtkSearchEntry);
declare_g_type(GtkStack);
declare_g_type(GtkContainer);

declare_g_type(GtkBuilder);

declare_g_type(GtkCssProvider);

declare_g_type(GdkPixbuf);
declare_g_type(GdkPixbufLoader);

using cairo_t = struct _cairo;

extern "C" void *g_object_ref_sink(void *);
extern "C" void *g_object_ref(void *);
extern "C" void g_object_unref(void *);

extern "C" std::uint32_t g_idle_add(int (*)(void *), void *);
extern "C" std::uint64_t g_signal_connect_data(void *,
                                               const char *,
                                               void (*)(),
                                               void *data,
                                               void (*)(void *, struct _GClosure *),
                                               GConnectFlags);

extern "C" GdkPixbufLoader *gdk_pixbuf_loader_new();
extern "C" std::int32_t gdk_pixbuf_loader_write(GdkPixbufLoader *loader,
                                                const std::uint8_t *buf,
                                                std::uint64_t count,
                                                GError **error);
extern "C" std::int32_t gdk_pixbuf_loader_close(GdkPixbufLoader *loader, GError **error);
extern "C" GdkPixbuf *gdk_pixbuf_scale_simple(const GdkPixbuf *src,
                                              std::int32_t dest_width,
                                              std::int32_t dest_height,
                                              GdkInterpType interp_type);
extern "C" GdkPixbuf *gdk_pixbuf_loader_get_pixbuf(GdkPixbufLoader *loader);

#endif // !SPRING_PLAYER_UTILITY_FORWARD_DECLARATIONS_H
