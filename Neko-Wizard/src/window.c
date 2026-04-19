#include "window.h"
#include "apps_manager.h"
#include "app_card.h"
#include "installer.h"
#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#endif

struct _NekoStoreWindow {
    GtkApplicationWindow parent_instance;
    GtkWidget *stack;
    
    // Welcome Page
    GtkWidget *welcome_page;
    
    // Apps Pages
    GtkWidget *gaming_page;
    GtkWidget *gaming_flowbox;
    
    GtkWidget *drawing_image_page;
    GtkWidget *drawing_image_flowbox;

    GtkWidget *audio_video_page;
    GtkWidget *audio_video_flowbox;
    
    GtkWidget *text_documents_page;
    GtkWidget *text_documents_flowbox;
    
    GtkWidget *social_page;
    GtkWidget *social_flowbox;
    
    // Drivers Page
    GtkWidget *drivers_page;
    GtkWidget *drivers_flowbox;
    
    // Finished Page
    GtkWidget *finished_page;
    GtkWidget *finished_label;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    
    // Installer State
    GList *apps_to_install;
    GList *current_installing;
    guint pulse_id;
};

G_DEFINE_TYPE (NekoStoreWindow, neko_store_window, GTK_TYPE_APPLICATION_WINDOW)

static void go_to_gaming_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->gaming_page);
}

static void go_to_drawing_image_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->drawing_image_page);
}

static void go_to_audio_video_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->audio_video_page);
}

static void go_to_text_documents_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->text_documents_page);
}

static void go_to_social_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->social_page);
}

static void go_to_drivers_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->drivers_page);
}

static void on_group_toggle_toggled(GtkCheckButton *btn, gpointer user_data) {
    GtkWidget *flowbox = GTK_WIDGET(user_data);
    gboolean active = gtk_check_button_get_active(btn);
    
    GtkWidget *child = gtk_widget_get_first_child(flowbox);
    while (child != NULL) {
        GtkWidget *card = gtk_flow_box_child_get_child(GTK_FLOW_BOX_CHILD(child));
        if (NEKO_IS_APP_CARD(card)) {
            neko_app_card_set_selected(NEKO_APP_CARD(card), active);
        }
        child = gtk_widget_get_next_sibling(child);
    }
}

static GtkWidget* create_app_group_page(NekoStoreWindow *self, const char *title, AppGroup group_filter, GtkWidget **flowbox_out, GCallback back_cb, GCallback next_cb, const char *next_btn_label, gboolean is_final_step) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    
    // Header
    GtkWidget *header = gtk_label_new(title);
    gtk_widget_add_css_class(header, "page-header");
    gtk_widget_set_halign(header, GTK_ALIGN_START);
    gtk_widget_set_margin_top(header, 16);
    gtk_widget_set_margin_start(header, 16);
    gtk_box_append(GTK_BOX(vbox), header);
    
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_margin_start(scrolled, 16);
    gtk_widget_set_margin_end(scrolled, 16);
    
    // Content box
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_bottom(content_box, 24);
    
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    
    GtkWidget *toggle_all = gtk_check_button_new_with_label("Select All");
    gtk_widget_set_halign(toggle_all, GTK_ALIGN_END);
    gtk_widget_set_hexpand(toggle_all, TRUE);
    
    gtk_box_append(GTK_BOX(hbox), toggle_all);
    gtk_box_append(GTK_BOX(content_box), hbox);
    
    GtkWidget *flowbox = gtk_flow_box_new();
    gtk_widget_set_valign(flowbox, GTK_ALIGN_START);
    gtk_widget_set_halign(flowbox, GTK_ALIGN_FILL);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flowbox), 6);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flowbox), GTK_SELECTION_NONE);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flowbox), 16);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flowbox), 16);
    
    g_signal_connect(toggle_all, "toggled", G_CALLBACK(on_group_toggle_toggled), flowbox);
    
    gtk_box_append(GTK_BOX(content_box), flowbox);
    *flowbox_out = flowbox;
    
    // Load apps
    GList *apps = get_all_apps();
    for (GList *l = apps; l != NULL; l = l->next) {
        AppInfo *info = (AppInfo *)l->data;
        if (info->group == group_filter) {
            GtkWidget *card = neko_app_card_new(info);
            gtk_flow_box_insert(GTK_FLOW_BOX(flowbox), card, -1);
        }
    }
    g_list_free(apps);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), content_box);
    gtk_box_append(GTK_BOX(vbox), scrolled);
    
    // Footer buttons box
    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_bottom(footer, 16);
    gtk_widget_set_margin_end(footer, 16);
    gtk_widget_set_margin_start(footer, 16);
    
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    
    if (back_cb != NULL) {
        GtkWidget *back_btn = gtk_button_new_with_label("Back");
        gtk_widget_add_css_class(back_btn, "suggested-action");
        gtk_widget_set_halign(back_btn, GTK_ALIGN_START);
        g_signal_connect(back_btn, "clicked", back_cb, self);
        gtk_box_append(GTK_BOX(footer), back_btn);
    }
    
    gtk_box_append(GTK_BOX(footer), spacer);
    
    GtkWidget *next_btn = gtk_button_new_with_label(next_btn_label);
    gtk_widget_add_css_class(next_btn, "suggested-action");
    gtk_widget_add_css_class(next_btn, "install-selected-btn"); // using same styling
    gtk_widget_set_halign(next_btn, GTK_ALIGN_END);
    g_signal_connect(next_btn, "clicked", next_cb, self);
    gtk_box_append(GTK_BOX(footer), next_btn);
    
    gtk_box_append(GTK_BOX(vbox), footer);
    
    return vbox;
}


