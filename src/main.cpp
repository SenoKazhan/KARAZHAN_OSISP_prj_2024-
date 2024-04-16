#include <gtk/gtk.h>

// Обработчик события при выборе "Quit"
void on_quit_clicked(GtkWidget *, gpointer) {
    // Ваш код обработки события
    gtk_main_quit(); // Выход из цикла GTK
}

int main(int argc, char *argv[]) {
    // Инициализация GTK
    gtk_init(&argc, &argv);

    // Создание главного окна
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Main Menu Example");
    gtk_widget_set_size_request(window, 300, 200);

    // Создание менюбара
    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_container_add(GTK_CONTAINER(window), menubar);

    // Создание пункта меню "File"
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);

    // Создание пункта меню "Quit" в подменю "File"
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_quit_clicked), NULL);

    // Обработчик закрытия окна
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Отображение всех виджетов
    gtk_widget_show_all(window);

    // Запуск главного цикла GTK
    gtk_main();
    
    return 0;
}
