#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>

#define NEKO_STORE_TYPE_WINDOW (neko_store_window_get_type())
G_DECLARE_FINAL_TYPE (NekoStoreWindow, neko_store_window, NEKO_STORE, WINDOW, GtkApplicationWindow)

NekoStoreWindow *neko_store_window_new (GtkApplication *app);

#endif
