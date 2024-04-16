#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Panel.h"
#include "InputHandler.h"
#include "File.h"

class FileManager {
private:
    Panel panel1;
    Panel panel2;
    bool isPanel1Active;
    bool isPanel2Active;

public:
    FileManager();
    void switchPanels();
    void processUserInput();
    void refreshPanels();
    void openFile(const string& filePath);
    vector<File> listFiles();
};

#endif // FILEMANAGER_H
