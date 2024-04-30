#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ncurses.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// Shows status bar
WINDOW *status_win;

#define MAX_FILENAME_LENGTH 256
#define SHOW_SELECTION_COUNT 1


int is_regular_file(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISDIR(path_stat.st_mode)) {
        // The path is a directory
        return 0;
    } else if (S_ISREG(path_stat.st_mode)) {
        // The path is a regular file
        return 1;
    } else {
        // The path is neither a directory nor a regular file
        return -1;
    }
}

typedef struct {
    char name[MAX_FILENAME_LENGTH];
    int isDir;
} FileEntry;

typedef struct {
    FileEntry *entries;
    int count;
    int selected;
} FileList;

void initializeCurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

void cleanupCurses() {
    endwin();
}
void drawFileList(WINDOW* win, const FileList* fileList, int panelNumber) {
    int i;

    werase(win);
    box(win, 0, 0);

    if (fileList->count == 0) {
        mvwprintw(win, 1, 1, "No files");
    } else if (fileList->count == 1) {
        mvwprintw(win, 1, 1, "%s", fileList->entries[0].name);
    } else {
        for (i = 0; i < fileList->count; i++) {
            if (i == fileList->selected) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, i + 1, 1, "%s", fileList->entries[i].name);
            wattroff(win, A_REVERSE);
        }
    }

    wrefresh(win);
}

int compareEntries(const void *a, const void *b) {
    const FileEntry *entryA = (const FileEntry *)a;
    const FileEntry *entryB = (const FileEntry *)b;

    if (entryA->isDir && !entryB->isDir) {
        return -1;  // Directories should appear before files
    } else if (!entryA->isDir && entryB->isDir) {
        return 1;   // Files should appear after directories
    } else {
        return strcmp(entryA->name, entryB->name);
    }
}

void populateFileList(FileList *fileList, const char *path) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        printf("Failed to open directory: %s\n", path);
        return;
    }

    fileList->count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (stat(fullpath, &entry_stat) == 0) {
            fileList->entries[fileList->count].isDir = S_ISDIR(entry_stat.st_mode);
        } else {
            fileList->entries[fileList->count].isDir = 0;
        }
        strcpy(fileList->entries[fileList->count].name, entry->d_name);
        fileList->count++;
    }

    closedir(dir);

    // Sort the file list
    qsort(fileList->entries, fileList->count, sizeof(FileEntry), compareEntries);
}

int main() {
    initializeCurses();

    char path1[PATH_MAX] = "/home/seno/left";
    char path2[PATH_MAX] = "/home/seno/right";

   
    FileList fileList1, fileList2;
    fileList1.entries = malloc(sizeof(FileEntry) * 100); // Allocate memory for entries
    fileList2.entries = malloc(sizeof(FileEntry) * 100); // Allocate memory for entries
    fileList1.count = 0;
    fileList2.count = 0;
    fileList1.selected = 0;

    WINDOW *win1 = newwin(LINES, COLS / 2, 0, 0);
    WINDOW *win2 = newwin(LINES, COLS / 2, 0, (COLS + 1) / 2);
int activePanel = 1; // 1 for panel 1 (left), 2 for panel 2 (right)

    while (1) {
        populateFileList(&fileList1, path1);
        populateFileList(&fileList2, path2);

        drawFileList(win1, &fileList1, 1);
        drawFileList(win2, &fileList2,2);

        int ch = getch();

    switch (ch) {
        case KEY_UP:
            if (activePanel == 1) {
                if (fileList1.selected > 0) {
                    fileList1.selected--;
                }
            } else if (activePanel == 2) {
                if (fileList2.selected > 0) {
                    fileList2.selected--;
                }
            }
            break;
        case KEY_DOWN:
            if (activePanel == 1) {
                if (fileList1.selected < fileList1.count - 1) {
                    fileList1.selected++;
                }
            } else if (activePanel == 2) {
                if (fileList2.selected < fileList2.count - 1) {
                    fileList2.selected++;
                }
            }
            break;
        case KEY_LEFT:
    if (activePanel == 1) {
        if (strcmp(path1, "/") != 0) {
            char *newPath = dirname(strdup(path1));
            strncpy(path1, newPath, PATH_MAX - 1);
            path1[PATH_MAX - 1] = '\0';
            free(newPath);
        }
    } else if (activePanel == 2) {
        if (strcmp(path2, "/") != 0) {
            char *newPath = dirname(strdup(path2));
            strncpy(path2, newPath, PATH_MAX - 1);
            path2[PATH_MAX - 1] = '\0';
            free(newPath);
        }
    }
            break;
        case KEY_RIGHT:
            if (activePanel == 1) {
                if (fileList1.entries[fileList1.selected].isDir) {
                    strcat(path1, "/");
                    strcat(path1, fileList1.entries[fileList1.selected].name);
                }
            } else if (activePanel == 2) {
                if (fileList2.entries[fileList2.selected].isDir) {
                    strcat(path2, "/");
                    strcat(path2, fileList2.entries[fileList2.selected].name);
                }
            }
            break;
        case '\t': // Tab key
            activePanel = (activePanel == 1) ? 2 : 1;
            break;
        case 'q':
            cleanupCurses();
            exit(0);
    }
}

    return 0;
}
