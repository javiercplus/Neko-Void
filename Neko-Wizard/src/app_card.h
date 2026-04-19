#ifndef APP_CARD_H
#define APP_CARD_H

#include <gtk/gtk.h>
#include "apps_manager.h"

G_BEGIN_DECLS

#define NEKO_TYPE_APP_CARD (neko_app_card_get_type())
G_DECLARE_FINAL_TYPE (NekoAppCard, neko_app_card, NEKO, APP_CARD, GtkWidget)

GtkWidget *neko_app_card_new(AppInfo *info);
void neko_app_card_set_selected(NekoAppCard *card, gboolean selected);
gboolean neko_app_card_get_selected(NekoAppCard *card);

G_END_DECLS

#endif
