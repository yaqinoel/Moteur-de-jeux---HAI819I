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

void SceneNode::RemoveChild(SceneNode *child) {
  if (!child)
    return;

  auto it = std::find(m_children.begin(), m_children.end(), child);

  if (it != m_children.end()) {
    (*it)->m_parent = nullptr;
    m_children.erase(it);
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
                     const glm::mat4 &projectionMatrix, const Cone &cone) {

  if (m_isCullable) {
    // 提取全局坐标
    glm::vec3 globalPos = glm::vec3(m_worldMatrix[3]);

    // 如果节点中心位置不在圆锥体内，直接抛弃该节点和它的所有子节点
    if (!cone.containsPoint(globalPos)) {
      return;
    }
  }

  if (m_model) {
    // 计算MVP矩阵
    glm::mat4 mvp = projectionMatrix * viewMatrix * m_worldMatrix;
    shader.setMat4("MVP", mvp);
    m_model->Draw(shader);
  }

  // 绘制孩子节点
  for (SceneNode *child : m_children) {
    child->Draw(shader, viewMatrix, projectionMatrix, cone);
  }
}