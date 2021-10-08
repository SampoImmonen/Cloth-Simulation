#pragma once

#include <memory>

#include "Texture2D.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Buffers.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct Material {
    glm::vec3 albedo = glm::vec3(1.0f);
    float metallic = 0.7f;
    float roughness = 0.1f;
    float ao = 1.0f;

    std::unique_ptr<Texture2D> m_texture = nullptr;
};

class DirLight {
public:
    DirLight() = default;
    DirLight(const glm::vec3& dir);
    void setUniforms(Shader& shader);
    void ImGuiControls();
    glm::mat4 getLightSpaceMatrix();
    void prepareShadowMap(Shader& shader);

private:
    glm::vec3 m_direction = glm::vec3(0.0f, 1.0f, 1.0f);
    glm::vec3 m_intensity = glm::vec3(10.0f);

    //shadow parameters
    float m_scale = 1.0f;
    float m_size = 0.005f;
    bool m_shadows = true;

    ShadowMapBuffer m_shadowMap;
};