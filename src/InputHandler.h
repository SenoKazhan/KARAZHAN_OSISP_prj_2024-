#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "libraries.h"
#include <string>

class InputHandler {
public:
    void handleInput();
    void processCommand(const string& command);
    void displayHelp();
    void listFiles();
    void switchPanels();
    void refreshPanels();
};

#endif // INPUTHANDLER_H
