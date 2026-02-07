//COMPILE With gcc -o mate_monitor mate-monitor.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define CHECK_INTERVAL 5

int silent_mode = 0;

void signal_handler(int sig) {
    if (!silent_mode) printf("\nMonitor detenido.\n");
    exit(0);
}

int count_desktop_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return 0;
    
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".desktop") == 0) {
            count++;
        }
    }
    closedir(dir);
    return count;
}

void execute_pkill() {
    if (!silent_mode) printf("Ejecutando pkill mate-panel...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo
        setsid();
        if (silent_mode) {
            // Modo silencioso
            execlp("pkill", "pkill", "mate-panel", NULL);
        } else {
            // Modo normal
            execlp("pkill", "pkill", "mate-panel", NULL);
        }
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    // Verificar argumento silencioso
    if (argc > 1 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--silent") == 0)) {
        silent_mode = 1;
    }
    
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Uso: %s [-s|--silent]\n", argv[0]);
        printf("Monitoriza cambios en archivos .desktop y ejecuta pkill mate-panel\n");
        return 0;
    }
    
    if (!silent_mode) {
        printf("Monitor de archivos .desktop (pkill version)\n");
        printf("Monitoreando cambios cada %d segundos...\n", CHECK_INTERVAL);
        printf("Presiona Ctrl+C para salir\n\n");
    }
    
    signal(SIGINT, signal_handler);
    
    char home_path[256];
    snprintf(home_path, sizeof(home_path), "%s/.local/share/applications", getenv("HOME"));
    
    int old_local_count = count_desktop_files(home_path);
    int old_usr_count = count_desktop_files("/usr/share/applications");
    
    if (!silent_mode) {
        printf("Archivos iniciales: \n");
        printf("  %s: %d archivos .desktop\n", home_path, old_local_count);
        printf("  /usr/share/applications: %d archivos .desktop\n\n", old_usr_count);
    }
    
    while (1) {
        sleep(CHECK_INTERVAL);
        
        int new_local_count = count_desktop_files(home_path);
        int new_usr_count = count_desktop_files("/usr/share/applications");
        
        if (new_local_count != old_local_count || new_usr_count != old_usr_count) {
            if (!silent_mode) {
                time_t now = time(NULL);
                struct tm *tm_info = localtime(&now);
                char time_str[20];
                strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
                printf("[%s] Cambios detectados!\n", time_str);
                printf("  Local: %d -> %d archivos\n", old_local_count, new_local_count);
                printf("  Usr: %d -> %d archivos\n", old_usr_count, new_usr_count);
            }
            
            old_local_count = new_local_count;
            old_usr_count = new_usr_count;
            
            execute_pkill();
        }
    }
    
    return 0;
}