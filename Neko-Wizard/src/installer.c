#include "installer.h"
#include <gio/gio.h>

typedef struct {
    InstallProgressCallback progress_cb;
    InstallFinishedCallback finished_cb;
    gpointer user_data;
    GSubprocess *subprocess;
    GDataInputStream *stream;
} InstallTask;

static void on_line_read(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    InstallTask *task = user_data;
    gsize length;
    GError *error = NULL;
    char *line = g_data_input_stream_read_line_finish(task->stream, res, &length, &error);
    
    if (line != NULL) {
        if (task->progress_cb) {
            task->progress_cb(line, task->user_data);
        }
        g_free(line);
        g_data_input_stream_read_line_async(task->stream, G_PRIORITY_DEFAULT, NULL, on_line_read, task);
    }
}

static void on_process_finished(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    InstallTask *task = user_data;
    GError *error = NULL;
    gboolean success = g_subprocess_wait_finish(G_SUBPROCESS(source_object), res, &error);
    
    if (error) {
        g_warning("Process error: %s", error->message);
        g_clear_error(&error);
        success = FALSE;
    } else {
        success = g_subprocess_get_if_exited(task->subprocess) && (g_subprocess_get_exit_status(task->subprocess) == 0);
    }
    
    if (task->finished_cb) {
        task->finished_cb(success, task->user_data);
    }
    
    g_clear_object(&task->stream);
    g_clear_object(&task->subprocess);
    g_free(task);
}

void install_app_async(const char *command, InstallProgressCallback progress_cb, InstallFinishedCallback finished_cb, gpointer user_data) {
    InstallTask *task = g_new0(InstallTask, 1);
    task->progress_cb = progress_cb;
    task->finished_cb = finished_cb;
    task->user_data = user_data;
    
    GError *error = NULL;
    task->subprocess = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE,
                                        &error,
                                        "bash", "-c", command, NULL);
    
    if (error) {
        g_warning("Could not launch subprocess: %s", error->message);
        g_clear_error(&error);
        if (finished_cb) finished_cb(FALSE, user_data);
        g_free(task);
        return;
    }
    
    GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(task->subprocess);
    task->stream = g_data_input_stream_new(stdout_stream);
    
    g_data_input_stream_read_line_async(task->stream, G_PRIORITY_DEFAULT, NULL, on_line_read, task);
    g_subprocess_wait_async(task->subprocess, NULL, on_process_finished, task);
}
