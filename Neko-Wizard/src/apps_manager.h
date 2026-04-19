#ifndef APPS_MANAGER_H
#define APPS_MANAGER_H

#include <glib.h>

typedef enum {
    GROUP_GAMING,
    GROUP_AUDIO_VIDEO,
    GROUP_DRAWING_IMAGE,
    GROUP_SOCIAL,
    GROUP_TEXT_DOCUMENTS,
    GROUP_DRIVERS
} AppGroup;

typedef struct {
    const char *name;
    const char *icon_path;
    const char *install_command;
    AppGroup group;
    gboolean selected;
    gboolean install_success;
} AppInfo;

GList *get_all_apps(void);
gchar *get_resource_path(const char *rel_path);

#endif
