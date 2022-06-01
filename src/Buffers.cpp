#include "Buffers.h"

VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices) : m_vertices(vertices) {
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices) : m_indices(indices) {
  glGenBuffers(1, &m_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), &m_indices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

VertexArray::VertexArray(const VertexBuffer& vbo, const IndexBuffer& ebo) : m_vbo(vbo), m_ebo(ebo) {
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);
  vbo.bind();
  ebo.bind();

  // position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  // color
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  // texCoords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
  // tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
}


ShadowMapBuffer::ShadowMapBuffer(unsigned int height, unsigned int width)
    : m_width(width), m_height(height) {
  glGenFramebuffers(1, &m_framebufferId);

  glGenTextures(1, &m_depthmapId);
  glBindTexture(GL_TEXTURE_2D, m_depthmapId);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_DEPTH_COMPONENT,
      m_width,
      m_height,
      0,
      GL_DEPTH_COMPONENT,
      GL_FLOAT,
      NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthmapId, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (!checkCompleteness()) {
    std::cout << "Buffer not complete\n";
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapBuffer::bind() {
  // change viewport to suit shadowmap resolution
  glViewport(0, 0, m_width, m_height);
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMapBuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapBuffer::bindDepthTexture(int textureunit) {
  glActiveTexture(GL_TEXTURE0 + textureunit);
  glBindTexture(GL_TEXTURE_2D, m_depthmapId);
}

unsigned int* ShadowMapBuffer::getTexture() {
  return &m_depthmapId;
}

bool ShadowMapBuffer::checkCompleteness() {
  bind();
  return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}