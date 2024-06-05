#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    // For open, O_CREAT, O_WRONLY
#include <unistd.h>   // For dup2, fork, execlp
#include <sys/wait.h> // For waitpid
#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <errno.h>
#include <libgen.h> // For dirname() function
WINDOW *win1, *win2, *status_win, *confirm_win;

int maxy, maxx;
int confirmHeight = 7;
int confirmWidth = 50;
#define MAX_FILENAME_LENGTH 256
#define MAX_SELECTED_FILES 100 // Or any other number you prefer

int is_regular_file(const char *path)
{
    struct stat pathinfo;
    stat(path, &pathinfo);
    return S_ISREG(pathinfo.st_mode);
}

typedef struct
{
    char fullPath[PATH_MAX];
    char name[MAX_FILENAME_LENGTH];
    int isDir;
    off_t size;  // Размер файла
    time_t date; // Дата добавления файла
    char unit;   // Единица измерения ('B', 'K', 'M', 'G')
} FileEntry;

typedef struct
{
    FileEntry *entries;
    int count;
    int selected;
    int topLine;
} FileList;

typedef struct
{
    char **selectedFiles; // Pointer to a pointer to char for storing selected file names
    int numSelectedFiles; // Number of selected files

} SelectedFiles;

void initializeCurses()
{
    initscr();
    getmaxyx(stdscr, maxy, maxx);
    curs_set(0);
    start_color();
    cbreak();
    noecho();

    status_win = newwin(1, maxx, 0, 0);

    // Create a confirmation window
    int confirmY = (maxy - confirmHeight) / 2;
    int confirmX = (maxx - confirmWidth) / 2;

    confirm_win = newwin(confirmHeight, confirmWidth, confirmY, confirmX);
    wattron(status_win, A_BOLD | A_REVERSE);
    mvwprintw(status_win, 0, 0, "Status Window");
    wattroff(status_win, A_BOLD | A_REVERSE);

    refresh();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    keypad(stdscr, TRUE);
}

int compareEntries(const void *a, const void *b)
{
    const FileEntry *entryA = (const FileEntry *)a;
    const FileEntry *entryB = (const FileEntry *)b;

    if (entryA->isDir && !entryB->isDir)
    {
        return -1; // Directories should appear before files
    }
    else if (!entryA->isDir && entryB->isDir)
    {
        return 1; // Files should appear after directories
    }
    else
    {
        return strcmp(entryA->name, entryB->name);
    }
}

void populateFileList(FileList *fileList, const char *path)
{
    struct stat pathinfo;
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir)
    {
        printf("Failed to open directory: %s\n", path);
        return;
    }

    fileList->count = 0;

    // Заполняем информацию о файлах
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (stat(fullpath, &pathinfo) == 0)
        {
            fileList->entries[fileList->count].isDir = S_ISDIR(pathinfo.st_mode);
            fileList->entries[fileList->count].size = pathinfo.st_size;
            fileList->entries[fileList->count].date = pathinfo.st_mtime;
        }
        else
        {
            fileList->entries[fileList->count].isDir = 0;
        }
        strcpy(fileList->entries[fileList->count].name, entry->d_name);
        strcpy(fileList->entries[fileList->count].fullPath, fullpath);
        fileList->count++;
    }

    closedir(dir);

    qsort(fileList->entries, fileList->count, sizeof(FileEntry), compareEntries);
}

void toggleFileSelection(FileList *fileList, SelectedFiles *selectedFiles, int selectedIndex)
{
    if (selectedIndex < 0 || selectedIndex >= fileList->count)
    {
        return; // Ensure the selected index is within the valid range
    }

    char *filePath = fileList->entries[selectedIndex].fullPath;
    int foundIndex = -1;

    // Check if the file is already selected
    for (int i = 0; i < selectedFiles->numSelectedFiles; ++i)
    {
        if (strcmp(selectedFiles->selectedFiles[i], filePath) == 0)
        {
            foundIndex = i;
            break;
        }
    }

    // If the file is found, remove it from the selection
    if (foundIndex != -1)
    {
        free(selectedFiles->selectedFiles[foundIndex]);
        for (int i = foundIndex; i < selectedFiles->numSelectedFiles - 1; ++i)
        {
            selectedFiles->selectedFiles[i] = selectedFiles->selectedFiles[i + 1];
        }
        --selectedFiles->numSelectedFiles;
    }
    else
    {
        // If the file is not found, add it to the selection list
        selectedFiles->selectedFiles = realloc(selectedFiles->selectedFiles, (selectedFiles->numSelectedFiles + 1) * sizeof(char *));
        selectedFiles->selectedFiles[selectedFiles->numSelectedFiles] = strdup(filePath);
        ++selectedFiles->numSelectedFiles;
    }
}

