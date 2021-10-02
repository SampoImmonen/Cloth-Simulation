#pragma once

#include "glad/glad.h" 
#include <iostream>
#include <vector>
#include "glm/glm.hpp"

struct Vertex{
    glm::vec3 position;
    glm::vec3 color = glm::vec3(0.3f);

    Vertex(const glm::vec3& pos, const glm::vec3& col): position(pos), color(col){}
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
