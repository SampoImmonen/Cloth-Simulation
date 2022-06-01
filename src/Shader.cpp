
#include "Shader.h"

Shader::Shader(const std::string& vpath, const std::string& fpath) {
  vertexPath = vpath;
  fragmentPath = fpath;
  compileShaders();
}

Shader::Shader(const std::string& vpath, const std::string& fpath, const std::string& gpath) {
  vertexPath = vpath;
  fragmentPath = fpath;
  geometryPath = gpath;
  compileShaders();
}

Shader::Shader(const std::string& cpath) {
  computePath = cpath;

  std::string csource = readfromFile(computePath);
  const char* csourcep = csource.c_str();
  std::cout << "\n---------------------------------\n";
  std::cout << "compiling shader:" << computePath << "\n";
  ComputeShader = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(ComputeShader, 1, &csourcep, NULL);
  glCompileShader(ComputeShader);
  int success;
  char infoLog[512];
  glGetShaderiv(ComputeShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(ComputeShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  program = glCreateProgram();
  glAttachShader(program, ComputeShader);

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  glDeleteShader(ComputeShader);
}

Shader::~Shader() {
  // glDeleteProgram(program);
}

void Shader::UseProgram() {
  glUseProgram(program);
}

void Shader::compileShaders(void) {
  std::string vsource = readfromFile(vertexPath);
  std::string fsource = readfromFile(fragmentPath);

  const char* vsourcep = vsource.c_str();
  const char* fsourcep = fsource.c_str();


  std::cout << "\n---------------------------------\n";
  std::cout << "compiling shader:" << vertexPath << "\n";
  VertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(VertexShader, 1, &vsourcep, NULL);
  glCompileShader(VertexShader);

  int success;
  char infoLog[512];
  glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(VertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  std::cout << "compiling shader:" << fragmentPath << "\n";
  FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(FragmentShader, 1, &fsourcep, NULL);
  glCompileShader(FragmentShader);

  glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  if (!geometryPath.empty()) {
    std::string gsource = readfromFile(geometryPath);
    const char* gsourcep = gsource.c_str();
    std::cout << "compiling shader:" << geometryPath << "\n";
    GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(GeometryShader, 1, &gsourcep, NULL);
    glCompileShader(GeometryShader);

    glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(GeometryShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
  }

  program = glCreateProgram();
  glAttachShader(program, VertexShader);
  glAttachShader(program, FragmentShader);
  if (!geometryPath.empty()) {
    glAttachShader(program, GeometryShader);
  }
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);
  if (!geometryPath.empty()) {
    glDeleteShader(GeometryShader);
  }
  // std::cout << program << "\n";
}

void Shader::setUniform1f(const char* name, float v0) {
  UseProgram();
  int loc = glad_glGetUniformLocation(program, name);
  glUniform1f(loc, v0);
}

void Shader::setUniform4f(const char* name, float v0, float v1, float v2, float v3) {
  UseProgram();
  int loc = glad_glGetUniformLocation(program, name);
  glUniform4f(loc, v0, v1, v2, v3);
}


void Shader::setUniformInt(const char* name, int value) {
  int loc = glad_glGetUniformLocation(program, name);
  glUniform1i(loc, value);
}

void Shader::setUniformMat4f(const char* name, const glm::mat4& mat) {
  unsigned int transformLoc = glGetUniformLocation(program, name);
  glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setUniformVec3(const char* name, const glm::vec3& vec) {
  unsigned int loc = glGetUniformLocation(program, name);
  glUniform3f(loc, vec.x, vec.y, vec.z);
}

std::string Shader::readfromFile(const std::string& path) {
  std::ifstream ifs(path.c_str());
  std::string str(std::istreambuf_iterator<char>{ifs}, {});

  // std::cout << source;
  return str;
}