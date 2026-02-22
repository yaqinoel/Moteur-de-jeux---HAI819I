#include "SceneNode.hpp"

SceneNode::SceneNode(Model *model)
    : m_model(model), m_parent(nullptr), m_worldMatrix(1.0f) {}

SceneNode::~SceneNode() {
  for (SceneNode *child : m_children) {
    delete child;
  }
  m_children.clear();
}

void SceneNode::AddChild(SceneNode *child) {
  if (child) {
    child->m_parent = this;
    m_children.push_back(child);
  }
}

void SceneNode::Update(const glm::mat4 &parentWorldMatrix) {
  // 获取世界矩阵（parent * local）
  m_worldMatrix = parentWorldMatrix * m_transform.getLocalModelMatrix();

  // 更新孩子
  for (SceneNode *child : m_children) {
    child->Update(m_worldMatrix);
  }
}

void SceneNode::Draw(Shader &shader, const glm::mat4 &viewMatrix,
                     const glm::mat4 &projectionMatrix) {
  if (m_model) {
    // 计算MVP矩阵
    glm::mat4 mvp = projectionMatrix * viewMatrix * m_worldMatrix;
    shader.setMat4("MVP", mvp);

    // shader.setMat4("Model", m_worldMatrix);

    m_model->Draw(shader);
  }

  // 绘制孩子节点
  for (SceneNode *child : m_children) {
    child->Draw(shader, viewMatrix, projectionMatrix);
  }
}
