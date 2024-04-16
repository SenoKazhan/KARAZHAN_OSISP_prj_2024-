#ifndef FILE_H
#define FILE_H

#include "libraries.h"

class File
{
private:
    string name;
    string path;
    bool isDirectory;
    int size;

public:
    File(const string &filename, const string &filepath, bool isDir, int filesize);
    void copyTo(const string &destinationPath);
    void moveTo(const string &destinationPath);
    void rename(const string &newName);
    void deleteFile();
    void createTextFile(const string &fileName);
    void createRARFile(const string &fileName);
    void createDocxFile(const string &fileName);
    void createFolder(const string &folderName);
    void createShortcut(const string &shortcutName, const string &targetPath);
    string getName() const;
};

#endif // FILE_H
