#pragma once

#include "glad/glad.h" 
#include <iostream>
#include <vector>
#include "glm/glm.hpp"

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;

    Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& tex, const glm::vec3& tangent): position(pos), normal(normal), texCoords(tex), tangent(tangent){}
};

class VertexBuffer{
public:
    VertexBuffer() = default;
    VertexBuffer(const std::vector<Vertex>& vertices);
    void bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    };
    void unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    };

    size_t getSize() const {
        return m_vertices.size();
    }
private:
    uint32_t m_vbo;
    std::vector<Vertex> m_vertices;
};

class IndexBuffer{
public:
    IndexBuffer() = default;
    IndexBuffer(const std::vector<uint32_t>& indices);
    void bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    };
    void unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    };
    size_t getSize() const {
        return m_indices.size();
    }
private:
    std::vector<uint32_t> m_indices;
    uint32_t m_ebo;
};

class VertexArray{
public:
    VertexArray() = default;
    VertexArray(const VertexBuffer& vbo, const IndexBuffer& ebo);
    void drawElements(){
        bind();
        glDrawElements(GL_TRIANGLES, m_ebo.getSize(), GL_UNSIGNED_INT, 0);
    };
    void bind() const {
        glBindVertexArray(m_vao);
    };
    void unbind() const{
        glBindVertexArray(0);
    };

private:
    VertexBuffer m_vbo;
    IndexBuffer m_ebo;
    uint32_t m_vao;
};

class ShadowMapBuffer {
public:
	ShadowMapBuffer() = default;
	ShadowMapBuffer(unsigned int height, unsigned int width);
	void bind();

	void unbind();
	void bindDepthTexture(int textureunit);
	unsigned int* getTexture();

private:
	unsigned int m_width, m_height;
	unsigned int m_framebufferId, m_depthmapId;
	bool checkCompleteness();
};