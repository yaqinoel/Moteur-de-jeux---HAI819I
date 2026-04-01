#ifndef SPHEREOBJET_HPP
#define SPHEREOBJET_HPP
#include "GameObjet.hpp"

class SphereObjet : public GameObjet {
public:
    float m_radius;
    float m_mass;
    glm::vec3 m_startPosition;

    SphereObjet(SceneManager& sceneManager, Shader* shader, float radius, float mass, glm::vec3 startPos) :GameObjet() {
        m_radius = radius;
        m_mass = mass;
        m_startPosition = startPos;

        std::string path = "./resources/models/planete/earth.obj";
        renderingModel = new RenderingModel(path);
        renderingModel->shader = shader;

        // 挂载到场景树上
        sceneNode = new SceneNode(renderingModel);
        sceneNode->GetTransform().setTranslation(m_startPosition);
        sceneNode->GetTransform().setRotation(glm::vec3(0.f,0.f,0.f));
        sceneManager.GetRoot()->AddChild(sceneNode);

        // 物理数据初始化
        SphereShape* cubeShape = new SphereShape(m_radius);
        physicsModel = new PhysicsModel(cubeShape, m_mass,sceneNode->GetTransform().getTranslation() );

        // 同步状态
        SyncTransform();
    }


    void SetVelocity(const glm::vec3& velocity) {
        if (physicsModel->isDynamic()) {
            physicsModel->m_velocity = velocity;
        }
    }

    void ResetStatus() {
        physicsModel->m_physicsPosition = m_startPosition;
        sceneNode->GetTransform().setRotation(glm::vec3(0.f,0.f,0.f));
    }

};

#endif //SPHEREOBJET_HPP