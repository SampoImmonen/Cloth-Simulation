#include "Shading.h"

DirLight::DirLight(const glm::vec3& dir): m_direction(dir){
    m_shadowMap = ShadowMapBuffer(2048 , 2048);
}

void DirLight::setUniforms(Shader& shader){
    shader.UseProgram();
    shader.setUniformVec3("dirlight.direction", m_direction);
    shader.setUniform1f("dirlight.size", m_size);
    shader.setUniformVec3("dirlight.intensity", m_intensity);
    shader.setUniformInt("dirlight.shadows", m_shadows);
    shader.setUniformInt("dirlight.shadowMap", 1);
    m_shadowMap.bindDepthTexture(1);
}

void DirLight::ImGuiControls(){
    if (ImGui::CollapsingHeader("Light")){
        ImGui::InputFloat3("direction", &m_direction[0]);
        ImGui::InputFloat3("intensity", &m_intensity[0]);
        ImGui::Checkbox("shadows", &m_shadows);
        ImGui::InputFloat("light size", &m_size);
    }
}



glm::mat4 DirLight::getLightSpaceMatrix(){
    float near_plane = 0.1f, far_plane = 100.0f;
    glm::mat4 projection = glm::ortho<float>(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    glm::mat4 view = glm::lookAt(10.0f*glm::normalize(m_direction), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return projection*view;
} 

void DirLight::prepareShadowMap(Shader& shader){
    shader.UseProgram();
    shader.setUniformMat4f("lightSpaceMatrix", getLightSpaceMatrix());
    m_shadowMap.bind();
}
