#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <filesystem>

#include "Buffers.h"
#include "Shader.h"
#include "Camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct FpsInfo {
	float deltatime = 0;
	float lastFrame = 0;
	float lastTime = 0.0f;
	float fps = 0.0f;
	int nbFrames = 0;

	float update() {
		float currentFrame = glfwGetTime();
		deltatime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		nbFrames++;
		if (currentFrame - lastTime >= 1.0f) {
			lastTime += 1.0f;
			fps = 1000.0 / double(nbFrames);
			nbFrames = 0;
		}
		return fps;
	}
};

struct MouseInfo {
	bool firstMouse = true;
	//window coordinates
	float lastX, lastY;
	//imGui scene viewport coordinates
	float ImlastX, ImlastY;
};

class Application{
public:
    Application();
    ~Application();
    void run();
private:
    void init();
    void render();
    void update();
    void terminate();

    void initBuffers();
    //first triangle ids
    //uint32_t m_vao, m_vbo, m_ebo;


    uint32_t m_readBuf = 0;
    //cloth buffer ids
    uint32_t m_posbufs[2];
    uint32_t m_velbufs[2];
    uint32_t m_nornBuf, m_elBuf, m_tcBuf;
    uint32_t m_clothVao;
    uint32_t m_numelements;
    //cloth attributes
    glm::vec2 m_clothsize = glm::vec2(4.0f, 3.0f);
    glm::ivec2 m_numParticles = glm::ivec2(40, 40);

    //window
    GLFWwindow* m_window = nullptr;
    uint32_t m_width = 800, m_height=600;

    VertexArray m_vao;
    std::vector<Shader> m_shaders;
    std::vector<Shader> m_computeShaders;

    Camera m_camera;

    FpsInfo m_fpsinfo;
    MouseInfo m_mouseinfo;

    //physics attributes
    float m_stiffness = 0.0001;
    float m_dampingConstant = 0.0001;

    void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow* window);

    //hack for GLFW callbacks
    class GLFWCallbackWrapper {
	public:
		GLFWCallbackWrapper() = delete;
		GLFWCallbackWrapper(const GLFWCallbackWrapper&) = delete;
		GLFWCallbackWrapper(GLFWCallbackWrapper&&) = delete;
		~GLFWCallbackWrapper() = delete;

		static void MouseCallback(GLFWwindow* window, double positionX, double positionY);
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		static void SetApplication(Application *application);

	private:
		static Application* s_application;
	};

};