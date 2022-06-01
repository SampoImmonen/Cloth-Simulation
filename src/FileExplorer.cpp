#include "FileExplorer.h"

#ifdef __linux__
// Linux filexplorer
std::string FileExplorer::openFile() {
  char filename[1024];
  FILE* f = popen("zenity --file-selection", "r");
  fgets(filename, 1024, f);
  return std::string(filename);
}

std::string FileExplorer::saveFile() {
  return std::string();
}

#elif _WIN32
// Windows version of file explorer
std::string FileExplorer::openFile() {
  // not implemented
  return std::string(filename);
}

std::string FileExplorer::saveFile() {
  // not implemented
  return std::string();
}

#endif
