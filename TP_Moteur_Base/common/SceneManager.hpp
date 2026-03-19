#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP

#include "Cone.hpp"
#include "SceneNode.hpp"
#include "shader.hpp"

class SceneManager {
public:
  SceneManager();
  ~SceneManager();

  SceneNode *GetRoot() { return m_root; }

  void Update();
  void Draw(const glm::mat4 &viewMatrix,
            const glm::mat4 &projectionMatrix, const Cone &cone);

private:
  SceneNode *m_root;
};

#endif // SCENEMANAGER_HPP
