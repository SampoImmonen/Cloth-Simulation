#pragma once

#include <glad/glad.h> 

#include <string>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:

	Shader(const std::string& vpath, const std::string& fpath);
	Shader(const std::string& vpath, const std::string& fpath, const std::string& gpath);
	Shader(const std::string& cpath);

	~Shader();
	
	void UseProgram();
	void compileShaders(void);

	void setUniform1f(const char* name, float v0);
	void setUniform4f(const char* name, float v0, float v1, float v2, float v3);
	

	void setUniformInt(const char* name, int value);
	void setUniformMat4f(const char* name, const glm::mat4& mat);
	void setUniformVec3(const char* name, const glm::vec3& vec);

private:
	std::string readfromFile(const std::string &path);
	std::string vertexPath = "";
	std::string fragmentPath = "";
	std::string geometryPath= "";
	std::string computePath = "";

	unsigned int VertexShader;
	unsigned int FragmentShader;
	unsigned int GeometryShader;
	unsigned int ComputeShader;

	unsigned int program;
};
