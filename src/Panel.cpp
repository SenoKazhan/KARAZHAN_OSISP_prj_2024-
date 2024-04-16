#include "Panel.h"
#include "FileManager.h"
namespace fs = boost::filesystem;

Panel::Panel() : fileManager(*new FileManager()) {} // Инициализация ссылки на FileManager в конструкторе по умолчанию

Panel::Panel(const string &path) : currentPath(path), fileManager(*new FileManager()) {} // Инициализация ссылки на FileManager в конструкторе с параметром

Panel::Panel(FileManager& fm) : fileManager(fm) {} // Инициализация ссылки на FileManager в конструкторе, принимающем ссылку на объект FileManager


void Panel::displayContent()
{
    // Открываем текущую директорию
    DIR *dir = opendir(currentPath.c_str());
    if (dir == nullptr)
    {
        cerr << "Error opening directory" << endl;
        return;
    }
    cout << "Displaying content of directory: " << currentPath << endl;
    // Читаем содержимое директории
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        // Пропускаем "." и ".."
        if (string(entry->d_name) == "." || string(entry->d_name) == "..")
        {
            continue;
        }

        // Выводим имя файла или директории
        cout << entry->d_name << endl;
    }

    // Закрываем директорию
    closedir(dir);
}

void Panel::refreshContent()
{
    // Очистить текущее отображение содержимого
    system("clear"); // Очистить консоль (для Linux)

    // Вывести обновленное содержимое панели
    displayContent();
}

void Panel::navigateUp()
{
    // Реализация перемещения вверх по директории
    cout << "Navigating up in directory: " << currentPath << endl;
}

void Panel::navigateToSubdirectory(const string &subdirectory)
{
    // Реализация перемещения в выбранную поддиректорию
    cout << "Navigating to subdirectory: " << subdirectory << endl;
}

void Panel::openSelectedFile(const string &fileName)
{
    // Реализация открытия выбранного файла
    fileManager.openFile(fileName);
}

vector<File> Panel::getFiles() const
{
    vector<File> files;



    return files;
}