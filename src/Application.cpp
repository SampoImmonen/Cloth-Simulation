#include "Application.h"



//GLFW wrapper function hack
void Application::GLFWCallbackWrapper::MouseCallback(GLFWwindow* window, double positionX, double positionY)
{
	s_application->mouseCallback(window, positionX, positionY);
}

void Application::GLFWCallbackWrapper::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	s_application->framebufferSizeCallback(window, width, height);
}

void Application::GLFWCallbackWrapper::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	s_application->scrollCallback(window, xoffset, yoffset);
}

void Application::GLFWCallbackWrapper::SetApplication(Application* application)
{
	GLFWCallbackWrapper::s_application = application;
}

Application* Application::GLFWCallbackWrapper::s_application = nullptr;


const std::vector<Vertex> tri_vertices = {
    {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f)},
    {glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
    {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.3f, 0.7f)},
    {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.3f, 0.7f)},
};

const std::vector<uint32_t> tri_indices = {
    0,1,2,
    1,2,3
};

Application::Application() {
    init();
}

Application::~Application(){
    terminate();
}

void Application::init(){
    //init glfw
    std::cout << "Initializing GLFW\n";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(m_width, m_height, "Compute Shader Cloth", NULL, NULL);
    
    if (m_window==NULL){
        throw std::runtime_error("Failed to create GLFW window\n");
    }

    glfwMakeContextCurrent(m_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		throw std::runtime_error("Failed to initialize glad\n");
	}

    glViewport(0, 0, m_width, m_height);
    GLFWCallbackWrapper::SetApplication(this);
	glfwSetFramebufferSizeCallback(m_window, GLFWCallbackWrapper::FramebufferResizeCallback);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MouseCallback);
	glfwSetScrollCallback(m_window, GLFWCallbackWrapper::scrollCallback);
    
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR);
    VertexBuffer vbo(tri_vertices);
    IndexBuffer ebo(tri_indices);
    m_vao = VertexArray(vbo, ebo);

    auto basepath = std::filesystem::current_path();
    auto workindir = basepath.parent_path().string()+"/src/";
    //std::cout << workindir << "\n";
    Shader shader(workindir+"basicshader.vert", workindir+"basicshader.frag");
    m_shaders.push_back(shader);
    //std::cout << "moimoi\n";
}

void Application::update(){

}

void Application::render(){

    processInput(m_window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glm::mat4 view = m_camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), (float)m_width / m_height, 0.1f, 100.0f);
    m_shaders[0].setUniformMat4f("view", view);
    m_shaders[0].setUniformMat4f("projection", projection);
    m_shaders[0].UseProgram();
    m_vao.drawElements();
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Application::run(){
    std::cout << "starting main loop\n";
	while (!glfwWindowShouldClose(m_window)) {
        m_fpsinfo.update();
		update();
		render();
	}
}

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (m_mouseinfo.firstMouse) {
		m_mouseinfo.lastX = xpos;
		m_mouseinfo.lastY = ypos;
		m_mouseinfo.firstMouse = false;
	}

	m_camera.ProcessMouseMovement(xpos - m_mouseinfo.lastX, m_mouseinfo.lastY - ypos);
	m_mouseinfo.lastX = xpos;
	m_mouseinfo.lastY = ypos;
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	m_camera.ProcessMouseScroll(yoffset);
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}



void Application::processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		m_camera.ProcessKeyboard(FORWARD, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		m_camera.ProcessKeyboard(LEFT, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		m_camera.ProcessKeyboard(RIGHT, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		m_camera.ProcessKeyboard(BACKWARD, m_fpsinfo.deltatime);
	}
}


void Application::terminate(){
    glfwTerminate();
}