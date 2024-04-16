#include "File.h"

namespace fs = boost::filesystem;

File::File(const string &filename, const string &filepath, bool isDir, int filesize)
{
    name = filename;
    path = filepath;
    isDirectory = isDir;
    size = filesize;
}

void File::copyTo(const string &destinationPath)
{
    if (isDirectory)
    {
        cerr << "Cannot copy a directory directly." << endl;
        return;
    }

    try
    {
        fs::copy_file(path, destinationPath + "/" + name, fs::copy_options::overwrite_existing);
        cout << "File " << name << " copied to " << destinationPath << endl;
    }
    catch (const fs::filesystem_error &e)
    {
        cerr << "Failed to copy file: " << e.what() << endl;
    }
}

void File::moveTo(const string &destinationPath)
{
    if (isDirectory)
    {
        cerr << "Cannot move a directory directly." << endl;
        return;
    }

    try
    {
        fs::rename(path, destinationPath + "/" + name);
        cout << "File " << name << " moved to " << destinationPath << endl;
    }
    catch (const fs::filesystem_error &e)
    {
        cerr << "Failed to move file: " << e.what() << endl;
    }
}

void File::rename(const string &newName)
{
    try
    {
        fs::path newPath = fs::path(path).parent_path() / newName;
        fs::rename(path, newPath);
        name = newName;
        path = newPath.string();
        cout << "File renamed to " << newName << endl;
    }
    catch (const fs::filesystem_error &e)
    {
        cerr << "Failed to rename file: " << e.what() << endl;
    }
}

void File::deleteFile()
{
    try
    {
        fs::remove(path);
        cout << "File " << name << " deleted." << endl;
    }
    catch (const fs::filesystem_error &e)
    {
        cerr << "Failed to delete file: " << e.what() << endl;
    }
}

void File::createTextFile(const string &fileName)
{
    ofstream file(fileName);
    if (file.is_open())
    {
        // Файл успешно создан
        file.close();
    }
    else
    {
        // Ошибка при создании файла
    }
}

void File::createRARFile(const string &fileName)
{
    // Проверяем существование файла или директории
    if (!fs::exists(path + "/" + fileName))
    {
        cerr << "Ошибка: файл или директория \"" << fileName << "\" не существует.\n";
        return;
    }

    // Формируем команду для создания RAR архива
    string command = "rar a \"" + name + ".rar\" \"" + path + "/" + fileName + "\"";

    // Запускаем команду в системе
    int result = system(command.c_str());

    // Проверяем результат выполнения команды
    if (result == 0)
    {
        cout << "RAR архив успешно создан: " << name << ".rar\n";
    }
    else
    {
        cerr << "Ошибка: не удалось создать RAR архив.\n";
    }
}

void File::createDocxFile(const string &fileName)
{
    // Реализация создания DOCX файла
    ofstream file(fileName + ".docx");
    if (file)
    {
        // Дополнительные действия при создании файла, если нужно
        cout << "DOCX файл \"" << fileName << ".docx\" успешно создан.\n";
    }
    else
    {
        cerr << "Ошибка при создании DOCX файла \"" << fileName << ".docx\"\n";
    }
}

void File::createFolder(const string &folderName)
{
    // Реализация создания директории
    if (mkdir(folderName.c_str(), 0777) == 0)
    {
        cout << "Директория \"" << folderName << "\" успешно создана.\n";
    }
    else
    {
        cerr << "Ошибка при создании директории \"" << folderName << "\"\n";
    }
}

void File::createShortcut(const string &shortcutName, const string &targetPath)
{
    // Реализация создания ярлыка
    if (symlink(targetPath.c_str(), (shortcutName + ".lnk").c_str()) == 0)
    {
        cout << "Ярлык \"" << shortcutName << "\" успешно создан.\n";
    }
    else
    {
        cerr << "Ошибка при создании ярлыка \"" << shortcutName << "\"\n";
    }
}

string File::getName() const
{
    return name;
}