static void install_next_app(NekoStoreWindow *self);

static gboolean pulse_progress_bar(gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(self->progress_bar));
    return G_SOURCE_CONTINUE;
}

static void install_finished_cb(gboolean success, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    
    if (self->current_installing) {
        AppInfo *info = (AppInfo *)self->current_installing->data;
        info->install_success = success;
    }
    
    self->current_installing = self->current_installing->next;
    install_next_app(self);
}

static void install_progress_cb(const char *status_message, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    if (status_message && g_utf8_validate(status_message, -1, NULL)) {
        char *trunc = g_strndup(status_message, 60);
        gtk_label_set_text(GTK_LABEL(self->status_label), trunc);
        g_free(trunc);
    }
}

static void install_next_app(NekoStoreWindow *self) {
    if (self->current_installing != NULL) {
        AppInfo *info = (AppInfo *)self->current_installing->data;
        char *status = g_strdup_printf("Installing %s...", info->name);
        gtk_label_set_text(GTK_LABEL(self->status_label), status);
        g_free(status);
        
        install_app_async(info->install_command, install_progress_cb, install_finished_cb, self);
    } else {
        // Finished everything
        if (self->pulse_id > 0) {
            g_source_remove(self->pulse_id);
            self->pulse_id = 0;
        }
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(self->progress_bar), 1.0);
        gtk_label_set_text(GTK_LABEL(self->finished_label), "Neko Void is ready! you can close this window");
        gtk_label_set_text(GTK_LABEL(self->status_label), "All installations finished.");
    }
}

static void system_update_finished_cb(gboolean success, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_label_set_text(GTK_LABEL(self->finished_label), "Installing Apps...");
    install_next_app(self);
}

static void on_install_selected_clicked(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    
    // Gather ALL selected apps from all categories
    g_list_free(self->apps_to_install);
    self->apps_to_install = NULL;
    
    GList *apps = get_all_apps();
    for (GList *l = apps; l != NULL; l = l->next) {
        AppInfo *info = (AppInfo *)l->data;
        if (info->selected) {
            self->apps_to_install = g_list_append(self->apps_to_install, info);
        }
    }
    g_list_free(apps);
    
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->finished_page);
    
    gtk_label_set_text(GTK_LABEL(self->finished_label), "Updating System...");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(self->progress_bar), 0.0);
    self->pulse_id = g_timeout_add(100, pulse_progress_bar, self);
    
    self->current_installing = self->apps_to_install;
    char *status = g_strdup("Running pkexec xbps-install -Syu...");
    gtk_label_set_text(GTK_LABEL(self->status_label), status);
    g_free(status);
    install_app_async("pkexec xbps-install -y -Syu", install_progress_cb, system_update_finished_cb, self);
}