void drawFileList(WINDOW *win, const FileList *fileList, const SelectedFiles *selectedFiles)
{
    int i, yMax, xMax;
    getmaxyx(win, yMax, xMax);

    werase(win);
    box(win, 0, 0);

    int displayLimit = yMax - 2; // Оставляем место для границы
    int start = fileList->topLine;
    int end = fileList->topLine + displayLimit;

    if (fileList->count == 0)
    {
        mvwprintw(win, 1, 1, "No files");
    }
    else
    {
        for (i = start; i < fileList->count && i < end; i++)
        {
            int line = i - start + 1;
            if (i == fileList->selected)
            {
                wattron(win, A_REVERSE);
            }

            if (fileList->entries[i].isDir)
            {
                wattron(win, A_BOLD);
                wattron(win, COLOR_PAIR(1));
            }

            // Отображаем имя файла
            mvwprintw(win, line, 1, "%-30s", fileList->entries[i].name);

            if (fileList->entries[i].isDir)
            {
                wattroff(win, COLOR_PAIR(1));
                wattroff(win, A_BOLD);
            }

            // Отображаем размер файла с единицей измерения
            if (!fileList->entries[i].isDir)
            {
                long long size = fileList->entries[i].size;
                char unit = 'B';
                if (size >= 1024)
                {
                    size /= 1024;
                    unit = 'K';
                }
                if (size >= 1024)
                {
                    size /= 1024;
                    unit = 'M';
                }
                if (size >= 1024)
                {
                    size /= 1024;
                    unit = 'G'; // делим, пока не приведём к нужной единице измерения
                }
                mvwprintw(win, line, xMax - 20, "%5lld%c", size, unit);
            }
            else
            {
                mvwprintw(win, line, xMax - 20, "%10s", "<DIR>");
            }

            // Отображаем дату добавления файла
            char dateStr[30];
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", localtime(&fileList->entries[i].date));
            mvwprintw(win, line, xMax - 40, "%s", dateStr);

            // Проверяем, выделен ли файл
            int isSelected = 0;
            for (int j = 0; j < selectedFiles->numSelectedFiles; j++)
            {
                if (strcmp(fileList->entries[i].fullPath, selectedFiles->selectedFiles[j]) == 0)
                {
                    isSelected = 1;
                    break;
                }
            }

            // Отображаем крестик для выделенных файлов
            if (isSelected)
            {
                wattron(win, A_REVERSE);
                mvwprintw(win, line, xMax - 5, "[x]");
                wattroff(win, A_REVERSE);
            }

            wattroff(win, A_REVERSE);
        }
    }

    wrefresh(win);
}

void updateStatusBar(const char *path, const char *fileName)
{
    werase(status_win);
    wattron(status_win, A_BOLD | A_REVERSE);
    mvwprintw(status_win, 0, 0, "Status Window: %s/%s", path, fileName);
    wattroff(status_win, A_BOLD | A_REVERSE);
    wrefresh(status_win);
}

void handleResize(FileList *fileList1, FileList *fileList2, WINDOW **win1, WINDOW **win2)
{
    getmaxyx(stdscr, maxy, maxx);
    wresize(status_win, 1, maxx);
    mvwin(status_win, 0, 0);
    wresize(*win1, maxy - 1, maxx / 2 + 2);
    wresize(*win2, maxy - 1, maxx / 2 - 1);
    mvwin(*win2, 1, maxx / 2 + 1);
}

