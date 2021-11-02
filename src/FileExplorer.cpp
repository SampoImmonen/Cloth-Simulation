#include "FileExplorer.h"

std::string FileExplorer::openFile(){
    char filename[1024];
    FILE *f = popen("zenity --file-selection", "r");
    fgets(filename, 1024, f);
    return std::string(filename);
}

std::string FileExplorer::saveFile(){
    return std::string();
}