static gboolean on_theme_switch_state_set(GtkSwitch *widget, gboolean state, gpointer user_data) {
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", state, NULL);
    
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    if (!state) {
        gtk_widget_add_css_class(GTK_WIDGET(self), "light-mode");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(self), "light-mode");
    }
    
    gtk_switch_set_state(widget, state);
    return TRUE;
}

static gboolean on_language_switch_state_set(GtkSwitch *widget, gboolean state, gpointer user_data) {
    // True = Spanish, False = English
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    
    // The welcome page is the container, its second child is main_content.
    GtkWidget *box = self->welcome_page;
    GtkWidget *main_content = gtk_widget_get_last_child(box);
    
    GtkWidget *icon = gtk_widget_get_first_child(main_content);
    GtkWidget *title = gtk_widget_get_next_sibling(icon);
    GtkWidget *subtitle = gtk_widget_get_next_sibling(title);
    GtkWidget *btn = gtk_widget_get_last_child(main_content); // The button is the last child
    
    if (state) {
        gtk_label_set_text(GTK_LABEL(title), "Bienvenido a Neko Void");
        gtk_label_set_text(GTK_LABEL(subtitle), "Preparemos su sistema con sus aplicaciones favoritas.");
        gtk_button_set_label(GTK_BUTTON(btn), "Iniciar Configuración");
    } else {
        gtk_label_set_text(GTK_LABEL(title), "Welcome to Neko Void");
        gtk_label_set_text(GTK_LABEL(subtitle), "Let's get your system ready with your favorite apps.");
        gtk_button_set_label(GTK_BUTTON(btn), "Start Setup");
    }
    
    gtk_switch_set_state(widget, state);
    return TRUE;
}

static void build_welcome_page(NekoStoreWindow *self) {
    self->welcome_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_valign(self->welcome_page, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(self->welcome_page, GTK_ALIGN_CENTER);
    
    // Top bar containing switches
    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_valign(top_bar, GTK_ALIGN_START);
    gtk_widget_set_hexpand(top_bar, TRUE);
    // Move it to the very top by setting heavy bottom margin, or position it absolutely.
    // For simplicity, we just use expanding space below.
    
    // Dark mode switch (Left)
    GtkWidget *theme_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *theme_icon = gtk_image_new_from_icon_name("weather-clear-night-symbolic");
    GtkWidget *theme_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(theme_switch), TRUE); // Default dark
    // Use g_signal_connect instead of state-set to ensure it triggers correctly
    g_signal_connect(theme_switch, "state-set", G_CALLBACK(on_theme_switch_state_set), self);
    gtk_box_append(GTK_BOX(theme_box), theme_icon);
    gtk_box_append(GTK_BOX(theme_box), theme_switch);
    
    // Spacer
    GtkWidget *spacer_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer_top, TRUE);
    
    // Language switch (Right)
    GtkWidget *lang_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *lang_label_en = gtk_label_new("EN");
    GtkWidget *lang_switch = gtk_switch_new();
    GtkWidget *lang_label_es = gtk_label_new("ES");
    g_signal_connect(lang_switch, "state-set", G_CALLBACK(on_language_switch_state_set), self);
    gtk_box_append(GTK_BOX(lang_box), lang_label_en);
    gtk_box_append(GTK_BOX(lang_box), lang_switch);
    gtk_box_append(GTK_BOX(lang_box), lang_label_es);
    
    gtk_box_append(GTK_BOX(top_bar), theme_box);
    gtk_box_append(GTK_BOX(top_bar), spacer_top);
    gtk_box_append(GTK_BOX(top_bar), lang_box);
    
    GtkWidget *main_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(main_content, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_content, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(main_content, TRUE);
    
    GtkWidget *icon;
    char *logo_path = get_resource_path("resources/logo.png");
    icon = gtk_image_new_from_file(logo_path);
    g_free(logo_path);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 128);
    gtk_widget_add_css_class(icon, "welcome-icon");
    
    GtkWidget *title = gtk_label_new("Welcome to Neko Void");
    gtk_widget_add_css_class(title, "welcome-title");
    
    GtkWidget *subtitle = gtk_label_new("Let's get your system ready with your favorite apps.");
    gtk_widget_add_css_class(subtitle, "welcome-subtitle");
    
    GtkWidget *btn = gtk_button_new_with_label("Start Setup");
    gtk_widget_add_css_class(btn, "suggested-action");
    gtk_widget_add_css_class(btn, "start-btn");
    g_signal_connect(btn, "clicked", G_CALLBACK(go_to_gaming_page), self);
    
    gtk_box_append(GTK_BOX(main_content), icon);
    gtk_box_append(GTK_BOX(main_content), title);
    gtk_box_append(GTK_BOX(main_content), subtitle);
    
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(spacer, -1, 32);
    gtk_box_append(GTK_BOX(main_content), spacer);
    
    gtk_box_append(GTK_BOX(main_content), btn);
    
    // Assemble the whole structure
    // We want the top bar at the strict top, and main content centered.
    GtkWidget *welcome_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(top_bar, 16);
    gtk_widget_set_margin_start(top_bar, 16);
    gtk_widget_set_margin_end(top_bar, 16);
    
    gtk_box_append(GTK_BOX(welcome_container), top_bar);
    gtk_box_append(GTK_BOX(welcome_container), main_content);
    
    // Re-assign self->welcome_page to the container, since it expects a widget to be added to the stack
    self->welcome_page = welcome_container;
}

