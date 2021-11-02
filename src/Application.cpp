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
    {glm::vec3(-5.0f, -1.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
    {glm::vec3(-5.0f, -1.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
    {glm::vec3(5.0f, -1.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
    {glm::vec3(5.0f, -1.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)}
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
    auto texturedir = basepath.parent_path().string()+"/textures/";
    auto workindir = basepath.parent_path().string()+"/src/";
    //std::cout << workindir << "\n";
    Shader shader(workindir+"basicshader.vert", workindir+"basicshader.frag");
    m_shaders.push_back(shader);
    Shader clothRender(workindir+"ClothRender.vert", workindir+"ClothRender.frag");
    m_shaders.push_back(clothRender);
    Shader ShadowMapShader(workindir+"ShadowDepthShader.vert", workindir+"ShadowDepthShader.frag");
    m_shaders.push_back(ShadowMapShader);
    Shader NormalVisualizationShader(workindir+"NormalShader.vert", workindir+"NormalShader.frag", workindir+"NormalShader.geo");
    m_shaders.push_back(NormalVisualizationShader);
    
    //compute shaders
    Shader ClothPositionShader(workindir+"ClothCompute.comp");
    m_computeShaders.push_back(ClothPositionShader);
    Shader ClothNormalShader(workindir+"ClothNormals.comp");
    m_computeShaders.push_back(ClothNormalShader);
    
    //compute shaders for trapezoid integrator
    Shader TrapezoidShaderPart1(workindir+"trapezoid_step1.comp");
    m_trapezoidShaders.push_back(TrapezoidShaderPart1);
    Shader TrapezoidShaderPart2(workindir+"trapezoid_step2.comp");
    m_trapezoidShaders.push_back(TrapezoidShaderPart2);
    Shader TrapezoidShaderPart3(workindir+"trapezoid_step3.comp");
    m_trapezoidShaders.push_back(TrapezoidShaderPart3);

    //init buffers
    initBuffers();

    float dx = m_clothsize.x/(m_numParticles.x-1);
    float dy = m_clothsize.y/(m_numParticles.y-1);
    m_computeShaders[0].setUniform1f("RestLengthHoriz", dx);
    m_computeShaders[0].setUniform1f("RestLengthVert", dy);
    m_computeShaders[0].setUniform1f("RestLengthDiag", sqrt(dx*dx+dy*dy));

    for (uint32_t i = 0; i < 3; ++i){
        m_trapezoidShaders[i].setUniform1f("RestLengthHoriz", dx);
        m_trapezoidShaders[i].setUniform1f("RestLengthVert", dy);
        m_trapezoidShaders[i].setUniform1f("RestLengthDiag", sqrt(dx*dx+dy*dy));
    }

    glPointSize(2.0f);

    m_light = DirLight(glm::vec3(1.0f));

    m_clothMaterial.m_texture = std::make_shared<Texture2D>(Texture2D(texturedir+"texture.jpg"));
    //m_clothtexture = Texture2D(texturedir+"texture.jpg");
    m_planeMaterial.m_texture = std::make_shared<Texture2D>(Texture2D(texturedir+"brickwall.jpg"));
    m_planeMaterial.m_normalMap = std::make_shared<Texture2D>(Texture2D(texturedir+"brickwall_normal.jpg"));
    //imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
    ImGui::StyleColorsDark();
}

void Application::trapezoidIntegrationUpdate(){
    //update uniforms
    for (uint32_t i = 0; i < 2; ++i){
        m_trapezoidShaders[i].setUniform1f("SpringK", m_stiffness);
        m_trapezoidShaders[i].setUniform1f("DampingConst", m_dampingConstant);
        m_trapezoidShaders[i].setUniformVec3("Gravity", m_gravity);
        m_trapezoidShaders[i].setUniformInt("hasWind", m_hasWind);
        m_trapezoidShaders[i].setUniformInt("hasShear", m_hasShear);
        m_trapezoidShaders[i].setUniformInt("hasFlex", m_hasFlex);
    }

    for (uint32_t i = 0; i < 500; ++i){
        m_trapezoidShaders[0].UseProgram();
        glDispatchCompute(m_numParticles.x/10, m_numParticles.y/10, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        m_trapezoidShaders[1].UseProgram();
        glDispatchCompute(m_numParticles.x/10, m_numParticles.y/10, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        m_trapezoidShaders[2].UseProgram();
        glDispatchCompute(m_numParticles.x/10, m_numParticles.y/10, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

}

void Application::update(){

    if (m_useTrapezoid){
        trapezoidIntegrationUpdate();
    }
    else {
    //set compute shader uniforms
        m_computeShaders[0].UseProgram();

        //update controllable uniforms
        m_computeShaders[0].setUniform1f("SpringK", m_stiffness);
        m_computeShaders[0].setUniform1f("DampingConst", m_dampingConstant);
        m_computeShaders[0].setUniformVec3("Gravity", m_gravity);
        m_computeShaders[0].setUniformInt("hasWind", m_hasWind);
        m_computeShaders[0].setUniformInt("hasShear", m_hasShear);
        m_computeShaders[0].setUniformInt("hasFlex", m_hasFlex);

        for (int i = 0; i<2000; ++i){
            glDispatchCompute(m_numParticles.x/10, m_numParticles.y/10, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            m_readBuf =  1 - m_readBuf;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posbufs[m_readBuf]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_posbufs[1-m_readBuf]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velbufs[m_readBuf]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velbufs[1-m_readBuf]);

        }
    }
    //update normals
    m_computeShaders[1].UseProgram();
    glDispatchCompute(m_numParticles.x/10, m_numParticles.y/10, 1);
    glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
}

void Application::initBuffers(){

    //create transformations
    glm::mat4 transf = glm::translate(glm::mat4(1.0), glm::vec3(0,m_clothsize.y,0));
    transf = glm::rotate(transf, glm::radians(-80.0f), glm::vec3(1,0,0));
    transf = glm::translate(transf, glm::vec3(0,-m_clothsize.y,0));
    
    //create data vectors
    std::vector<glm::vec4> initial_positions;
    std::vector<glm::vec4> initial_velocities(m_numParticles.x*m_numParticles.y, glm::vec4(0.0f));
    std::vector<glm::vec2> tx_coordinates;
    float dx = m_clothsize.x/(m_numParticles.x-1);
    float dy = m_clothsize.y/(m_numParticles.y-1);
    float du = 1.0f/(m_numParticles.x-1);
    float dv = 1.0f/(m_numParticles.y-1);

    glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < m_numParticles.y ; ++i){
        for (int j = 0; j < m_numParticles.x; ++j){
            
            p.x = j*dx;
            p.y = i*dy;
            p.z = 0.0f;
            p = transf*p;
            p.w = 1.0f;
            initial_positions.push_back(p);

            tx_coordinates.push_back(glm::vec2(du*j, dv*i));
        }
    }

    //create indices for drawing
    //square in the grid is composed of two triangles
    std::vector<uint32_t> indices;
    for (int row = 0; row < m_numParticles.y-1; ++row){
        for (int col = 0; col < m_numParticles.x-1; ++col){
                        
            uint32_t row1 = row*m_numParticles.x;
            uint32_t row2 = (row+1)*m_numParticles.x;
            
            //triangle 1
            indices.push_back(row1+col);
            indices.push_back(row1+col+1);
            indices.push_back(row2+col+1);

            //triangle 2
            indices.push_back(row1+col);
            indices.push_back(row2+col+1);
            indices.push_back(row2+col);
        }
    }

    //create storage buffers
    uint32_t buffers[9];
    glGenBuffers(9, buffers);
    m_posbufs[0] = buffers[0];
    m_posbufs[1] = buffers[1];
    m_velbufs[0] = buffers[2];
    m_velbufs[1] = buffers[3];
    m_elBuf = buffers[4];
    m_normBuf = buffers[5];
    m_tcBuf = buffers[6];
    m_kBuffers[0] = buffers[7];
    m_kBuffers[1] = buffers[8];

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

    //normal buffers
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_normBuf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

    //slope buffers (acceleration in our case)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_kBuffers[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_kBuffers[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles*sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);


    //index buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_elBuf);
    glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_DYNAMIC_COPY);

    glBindBuffer(GL_ARRAY_BUFFER, m_tcBuf);
    glBufferData(GL_ARRAY_BUFFER, tx_coordinates.size() * sizeof(glm::vec2), &tx_coordinates[0], GL_STATIC_DRAW);

    m_numelements = indices.size();
    std::cout << "Num of Elements: " << m_numelements << "\n";
    //create vertex array

    glGenVertexArrays(1, &m_clothVao);
    glBindVertexArray(m_clothVao);

    glBindBuffer(GL_ARRAY_BUFFER, m_posbufs[0]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_normBuf);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, m_tcBuf);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elBuf);
    glBindVertexArray(0);
}

void Application::render(){

    updateShadowMap();
    glViewport(0, 0, m_width, m_height);
    m_light.setUniforms(m_shaders[1]);
    m_shaders[1].setUniformVec3("viewPos", m_camera.Position);

    processInput(m_window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glm::mat4 view = m_camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), (float)m_width / m_height, 0.1f, 100.0f);

    //render cloth
    m_shaders[1].UseProgram();
    setMaterialUniforms(m_clothMaterial);
    m_shaders[1].setUniformMat4f("view", view);
    m_shaders[1].setUniformMat4f("projection", projection);
    m_shaders[1].setUniformMat4f("model", glm::mat4(1.0f));
    m_shaders[1].setUniformInt("Tex", 0);
    m_clothtexture.bind(0);
    glBindVertexArray(m_clothVao);
    
    if (m_renderpoints){
        m_shaders[1].setUniformVec3("colordebug", glm::vec3(0.3f, 1.0f, 0.0f));
        glDrawArrays(GL_POINTS, 0, m_numParticles.x*m_numParticles.y);
    }
    else{
        m_shaders[1].setUniformVec3("colordebug", glm::vec3(0.0f, 0.0f, 0.0f));
        glDrawElements(GL_TRIANGLES, m_numelements, GL_UNSIGNED_INT, 0);
    }
    
    if (m_renderNormals){
        m_shaders[3].UseProgram();
        m_shaders[3].setUniformMat4f("model", glm::mat4(1.0f));
        m_shaders[3].setUniformMat4f("view", view);
        m_shaders[3].setUniformMat4f("projection", projection);

        glBindVertexArray(m_clothVao);
        glDrawElements(GL_TRIANGLES, m_numelements, GL_UNSIGNED_INT, 0);
    }

    m_shaders[1].setUniformMat4f("lightSpaceMatrix", m_light.getLightSpaceMatrix());
    setMaterialUniforms(m_planeMaterial);
    m_vao.drawElements();
    //glBindVertexArray(0);

    //render imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Demo window");
    ImGui::InputFloat("stiffness", &m_stiffness);
    ImGui::InputFloat("flow resistance", &m_dampingConstant);
    ImGui::InputFloat3("Gravity", &m_gravity[0]);
    ImGui::Checkbox("render points", &m_renderpoints);
    ImGui::Checkbox("render normals", &m_renderNormals);
    ImGui::Checkbox("Shear Springs", &m_hasShear);
    ImGui::Checkbox("Flex Springs", &m_hasFlex);
    ImGui::Checkbox("use Trapezoid", &m_useTrapezoid);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    m_light.ImGuiControls();
    clothMaterialUI();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

void Application::reloadShader(){
    auto basepath = std::filesystem::current_path();
    auto texturedir = basepath.parent_path().string()+"/textures/";
    auto workindir = basepath.parent_path().string()+"/src/";
    //std::cout << workindir << "\n";
    Shader clothRender(workindir+"ClothRender.vert", workindir+"ClothRender.frag");
    m_shaders[1] = clothRender;
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
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		m_hasWind = true;
	}
    if (glfwGetKey(window, GLFW_KEY_F) != GLFW_PRESS) {
		m_hasWind = false;
	}
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		reloadShader();
	}
    
}

void Application::updateShadowMap(){
    glEnable(GL_DEPTH_TEST);
    m_shaders[2].UseProgram();
    m_light.prepareShadowMap(m_shaders[2]);
    m_shaders[2].setUniformMat4f("model", glm::mat4(1.0f));
    glBindVertexArray(m_clothVao);
    glDrawElements(GL_TRIANGLES, m_numelements, GL_UNSIGNED_INT, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::setMaterialUniforms(const Material& material){
    m_shaders[1].setUniformVec3("material.albedo", material.albedo);
    m_shaders[1].setUniform1f("material.metallic", material.metallic);
    m_shaders[1].setUniform1f("material.roughness", material.roughness);
    m_shaders[1].setUniform1f("material.ao", material.ao);

    bool hasTexture = (material.m_texture != nullptr);
    //currently only supports albedo textures
    m_shaders[1].setUniformInt("material.hasAlbedo", hasTexture);
    if (hasTexture){
        m_shaders[1].setUniformInt("material.albedoMap", 0);
        //m_clothtexture.bind(0)
        material.m_texture->bind(0);
    }
    bool hasNormalMap = (material.m_normalMap!=nullptr);
    m_shaders[1].setUniformInt("material.hasNormal", hasNormalMap);
    if (hasNormalMap)
    {
        m_shaders[1].setUniformInt("material.normalMap", 2);
        material.m_normalMap->bind(2);
    }
    bool hasRoughnessMap = (material.m_roughnessMap!=nullptr);
    m_shaders[1].setUniformInt("material.hasRoughness", hasRoughnessMap);
    if (hasRoughnessMap){
        m_shaders[1].setUniformInt("material.roughnessMap", 3);
        //m_clothtexture.bind(0)
        material.m_roughnessMap->bind(3);
    }

}

void Application::clothMaterialUI(){
    if (ImGui::CollapsingHeader("Cloth Material")){
        ImGui::InputFloat3("cloth albedo", &m_clothMaterial.albedo[0]);
        ImGui::InputFloat("cloth metallic", &m_clothMaterial.metallic);
        ImGui::InputFloat("cloth roughness", &m_clothMaterial.roughness);
        ImGui::InputFloat("cloth ao", &m_clothMaterial.ao);
    }

    if (ImGui::CollapsingHeader("Plane Material")){
        ImGui::InputFloat3("plane albedo", &m_planeMaterial.albedo[0]);
        unsigned int albedoID = m_planeMaterial.m_texture->getTextureID();
        if (ImGui::ImageButton((void*)(intptr_t)albedoID, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0))){
            std::string albedoName = FileExplorer::openFile();
            albedoName.pop_back();
            m_planeMaterial.m_texture = std::make_shared<Texture2D>(Texture2D(albedoName));
        }
        unsigned int normalID = m_planeMaterial.m_normalMap->getTextureID();
        if (ImGui::ImageButton((void*)(intptr_t)normalID, ImVec2(50, 50), ImVec2(0, 1), ImVec2(1, 0))){
            std::cout << "change normal\n";
        }
        ImGui::InputFloat("plane metallic", &m_planeMaterial.metallic);
        ImGui::InputFloat("plane roughness", &m_planeMaterial.roughness);
        ImGui::InputFloat("plane ao", &m_planeMaterial.ao);
    }
}


void Application::terminate(){
    glfwTerminate();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}