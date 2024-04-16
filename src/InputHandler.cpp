#include "InputHandler.h"
#include "FileManager.h"

void InputHandler::handleInput()
{
    string command;
    cout << "Enter a command: ";
    cin >> command;

    processCommand(command); // Передаем введенную команду на обработку
}

void InputHandler::processCommand(const string &command)
{
    FileManager fileManager; // Создаем экземпляр FileManager для работы с панелями и файлами

    if (command == "help")
    {
        displayHelp();
    }
    else if (command == "list")
    {
        listFiles();
    }
    else if (command == "switch")
    {
        switchPanels();
    }
    else if (command == "refresh")
    {
        refreshPanels();
    }
    else
    {
        cout << "Unknown command. Type 'help' for available commands." << endl;
    }
}

void InputHandler::displayHelp()
{
    cout << "Available commands:" << endl;
    cout << "  help    - Display available commands" << endl;
    cout << "  list    - List files in the current directory" << endl;
    cout << "  switch  - Switch between panels" << endl;
    cout << "  refresh - Refresh content of panels" << endl;
}

void InputHandler::listFiles()
{
    FileManager fileManager; 
    fileManager.listFiles(); // Вызываем метод списка файлов из FileManager
}

void InputHandler::switchPanels()
{
    FileManager fileManager;    
    fileManager.switchPanels(); // Вызываем метод смены панелей из FileManager
}

void InputHandler::refreshPanels()
{
    FileManager fileManager;     
    fileManager.refreshPanels(); // Вызываем метод обновления панелей из FileManager
}
