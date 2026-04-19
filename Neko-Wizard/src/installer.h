#ifndef INSTALLER_H
#define INSTALLER_H

#include <glib.h>

typedef void (*InstallProgressCallback)(const char *status_message, gpointer user_data);
typedef void (*InstallFinishedCallback)(gboolean success, gpointer user_data);

void install_app_async(const char *command, InstallProgressCallback progress_cb, InstallFinishedCallback finished_cb, gpointer user_data);

#endif
