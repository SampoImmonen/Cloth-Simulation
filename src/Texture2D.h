#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>
#include "stb_image.h"


class Texture2D {
 public:
  Texture2D(const std::string& path);
  Texture2D();
  ~Texture2D();

  unsigned int getTextureID() const {
    return texture;
  }
  void bind(int textureunit = 0);
  void setTexParameteri(GLenum target, GLenum pname, GLint param);


 private:
  int width, height, nrChannels;
  unsigned int texture;
};