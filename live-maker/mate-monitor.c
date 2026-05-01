/*
 * mate_monitor.c – Monitor de cambios en directorios de aplicaciones
 *                  y reinicio automático de mate-panel.
 *
 * Compilación:
 *   gcc -o mate_monitor mate_monitor.c -D_GNU_SOURCE -Wall -Wextra -O2
 *
 * Uso:
 *   ./mate_monitor [-s]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

#define CHECK_INTERVAL    5      // segundos entre verificaciones
#define MAX_PATH_LENGTH   256

// Estructura para cada directorio vigilado
struct FileStats {
    char path[MAX_PATH_LENGTH];
    int  count;
};

// --- Variables globales ---
static volatile sig_atomic_t keep_running = 1;   // bandera de salida gestionada por señales
static bool silent_mode = false;                // activado con -s

// --- Manejador de señales (mínimo y seguro) ---
static void signal_handler(int sig) {
    (void) sig;                 // silencia advertencia
    keep_running = 0;           // pedimos salida ordenada
}

// --- Configuración de señales ---
static void setup_signals(void) {
    struct sigaction sa;

    // Para SIGINT (Ctrl+C) – salida limpia
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // Ignorar SIGCHLD para evitar zombies (el kernel recoge los hijos automáticamente)
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;  // no crear zombies, comportamiento moderno
    sigaction(SIGCHLD, &sa, NULL);
}

// --- Cuenta archivos .desktop en un directorio ---
static int count_desktop_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        // El directorio puede no existir aún; no es un error crítico
        if (!silent_mode)
            fprintf(stderr, "Aviso: no se pudo abrir %s\n", path);
        return 0;
    }

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Saltamos las entradas especiales . y ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        // ¿Termina en .desktop?
        const char *ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".desktop") == 0)
            count++;
    }
    closedir(dir);
    return count;
}

// --- Ejecuta pkill en un proceso hijo ---
static bool execute_pkill(void) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return false;
    }
    if (pid == 0) {
        // Proceso hijo
        setsid();   // nueva sesión, independiza del terminal
        execlp("pkill", "pkill", "mate-panel", (char *)NULL);
        perror("execlp");   // solo se ejecuta si execlp falla
        _exit(EXIT_FAILURE);
    }
    // Padre: no hace wait porque SIGCHLD está ignorado
    return true;
}

// --- Función principal ---
int main(int argc, char *argv[]) {
    // --- Procesar argumentos ---
    if (argc > 1) {
        if (strncmp(argv[1], "-s", 2) == 0) {
            silent_mode = true;
        } else if (strncmp(argv[1], "-h", 2) == 0) {
            fprintf(stderr, "Uso: %s [-s]\n", argv[0]);
            fprintf(stderr, "Monitoriza cambios en .desktop y reinicia mate-panel\n");
            return EXIT_SUCCESS;
        }
    }

    // --- Construir rutas ---
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: la variable HOME no está definida\n");
        return EXIT_FAILURE;
    }
    char home_desktop[MAX_PATH_LENGTH];
    snprintf(home_desktop, sizeof(home_desktop),
             "%s/.local/share/applications", home);

    // --- Inicializar los dos directorios vigilados ---
    struct FileStats dirs[2];
    snprintf(dirs[0].path, sizeof(dirs[0].path), "%s", home_desktop);
    snprintf(dirs[1].path, sizeof(dirs[1].path), "%s", "/usr/share/applications");

    dirs[0].count = count_desktop_files(dirs[0].path);
    dirs[1].count = count_desktop_files(dirs[1].path);

    if (!silent_mode) {
        printf("Monitorizando:\n  %s\n  %s\n", dirs[0].path, dirs[1].path);
        printf("Contadores iniciales: %d, %d\n", dirs[0].count, dirs[1].count);
    }

    // --- Configurar señales ---
    setup_signals();

    // --- Bucle principal ---
    while (keep_running) {
        sleep(CHECK_INTERVAL);
        if (!keep_running) break;  // salir inmediatamente si se recibió señal durante sleep

        if (!silent_mode)
            printf("Verificando cambios...\n");

        bool changed = false;

        // Revisar primer directorio
        int new_count = count_desktop_files(dirs[0].path);
        if (new_count != dirs[0].count) {
            if (!silent_mode)
                printf("Cambio en %s: %d -> %d\n",
                       dirs[0].path, dirs[0].count, new_count);
            dirs[0].count = new_count;
            changed = true;
        }

        // Revisar segundo directorio
        new_count = count_desktop_files(dirs[1].path);
        if (new_count != dirs[1].count) {
            if (!silent_mode)
                printf("Cambio en %s: %d -> %d\n",
                       dirs[1].path, dirs[1].count, new_count);
            dirs[1].count = new_count;
            changed = true;
        }

        // Actuar si hubo cambios
        if (changed) {
            if (!silent_mode)
                printf("Intentando reiniciar mate-panel...\n");
            if (execute_pkill()) {
                if (!silent_mode)
                    printf("mate-panel reiniciado.\n");
            } else {
                if (!silent_mode)
                    fprintf(stderr, "Fallo al ejecutar pkill.\n");
            }
        }
    }

    if (!silent_mode)
        printf("Monitor detenido correctamente.\n");

    return EXIT_SUCCESS;
}
