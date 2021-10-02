#include "Application.h"


#define PRIM_RESTART 0xffffff

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
    {glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(1.0f)},
    {glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
    {glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.3f, 0.7f)},
    {glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.3f, 0.7f)},
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
    Shader clothRender(workindir+"ClothRender.vert", workindir+"ClothRender.frag");
    m_shaders.push_back(clothRender);

    initBuffers();
}

void Application::update(){

    //update positions and velocities


}

void Application::initBuffers(){

    //create transformations
    glm::mat4 transf = glm::translate(glm::mat4(1.0), glm::vec3(0,m_clothsize.y,0));
    transf = glm::rotate(transf, glm::radians(-80.0f), glm::vec3(1,0,0));
    transf = glm::translate(transf, glm::vec3(0,-m_clothsize.y,0));
    
    //create data vectors
    std::vector<glm::vec4> initial_positions;
    std::vector<glm::vec4> initial_velocities(m_numParticles.x*m_numParticles.y*4, glm::vec4(0.0f));

    float dx = m_clothsize.x/(m_numParticles.x-1);
    float dy = m_clothsize.y/(m_numParticles.y-1);
    
    glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < m_numParticles.x ; ++i){
        for (int j = 0; j < m_numParticles.y; ++j){
            
            p.x = j*dx;
            p.y = i*dy;
            p.z = 0.0f;
            p = transf*p;
            p.w = 1.0f;
            initial_positions.push_back(p);

        }
    }

    // Every row is one triangle strip
    std::vector<uint32_t> indices;
    for (int row = 0; row < m_numParticles.y-1 ; ++row){
        for (int col = 0; col < m_numParticles.x; ++col){
            indices.push_back((row+1)*m_numParticles.x+(col ));
            indices.push_back((row )*m_numParticles.x+(col ));
        }
        indices.push_back(PRIM_RESTART);
    }

    //create storage buffers
    uint32_t buffers[7];
    glGenBuffers(7, buffers);
    m_posbufs[0] = buffers[0];
    m_posbufs[1] = buffers[1];
    m_velbufs[0] = buffers[2];
    m_velbufs[1] = buffers[3];
    m_elBuf = buffers[4];


    uint32_t numParticles = m_numParticles.x*m_numParticles.y;

    //position buffers
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posbufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(glm::vec4), &initial_positions[0], GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_posbufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

    //velocity buffers
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velbufs[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(glm::vec4), &initial_velocities[0], GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velbufs[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);

    //index buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_elBuf);
    glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_DYNAMIC_COPY);

    m_numelements = static_cast<uint32_t>(indices.size());
    //create vertex array

    glGenVertexArrays(1, &m_clothVao);
    glBindVertexArray(m_clothVao);

    glBindBuffer(GL_ARRAY_BUFFER, m_posbufs[0]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elBuf);
    glBindVertexArray(0);
}

void Application::render(){

    processInput(m_window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), (float)m_width / m_height, 0.1f, 100.0f);


    //render cloth

    m_shaders[1].UseProgram();
    m_shaders[0].setUniformMat4f("view", view);
    m_shaders[0].setUniformMat4f("projection", projection);
    glBindVertexArray(m_clothVao);
    glDrawArrays(GL_POINTS, 0, m_numParticles.x*m_numParticles.y);
    //glDrawElements(GL_TRIANGLE_STRIP, m_numelements, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    

    m_shaders[0].UseProgram();
    m_shaders[0].setUniformMat4f("view", view);
    m_shaders[0].setUniformMat4f("projection", projection);
    m_vao.drawElements();
    //glBindVertexArray(0);
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