static void build_finished_page(NekoStoreWindow *self) {
    self->finished_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_valign(self->finished_page, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(self->finished_page, GTK_ALIGN_CENTER);
    
    self->finished_label = gtk_label_new("Installing Apps...");
    gtk_widget_add_css_class(self->finished_label, "welcome-title");
    
    self->progress_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(self->progress_bar, 400, -1);
    
    self->status_label = gtk_label_new("Preparing...");
    gtk_widget_add_css_class(self->status_label, "dim-label");
    
    gtk_box_append(GTK_BOX(self->finished_page), self->finished_label);
    
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(spacer, -1, 32);
    gtk_box_append(GTK_BOX(self->finished_page), spacer);
    
    gtk_box_append(GTK_BOX(self->finished_page), self->progress_bar);
    gtk_box_append(GTK_BOX(self->finished_page), self->status_label);
}

static void go_to_welcome_page(GtkButton *btn, gpointer user_data) {
    NekoStoreWindow *self = NEKO_STORE_WINDOW(user_data);
    gtk_stack_set_visible_child(GTK_STACK(self->stack), self->welcome_page);
}

static void neko_store_window_init (NekoStoreWindow *self) {
    self->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(self->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    build_welcome_page(self);
    
    self->gaming_page = create_app_group_page(self, "Step 1: Gaming Apps", GROUP_GAMING, &self->gaming_flowbox, G_CALLBACK(go_to_welcome_page), G_CALLBACK(go_to_drawing_image_page), "Next", FALSE);
    self->drawing_image_page = create_app_group_page(self, "Step 2: Drawing and Image Editing", GROUP_DRAWING_IMAGE, &self->drawing_image_flowbox, G_CALLBACK(go_to_gaming_page), G_CALLBACK(go_to_audio_video_page), "Next", FALSE);
    self->audio_video_page = create_app_group_page(self, "Step 3: Audio & Video Editing", GROUP_AUDIO_VIDEO, &self->audio_video_flowbox, G_CALLBACK(go_to_drawing_image_page), G_CALLBACK(go_to_text_documents_page), "Next", FALSE);
    self->text_documents_page = create_app_group_page(self, "Step 4: Text Editing and Documents", GROUP_TEXT_DOCUMENTS, &self->text_documents_flowbox, G_CALLBACK(go_to_audio_video_page), G_CALLBACK(go_to_social_page), "Next", FALSE);
    self->social_page = create_app_group_page(self, "Step 5: Social Apps and Internet", GROUP_SOCIAL, &self->social_flowbox, G_CALLBACK(go_to_text_documents_page), G_CALLBACK(go_to_drivers_page), "Next", FALSE);
    self->drivers_page = create_app_group_page(self, "Step 6: Drivers", GROUP_DRIVERS, &self->drivers_flowbox, G_CALLBACK(go_to_social_page), G_CALLBACK(on_install_selected_clicked), "Install", TRUE);
    
    build_finished_page(self);
    
    gtk_stack_add_named(GTK_STACK(self->stack), self->welcome_page, "welcome");
    gtk_stack_add_named(GTK_STACK(self->stack), self->gaming_page, "gaming");
    gtk_stack_add_named(GTK_STACK(self->stack), self->drawing_image_page, "drawing");
    gtk_stack_add_named(GTK_STACK(self->stack), self->audio_video_page, "audio_video");
    gtk_stack_add_named(GTK_STACK(self->stack), self->text_documents_page, "text_documents");
    gtk_stack_add_named(GTK_STACK(self->stack), self->social_page, "social");
    gtk_stack_add_named(GTK_STACK(self->stack), self->drivers_page, "drivers");
    gtk_stack_add_named(GTK_STACK(self->stack), self->finished_page, "finished");
    
    gtk_window_set_child(GTK_WINDOW(self), self->stack);
}

static void neko_store_window_dispose(GObject *object) {
    NekoStoreWindow *self = (NekoStoreWindow *)object;
    if (self->pulse_id > 0) {
        g_source_remove(self->pulse_id);
        self->pulse_id = 0;
    }
    if (self->apps_to_install) {
        g_list_free(self->apps_to_install);
        self->apps_to_install = NULL;
    }
    G_OBJECT_CLASS(neko_store_window_parent_class)->dispose(object);
}

static void on_window_map(GtkWidget *widget, gpointer data) {
#ifdef GDK_WINDOWING_X11
    GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(widget));
    if (surface && GDK_IS_X11_SURFACE(surface)) {
        Display *xdisplay = gdk_x11_display_get_xdisplay(gdk_surface_get_display(surface));
        Window xid = gdk_x11_surface_get_xid(surface);
        
        int w = gtk_widget_get_width(widget);
        int h = gtk_widget_get_height(widget);
        if (w <= 0) w = 1000;
        if (h <= 0) h = 700;
        
        int screen = DefaultScreen(xdisplay);
        int sw = DisplayWidth(xdisplay, screen);
        int sh = DisplayHeight(xdisplay, screen);
        
        if (sw > w && sh > h) {
            XMoveWindow(xdisplay, xid, (sw - w) / 2, (sh - h) / 2);
        }
    }
#endif
}

static void neko_store_window_class_init (NekoStoreWindowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = neko_store_window_dispose;
}

NekoStoreWindow *neko_store_window_new (GtkApplication *app) {
    NekoStoreWindow *window = g_object_new (NEKO_STORE_TYPE_WINDOW, "application", app, NULL);
    
    gtk_window_set_title (GTK_WINDOW (window), "Neko Void Setup");
    gtk_window_set_default_size (GTK_WINDOW (window), 1000, 700);
    
    GtkWidget *header = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    
    // Provide a greeting
    const char *homedir;
    struct passwd *pw;
    pw = getpwuid(getuid());
    if (pw) {
        char *greeting = g_strdup_printf("Neko Void - %s", pw->pw_name);
        GtkWidget *label = gtk_label_new(greeting);
        g_free(greeting);
        gtk_widget_add_css_class(label, "title-label");
        gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), label);
    }
    
    GtkCssProvider *provider = gtk_css_provider_new();
    char *css_path = get_resource_path("data/style.css");
    GFile *css_file = g_file_new_for_path(css_path);
    gtk_css_provider_load_from_file(provider, css_file);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    g_object_unref(css_file);
    g_free(css_path);
    
    g_signal_connect(window, "map", G_CALLBACK(on_window_map), NULL);
    
    gtk_window_present (GTK_WINDOW (window));
    return window;
}