void logToFile(const char *filePath, const char *directory)
{
    FILE *file = fopen("/home/seno/proj/project/test.txt", "a");
    if (file == NULL)
    {
        printf("Failed to open file for writing.\n");
        return;
    }

    fprintf(file, "File Path: %s\n", filePath);
    fprintf(file, "Directory: %s\n", directory);

    fclose(file);
}
// Извлечение из архива
void extractArchive(const char *filePath)
{
    // Создаем копию filePath
    char *directory = strdup(filePath);
    if (directory == NULL)
    {
        // Ошибка выделения памяти
        printf("Error extracting archive: Unable to allocate memory.\n");
        return;
    }

    // Получаем имя каталога из копии filePath
    char *dirName = dirname(directory);

    // Записываем информацию о filePath и directory в файл
    logToFile(filePath, dirName);
    endwin();
    pid_t pid = fork();
    if (pid == 0)
    {
        // Дочерний процесс
        char command[1024];
        const char *extension = strrchr(filePath, '.');
        if (extension != NULL)
        {
            extension++; // Пропускаем точку
            if (strcmp(extension, "rar") == 0)
            {
                snprintf(command, sizeof(command), "rar x %s %s", filePath, dirName);
            }
            else if (strcmp(extension, "gz") == 0)
            {
                snprintf(command, sizeof(command), "gzip -d %s -c %s", filePath, dirName);
            }
            else if (strcmp(extension, "bz2") == 0)
            {
                snprintf(command, sizeof(command), "bzip2 -d %s -c %s", filePath, dirName);
            }
            else if (strcmp(extension, "zip") == 0)
            {
                snprintf(command, sizeof(command), "unzip -d %s %s", dirName, filePath);
            }
            else
            {
                exit(EXIT_FAILURE); // Неизвестное расширение
            }
        }
        else
        {
            exit(EXIT_FAILURE); // Расширение не найдено
        }

        // Выполнение команды
        execl("/bin/sh", "sh", "-c", command, (char *)0);
        exit(EXIT_FAILURE); // В случае ошибки exec
    }
    else if (pid < 0)
    {
        // Ошибка при создании дочернего процесса
        printf("Failed to fork process.\n");
    }
    else
    {
        // Родительский процесс - ожидаем завершения дочернего
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            // Код завершения не нулевой, возможно, произошла ошибка
            printf("Error extracting archive.\n");
        }
    }
    refresh();
    free(directory); // Освобождаем память, выделенную для копии filePath
}

void openFileWithEvince(const char *filepath)
{
    char command[1024];
    snprintf(command, sizeof(command), "evince \"%s\" >/dev/null 2>&1 &", filepath);
    system(command);
}
//Помещаем filepath (полный путь к файлу) в двойные кавычки "
//Без кавычек, если в пути есть пробелы, система будет воспринимать их как разделители аргументов, что приведет к неправильному открытию файла.

void openTextFile(const char *path, const char *fileName)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Change the working directory to the specified path
        if (chdir(path) == -1)
        {
            perror("chdir failed");
            exit(EXIT_FAILURE);
        }

        // Construct the full path to the file
        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, fileName);

        // Debug print to check the full path
        printf("Opening file with Vim: %s\n", fullPath);
        fflush(stdout);

        // Open the file with Vim
        execlp("vim", "vim", fullPath, NULL);
        perror("execlp failed"); // If execlp fails
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        mvwprintw(status_win, 0, 0, "Error opening file.");
        wrefresh(status_win);
    }
    else
    {
        // Wait for Vim to exit
        waitpid(pid, NULL, 0);

        // Clear the screen and refresh
        endwin();
        refresh();
    }
}

void openFile(const char *path, const char *fileName)
{
    char fullPath[PATH_MAX];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", path, fileName);

    const char *extension = strrchr(fileName, '.');
    if (extension)
    {
        extension++;
        if (strcasecmp(extension, "txt") == 0 || strcasecmp(extension, "c") == 0 || strcasecmp(extension, "h") == 0)
        {
            openTextFile(path, fileName);
            initializeCurses();
        }
        else if (strcasecmp(extension, "pdf") == 0)
        {
            refresh();
            openFileWithEvince(fullPath);
        }
        else if (strcasecmp(extension, "jpg") == 0 || strcasecmp(extension, "jpeg") == 0 || strcasecmp(extension, "png") == 0 || strcasecmp(extension, "gif") == 0)
        {
            refresh();
            char command[1024];
            snprintf(command, sizeof(command), "feh \"%s\"", fullPath);
            system(command);
        }
        else
        {
            mvwprintw(status_win, 0, 0, "Unsupported file type: %s", fileName);
            wrefresh(status_win);
        }
    }
    else
    {
        openTextFile(path, fileName);
        initializeCurses();
    }
}

