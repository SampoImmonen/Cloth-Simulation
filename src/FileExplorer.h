#pragma once

#include <stdio.h>
#include <iostream>
#include <string>

class FileExplorer {
 public:
  static std::string openFile();
  static std::string saveFile();
};