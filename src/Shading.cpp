#include "Shading.h"

void DirLight::setUniforms(Shader& shader){
    shader.UseProgram();
    shader.setUniformVec3("dirlight.direction", m_direction);
    shader.setUniform1f("dirlight.size", m_size);
    shader.setUniformVec3("dirlight.intensity", m_intensity);
}

void DirLight::ImGuiControls(){
    if (ImGui::CollapsingHeader("Light")){
        ImGui::InputFloat3("direction", &m_direction[0]);
        ImGui::InputFloat3("intensity", &m_intensity[0]);
    }
}