// Функция для рекурсивного удаления директорий
void removeDirectoryRecursively(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat pathinfo; // Локальная переменная
        if (stat(fullPath, &pathinfo) == 0)
        {
            if (S_ISDIR(pathinfo.st_mode))
            {
                removeDirectoryRecursively(fullPath);
            }
            else
            {
                remove(fullPath);
            }
        }
    }
    closedir(dir);
    rmdir(path);
}

// Функция для удаления файла
void removeFile(const char *path, const char *fileName)
{
    box(confirm_win, 0, 0);
    int confirmWidth = 50;
    mvwprintw(confirm_win, 2, (confirmWidth - strlen("Are you sure you want to delete")) / 2, "Are you sure you want to delete");
    mvwprintw(confirm_win, 3, (confirmWidth - strlen(fileName) - 1) / 2, "%s?", fileName);
    mvwprintw(confirm_win, 4, (confirmWidth - strlen("Press 'y' to confirm or 'n' to cancel.")) / 2, "Press 'y' to confirm or 'n' to cancel.");
    wrefresh(confirm_win);

    // Wait for user confirmation
    int ch;
    while ((ch = getch()) != 'y' && ch != 'n')
    {
        // Display error message if invalid input
        mvwprintw(confirm_win, 6, (confirmWidth - strlen("Invalid input. Press 'y' or 'n'.")) / 2, "Invalid input. Press 'y' or 'n'.");
        wrefresh(confirm_win);
    }

    // Clear the confirmation window
    werase(confirm_win);
    box(confirm_win, 0, 0);

    // Обработка ввода пользователя
    if (ch == 'y')
    {
        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, fileName);

        struct stat statinfo; // Локальная переменная
        if (stat(fullPath, &statinfo) == 0)
        {
            if (S_ISDIR(statinfo.st_mode))
            {
                removeDirectoryRecursively(fullPath);
            }
            else
            {
                if (remove(fullPath) == 0)
                {
                    mvwprintw(confirm_win, 3, (confirmWidth - strlen("Deleted successfully")) / 2, "Deleted successfully");
                    wrefresh(confirm_win);
                    getch();
                }
                else
                {
                    mvwprintw(confirm_win, 3, (confirmWidth - strlen("Failed to delete")) / 2, "Failed to delete");
                    wrefresh(confirm_win);
                    getch();
                }
            }
        }
    }
    werase(confirm_win);
    wrefresh(confirm_win);
}

int getUserChoice()
{
    wclear(confirm_win);
    box(confirm_win, 0, 0);

    const char *message = "File already exists. What do you want to do?";
    const char *choices = "(o)verwrite, (s)kip";
    int message_len = strlen(message);
    int choices_len = strlen(choices);

    mvwprintw(confirm_win, 2, (confirmWidth - message_len) / 2, "%s", message);
    mvwprintw(confirm_win, 4, (confirmWidth - choices_len) / 2, "%s", choices);
    wrefresh(confirm_win);

    int ch = wgetch(confirm_win);
    while (ch != 'o' && ch != 's')
    {
        ch = wgetch(confirm_win);
    }
    return ch;
}

void moveFile(const char *srcPath, const char *destPath, const char *fileName)
{
    struct stat pathinfo;
    char srcFullPath[PATH_MAX];
    char destFullPath[PATH_MAX];
    snprintf(srcFullPath, sizeof(srcFullPath), "%s/%s", srcPath, fileName);
    snprintf(destFullPath, sizeof(destFullPath), "%s/%s", destPath, fileName);

    if (stat(destFullPath, &pathinfo) == 0)
    {
        int choice = getUserChoice();
        if (choice == 's')
        {
            return; // Skip the file
        }
    }

    if (rename(srcFullPath, destFullPath) != 0)
    {
        perror("Error moving file");
    }
}

