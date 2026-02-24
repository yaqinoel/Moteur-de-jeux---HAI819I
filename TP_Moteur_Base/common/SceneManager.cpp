#include "SceneManager.hpp"
#include <glm/gtc/matrix_transform.hpp>

SceneManager::SceneManager() { m_root = new SceneNode(nullptr); }

SceneManager::~SceneManager() { delete m_root; }

void SceneManager::Update() {
  if (m_root) {
    m_root->Update(glm::mat4(1.0f));
  }
}

void SceneManager::Draw(Shader &shader, const glm::mat4 &viewMatrix,
                        const glm::mat4 &projectionMatrix, const Cone &cone) {
  if (m_root) {
    m_root->Draw(shader, viewMatrix, projectionMatrix, cone);
  }
}

