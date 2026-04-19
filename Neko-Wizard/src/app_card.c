#include "app_card.h"
#include "installer.h"

struct _NekoAppCard {
    GtkWidget parent_instance;
    AppInfo *info;
    GtkWidget *box;
    GtkWidget *icon_image;
    GtkWidget *name_label;
};

G_DEFINE_TYPE (NekoAppCard, neko_app_card, GTK_TYPE_WIDGET)

void neko_app_card_set_selected(NekoAppCard *self, gboolean selected) {
    self->info->selected = selected;
    if (selected) {
        gtk_widget_add_css_class(GTK_WIDGET(self), "selected");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(self), "selected");
    }
}

gboolean neko_app_card_get_selected(NekoAppCard *self) {
    return self->info->selected;
}

static void on_card_click(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    NekoAppCard *self = NEKO_APP_CARD(user_data);
    neko_app_card_set_selected(self, !self->info->selected);
}

static void neko_app_card_dispose(GObject *object) {
    NekoAppCard *self = (NekoAppCard *)object;
    g_clear_pointer(&self->box, gtk_widget_unparent);
    G_OBJECT_CLASS(neko_app_card_parent_class)->dispose(object);
}

static void neko_app_card_class_init(NekoAppCardClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = neko_app_card_dispose;
    
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
    gtk_widget_class_set_css_name(widget_class, "appcard");
}

static void neko_app_card_init(NekoAppCard *self) {
    self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_parent(self->box, GTK_WIDGET(self));
    
    gtk_widget_set_size_request(GTK_WIDGET(self), 180, 200);
    gtk_widget_set_margin_top(GTK_WIDGET(self), 8);
    gtk_widget_set_margin_bottom(GTK_WIDGET(self), 8);
    gtk_widget_set_margin_start(GTK_WIDGET(self), 8);
    gtk_widget_set_margin_end(GTK_WIDGET(self), 8);
    
    self->icon_image = gtk_image_new();
    gtk_image_set_pixel_size(GTK_IMAGE(self->icon_image), 64);
    gtk_widget_set_halign(self->icon_image, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(self->icon_image, 16);
    gtk_widget_add_css_class(self->icon_image, "card-icon");
    
    self->name_label = gtk_label_new("");
    gtk_widget_add_css_class(self->name_label, "card-title");
    gtk_label_set_wrap(GTK_LABEL(self->name_label), TRUE);
    gtk_label_set_justify(GTK_LABEL(self->name_label), GTK_JUSTIFY_CENTER);
    gtk_widget_set_halign(self->name_label, GTK_ALIGN_CENTER);
    
    gtk_box_append(GTK_BOX(self->box), self->icon_image);
    gtk_box_append(GTK_BOX(self->box), self->name_label);
    
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(self->box), spacer);
    
    // Add clickable area
    GtkGesture *click_gesture = gtk_gesture_click_new();
    g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_card_click), self);
    gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(click_gesture));
}

GtkWidget *neko_app_card_new(AppInfo *info) {
    NekoAppCard *card = g_object_new(NEKO_TYPE_APP_CARD, NULL);
    card->info = info;
    
    char *rel_path = g_strdup_printf("resources/%s", info->icon_path);
    char *full_path = get_resource_path(rel_path);
    
    gtk_image_set_from_file(GTK_IMAGE(card->icon_image), full_path);
    
    g_free(rel_path);
    g_free(full_path);
    
    char *markup = g_markup_printf_escaped("<b>%s</b>", info->name);
    gtk_label_set_markup(GTK_LABEL(card->name_label), markup);
    g_free(markup);
    
    return GTK_WIDGET(card);
}
