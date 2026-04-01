#ifndef GameObjet_HPP
#define GameObjet_HPP
#include "SceneManager.hpp"
#include "SceneNode.hpp"
#include "RenderingModel.hpp"
#include "PhysicsModel.hpp"

class GameObjet {
public:
    // 场景图
    SceneNode* sceneNode;

    // 物理模拟数据
    PhysicsModel* physicsModel;

    // 渲染数据
    RenderingModel* renderingModel;

    GameObjet() : sceneNode(nullptr), physicsModel(nullptr), renderingModel(nullptr) {}


    virtual ~GameObjet() {
        if (sceneNode) delete sceneNode;
        if (physicsModel) delete physicsModel;
        if (renderingModel) delete renderingModel;
    }


    // 更新物理状态
    virtual void SyncTransform() {
        if (!sceneNode || !physicsModel) {
            return;
        }
        if (physicsModel->isDynamic()) {
            sceneNode->GetTransform().setTranslation(physicsModel->m_physicsPosition);
            glm::vec3 eulerRadians = glm::eulerAngles(physicsModel->m_orientation);
            glm::vec3 eulerDegrees = glm::degrees(eulerRadians);
            sceneNode->GetTransform().setRotation(eulerDegrees);
        }else {
            physicsModel->m_physicsPosition = sceneNode->GetTransform().getTranslation();
            glm::vec3 eulerDegrees = sceneNode->GetTransform().getRotation();
            glm::vec3 eulerRadians = glm::radians(eulerDegrees);
            physicsModel->m_orientation = glm::quat(eulerRadians);
        }
    }


};

#endif // GameObjet_HPP