#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include "Cone.hpp"
#include "Transform.hpp"
#include "model.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <vector>

class SceneNode {
public:
  SceneNode(Model *model = nullptr);
  ~SceneNode();

  void AddChild(SceneNode *child);
  void RemoveChild(SceneNode *child);

  void Update(const glm::mat4 &parentWorldMatrix);
  void Draw(Shader &shader, const glm::mat4 &viewMatrix,
            const glm::mat4 &projectionMatrix, const Cone &cone);

  Transform &GetTransform() { return m_transform; }
  const glm::mat4 &GetWorldMatrix() const { return m_worldMatrix; }

  void SetModel(Model* newModel) {
      m_model = newModel;
  }

  bool m_isCullable = false;

private:
  Transform m_transform;
  glm::mat4 m_worldMatrix;

  SceneNode *m_parent;
  std::vector<SceneNode *> m_children;

  Model *m_model;
};

#endif // SCENENODE_HPP