void copyFile(const char *srcPath, const char *destPath, const char *fileName)
{
    char srcFullPath[PATH_MAX];
    char destFullPath[PATH_MAX];
    struct stat pathinfo;
    snprintf(srcFullPath, PATH_MAX, "%s/%s", srcPath, fileName);
    snprintf(destFullPath, PATH_MAX, "%s/%s", destPath, fileName);
    if (stat(destFullPath, &pathinfo) == 0)
    {
        int choice = getUserChoice();
        if (choice == 's')
        {
            return; // Skip the file
        }
    }

    FILE *srcFile = fopen(srcFullPath, "rb");
    if (srcFile == NULL)
    {
        perror("Error opening source file");
        return;
    }

    FILE *destFile = fopen(destFullPath, "wb");
    if (destFile == NULL)
    {
        perror("Error opening destination file");
        fclose(srcFile);
        return;
    }

    char buffer[BUFSIZ];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFSIZ, srcFile)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, destFile) != bytesRead)
        {
            perror("Error writing to destination file");
            fclose(srcFile);
            fclose(destFile);
            return;
        }
    }

    if (ferror(srcFile))
    {
        perror("Error reading from source file");
    }

    fclose(srcFile);
    fclose(destFile);
}

void createDirectory(const char *basePath, const char *dirName)
{
    char fullPath[PATH_MAX];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, dirName);

    if (mkdir(fullPath, 0755) == 0)
    {
        mvprintw(LINES - 1, 0, "Directory created: %s", fullPath);
    }
    else
    {
        mvprintw(LINES - 1, 0, "Failed to create directory: %s", fullPath);
    }
    clrtoeol();
    refresh();
}

void createArchive(const char *archiveName, const char *files[], int numFiles, const char *currentPath)
{
    if (numFiles == 0)
    {
        return; // No files to archive
    }
    struct stat pathinfo;
    char command[2048]; // Increased buffer size to accommodate longer file paths
    snprintf(command, sizeof(command), "rar a -ep \"%s/%s\" ", currentPath, archiveName);
    // tar -cf name.tar.gz -C /home/seno/proj/fciles .
    //  a create, -ep Exclude paths from names
    for (int i = 0; i < numFiles; i++)
    {
        if (stat(files[i], &pathinfo) == 0)
        {

            strcat(command, "\""); // Enclose file paths in double quotes to handle spaces
            strcat(command, files[i]);
            strcat(command, "\" ");
        }
    }

    endwin();        // Close curses window
    system(command); // Execute the command to create the RAR archive
}

void createDirectoryOrArchive(const char *currentPath, SelectedFiles *selectedFiles)
{
    WINDOW *confirm_win = newwin(3, COLS - 2, LINES - 4, 1);
    box(confirm_win, 0, 0);
    mvwprintw(confirm_win, 1, 2, "Create (d)irectory or (a)rchive?");
    wrefresh(confirm_win);

    int choice = wgetch(confirm_win);
    werase(confirm_win);
    wrefresh(confirm_win);
    delwin(confirm_win);

    if (choice == 'd' || choice == 'a')
    {
        char name[MAX_FILENAME_LENGTH];
        WINDOW *input_win = newwin(3, COLS - 2, LINES - 4, 1);
        box(input_win, 0, 0);
        mvwprintw(input_win, 1, 2, "Enter name: ");
        wrefresh(input_win);

        echo();
        mvwgetnstr(input_win, 1, 13, name, sizeof(name) - 1);
        noecho();

        werase(input_win);
        wrefresh(input_win);
        delwin(input_win);
        refresh();

        if (choice == 'd')
        {
            createDirectory(currentPath, name);
        }
        else if (choice == 'a')
        {
            if (selectedFiles->numSelectedFiles == 0)
            {
                mvprintw(LINES - 1, 0, "No files selected for archive.");
                clrtoeol();
                refresh();
                return;
            }
            createArchive(name, (const char **)selectedFiles->selectedFiles, selectedFiles->numSelectedFiles, currentPath);
            for (int i = 0; i < selectedFiles->numSelectedFiles; i++)
            {
                free(selectedFiles->selectedFiles[i]);
            }
            selectedFiles->numSelectedFiles = 0;
        }
    }
}

