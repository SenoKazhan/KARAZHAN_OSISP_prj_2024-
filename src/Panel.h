// Panel.h
#ifndef PANEL_H
#define PANEL_H

#include "File.h"
#include "libraries.h"

// Предварительное объявление класса FileManager
class FileManager;

class Panel
{
public:
    Panel();
    Panel(const string &path);
    Panel(FileManager& fm); // Объявление конструктора с инициализацией ссылки на FileManager

    void displayContent();
     void refreshContent();
    void navigateUp(); //!!
    void navigateToSubdirectory(const string &subdirectory); //!!
    void openSelectedFile(const string &fileName);
    
    vector<File> getFiles() const; //!!

private:
    string currentPath;
    FileManager& fileManager; // Ссылка на объект FileManager
};

#endif // PANEL_H
