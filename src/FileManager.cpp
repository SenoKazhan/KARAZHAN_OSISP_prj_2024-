#include "FileManager.h"

FileManager::FileManager() : panel1(), panel2(), isPanel1Active(true), isPanel2Active(false) {}

void FileManager::switchPanels()
{
    // Меняем активные панели местами с помощью тернарного оператора
    isPanel1Active = isPanel2Active ? true : false;
    isPanel2Active = !isPanel1Active;
}

void FileManager::processUserInput()
{
    InputHandler inputHandler;

    // Обработка пользовательского ввода
    inputHandler.handleInput();
}

void FileManager::refreshPanels()
{
    // Обновляем содержимое панелей
    panel1.refreshContent();
    panel2.refreshContent();
}

vector<File> FileManager::listFiles()
{
    vector<File> files;

    // Получаем список файлов из активной панели
    if (isPanel1Active)
    {
        files = panel1.getFiles();
    }
    else
    {
        files = panel2.getFiles();
    }

    // Сортируем список файлов по имени
    sort(files.begin(), files.end(), [](const File &a, const File &b)
         { return a.getName() < b.getName(); });

    return files;
}

void FileManager::openFile(const string& filePath) {
    // Проверяем, существует ли файл по указанному пути
    if (!boost::filesystem::exists(filePath)) {
        std::cerr << "Ошибка: Файл \"" << filePath << "\" не существует\n";
        return;
    }

    // Открываем файл
    std::ifstream file(filePath);
    
    // Проверяем, удалось ли открыть файл
    if (file.is_open()) {
        // Чтение и вывод содержимого файла на экран
        std::cout << "Содержимое файла " << filePath << ":\n";
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << '\n';
        }
        file.close();
    } else {
        // Если файл не удалось открыть, выводим сообщение об ошибке
        std::cerr << "Ошибка: Не удалось открыть файл " << filePath << '\n';
    }
}