void renameFile(const char *path, const char *fileName)
{
    char newFileName[MAX_FILENAME_LENGTH];
    WINDOW *input_win = newwin(3, COLS - 2, LINES - 4, 1);
    box(input_win, 0, 0);
    const char *prompt = "Enter new name for %s: ";
    int prompt_len = strlen(prompt) + strlen(fileName);
    mvwprintw(input_win, 1, 2, prompt, fileName);
    wrefresh(input_win);

    echo(); // Включаем отображение введенных символов
    mvwgetnstr(input_win, 1, prompt_len - 1, newFileName, sizeof(newFileName) - 1);
    noecho(); // Выключаем отображение введенных символов

    // Очищаем и обновляем input_win
    werase(input_win);
    wrefresh(input_win);
    delwin(input_win);

    // Получаем расширение исходного имени файла, если оно есть
    const char *extension = strrchr(fileName, '.');
    if (extension)
    {
        // Сохраняем расширение в новом имени файла
        snprintf(newFileName + strlen(newFileName), sizeof(newFileName) - strlen(newFileName), "%s", extension);
    }

    // Полный путь к текущему файлу
    char oldPath[MAX_FILENAME_LENGTH];
    snprintf(oldPath, sizeof(oldPath), "%s/%s", path, fileName);

    // Полный путь к новому файлу
    char newPath[MAX_FILENAME_LENGTH];
    snprintf(newPath, sizeof(newPath), "%s/%s", path, newFileName);

    // Переименовываем файл с помощью функции rename
    if (rename(oldPath, newPath) == 0)
    {
        // Обновляем status_win с сообщением об успешном переименовании
        mvprintw(LINES - 1, 0, "File renamed successfully to %s.", newFileName);
    }
    else
    {
        // Обновляем status_win с сообщением о неудаче
        mvprintw(LINES - 1, 0, "Failed to rename file.");
    }
    clrtoeol();
    refresh();
}

void getInfo(char *filepath, int maxy, int maxx)
{
    pid_t pid;
    int fd;

    // Temporary directory path
    char temp_dir[PATH_MAX];
    snprintf(temp_dir, sizeof(temp_dir), "%s/.config/cfiles/preview", getenv("HOME"));

    endwin();

    // Create a child process to run "mediainfo filepath > ~/.config/cfiles/preview"
    pid = fork();
    if (pid == 0)
    {
        remove(temp_dir);
        fd = open(temp_dir, O_CREAT | O_WRONLY, 0755);
        // Redirect stdout
        dup2(fd, 1);
        if (is_regular_file(filepath) == 0)
        {
            execlp("du", "du", "-s", "-h", filepath, (char *)0);
        }
        else
        {
            execlp("mediainfo", "mediainfo", filepath, (char *)0);
        }
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        pid = fork();
        if (pid == 0)
        {
            execlp("less", "less", temp_dir, (char *)0);
            exit(1);
        }
        else
        {
            waitpid(pid, &status, 0);
        }

        refresh();
    }
}

void showHelp()
{
    const char *helpText[] = {
        // массив указателей на строковые константы
        "Help:",
        "Up/Down Arrow: Navigate",
        "Left/Right Arrow: Change Directory",
        "o: Open File",
        "d: Delete File",
        "c: Copy File",
        "m: Move File",
        "a: Select File",
        "q: Quit",
    };
    int numLines = sizeof(helpText) / sizeof(helpText[0]);
    int width = 0;

    for (int i = 0; i < numLines; ++i)
    {
        int len = strlen(helpText[i]);
        if (len > width)
        {
            width = len;
        }
    }

    width += 4; // Add padding
    int height = numLines + 2;
    int startY = (LINES - height) / 2;
    int startX = (COLS - width) / 2;

    WINDOW *help_win = newwin(height, width, startY, startX);
    box(help_win, 0, 0);

    // Центрирование заголовка "Help:"
    int titleLen = strlen(helpText[0]);
    int titlePosX = (width - titleLen) / 2;
    mvwprintw(help_win, 1, titlePosX, "%s", helpText[0]);

    for (int i = 1; i < numLines; ++i)
    {
        mvwprintw(help_win, i + 1, 2, "%s", helpText[i]);
    }

    wrefresh(help_win);
    wgetch(help_win);
    delwin(help_win);
}

