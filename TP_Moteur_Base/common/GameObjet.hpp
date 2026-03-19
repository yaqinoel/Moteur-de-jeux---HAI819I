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

        }else {
            physicsModel->m_physicsPosition = sceneNode->GetTransform().getTranslation();
        }
    }


};

#endif // GameObjet_HPP