int main()
{
    setlocale(LC_ALL, "");
    initializeCurses();
    char path1[PATH_MAX] = "/home/seno/left";
    char path2[PATH_MAX] = "/home/seno/right";
    getmaxyx(stdscr, maxy, maxx);

    SelectedFiles selectedFiles;
    selectedFiles.selectedFiles = malloc(MAX_SELECTED_FILES * sizeof(char *));
    selectedFiles.numSelectedFiles = 0;

    FileList fileList1, fileList2;
    fileList1.entries = malloc(sizeof(FileEntry) * 100); // Allocate memory for entries
    fileList2.entries = malloc(sizeof(FileEntry) * 100); // Allocate memory for entries
    fileList1.count = 0;
    fileList2.count = 0;
    fileList1.selected = 0;

    WINDOW *win1 = newwin(maxy - 1, maxx / 2 + 2, 1, 0);
    WINDOW *win2 = newwin(maxy - 1, maxx / 2 - 1, 1, maxx / 2 + 1);
    int activePanel = 1; // 1 for panel 1 (left), 2 for panel 2 (right)

    while (1)
    {
        populateFileList(&fileList1, path1);
        populateFileList(&fileList2, path2);

        drawFileList(win1, &fileList1, &selectedFiles);
        drawFileList(win2, &fileList2, &selectedFiles);

        const char *selectedFileName = (activePanel == 1) ? fileList1.entries[fileList1.selected].name : fileList2.entries[fileList2.selected].name;
        const char *currentPath = (activePanel == 1) ? path1 : path2;
        updateStatusBar(currentPath, selectedFileName);

        int ch = getch();

        switch (ch)
        {
        case KEY_UP:
            if (activePanel == 1)
            {
                if (fileList1.selected > 0)
                {
                    fileList1.selected--;
                    if (fileList1.selected < fileList1.topLine)
                    {
                        fileList1.topLine--;
                    }
                }
            }
            else if (activePanel == 2)
            {
                if (fileList2.selected > 0)
                {
                    fileList2.selected--;
                    if (fileList2.selected < fileList2.topLine)
                    {
                        fileList2.topLine--;
                    }
                }
            }
            break;
        case KEY_DOWN:
            if (activePanel == 1)
            {
                if (fileList1.selected < fileList1.count - 1)
                {
                    fileList1.selected++;
                    int win1Lines = maxy - 3;
                    if (fileList1.selected >= fileList1.topLine + win1Lines)
                    {
                        fileList1.topLine++;
                    }
                }
            }
            else if (activePanel == 2)
            {
                if (fileList2.selected < fileList2.count - 1)
                {
                    fileList2.selected++;
                    int win2Lines = maxy - 3;
                    if (fileList2.selected >= fileList2.topLine + win2Lines)
                    {
                        fileList2.topLine++;
                    }
                }
            }
            break;
        case KEY_LEFT:
            if (activePanel == 1)
            {
                refresh();
                if (strcmp(path1, "/") != 0)
                {
                    char *newPath = dirname(strdup(path1));
                    strncpy(path1, newPath, PATH_MAX - 1);
                    path1[PATH_MAX - 1] = '\0';
                    fileList1.selected = 0;
                    fileList1.topLine = 0;
                    free(newPath);
                }
            }
            else if (activePanel == 2)
            {
                refresh();
                if (strcmp(path2, "/") != 0)
                {
                    char *newPath = dirname(strdup(path2));
                    strncpy(path2, newPath, PATH_MAX - 1);
                    path2[PATH_MAX - 1] = '\0';
                    fileList2.selected = 0;
                    fileList2.topLine = 0;
                    free(newPath);
                }
            }
            break;
        case KEY_RIGHT:
            if (activePanel == 1)
            {
                refresh();
                if (fileList1.entries[fileList1.selected].isDir)
                {
                    strcat(path1, "/");
                    strcat(path1, fileList1.entries[fileList1.selected].name);
                }
                fileList1.selected = 0;
                fileList1.topLine = 0;
            }
            else if (activePanel == 2)
            {
                refresh();
                if (fileList2.entries[fileList2.selected].isDir)
                {
                    strcat(path2, "/");
                    strcat(path2, fileList2.entries[fileList2.selected].name);
                }
                fileList2.selected = 0;
                fileList2.topLine = 0;
            }
            break;
        case 'o':
            openFile(currentPath, selectedFileName);
            break;
        case 'd':
        {
            const char *srcPath = (activePanel == 1) ? path1 : path2;

            for (int i = 0; i < selectedFiles.numSelectedFiles; i++)
            {
                char *fileName = basename(selectedFiles.selectedFiles[i]);
                removeFile(srcPath, fileName);
            }

            // Очистка списка выбранных файлов
            for (int i = 0; i < selectedFiles.numSelectedFiles; i++)
            {
                free(selectedFiles.selectedFiles[i]);
            }
            selectedFiles.numSelectedFiles = 0;
        }
        break;
        case 'c':
            if (selectedFiles.numSelectedFiles > 0)
            {
                const char *srcPath = (activePanel == 1) ? path1 : path2;
                const char *destPath = (activePanel == 1) ? path2 : path1;

                for (int i = 0; i < selectedFiles.numSelectedFiles; i++)
                {
                    char *fileName = basename(selectedFiles.selectedFiles[i]);
                    copyFile(srcPath, destPath, fileName);
                }

                // Очистка списка выбранных файлов
                for (int i = 0; i < selectedFiles.numSelectedFiles; i++)
                {
                    free(selectedFiles.selectedFiles[i]);
                }
                selectedFiles.numSelectedFiles = 0;
            }
            refresh();
            break;
            break;
        case 'm':
            if (selectedFiles.numSelectedFiles > 0)
            {
                const char *srcPath = (activePanel == 1) ? path1 : path2;
                const char *destPath = (activePanel == 1) ? path2 : path1;

                for (int i = 0; i < selectedFiles.numSelectedFiles; i++)
                {
                    char *fileName = basename(selectedFiles.selectedFiles[i]);
                    moveFile(srcPath, destPath, fileName);
                }

                // Clear the selection list
                for (int i = 0; i < selectedFiles.numSelectedFiles; i++)
                {
                    free(selectedFiles.selectedFiles[i]);
                }
                selectedFiles.numSelectedFiles = 0;
            }
            break;
            refresh();
            break;
        case 'n':
            createDirectoryOrArchive(currentPath, &selectedFiles);
            break;
        case 'i':
        {
            const char *selectedFileName = (activePanel == 1) ? fileList1.entries[fileList1.selected].name : fileList2.entries[fileList2.selected].name;
            const char *currentPath = (activePanel == 1) ? path1 : path2;
            char fullPath[PATH_MAX];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, selectedFileName);
            getInfo(fullPath, maxy, maxx);
            initializeCurses();
        }
        break;
        case KEY_F(2): // F2 key
            renameFile(currentPath, selectedFileName);
            break;
        case 'a':
        {
            if (activePanel == 1)
            {
                // Вызываем toggleFileSelection для текущего выбранного файла на панели 1
                toggleFileSelection(&fileList1, &selectedFiles, fileList1.selected);
            }
            else if (activePanel == 2)
            {
                // Вызываем toggleFileSelection для текущего выбранного файла на панели 2
                toggleFileSelection(&fileList2, &selectedFiles, fileList2.selected);
            }
        }
        break;
        case 'x':
        {
            const char *currentPath = (activePanel == 1) ? path1 : path2;
            const char *selectedFileName = (activePanel == 1) ? fileList1.entries[fileList1.selected].name : fileList2.entries[fileList2.selected].name;
            char fullPath[PATH_MAX];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, selectedFileName);
            extractArchive(fullPath);
        }
        break;
        case '\t': // Tab key
            activePanel = (activePanel == 1) ? 2 : 1;
            break;
        case 'h':
            showHelp();
            break;
        case 'q':
            clear();
            endwin();
            exit(0);
        case KEY_RESIZE:
            handleResize(&fileList1, &fileList2, &win1, &win2);
            break;
        }
    }
    endwin();           // Завершение работы с curses
    exit(EXIT_SUCCESS); // Выход из программы
}
