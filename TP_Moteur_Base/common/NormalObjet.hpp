#ifndef NORMALOBJET_HPP
#define NORMALOBJET_HPP
#include "GameObjet.hpp"

class NormalObjet : public GameObjet {
public:

    NormalObjet(SceneManager& sceneManager, Shader* shader,std::string path,  float scale, float mass, glm::vec3 startPos) : GameObjet() {
        // 初始化模型
        renderingModel = new RenderingModel(path);
        renderingModel->shader = shader;

        // 绑定节点
        sceneNode = new SceneNode(nullptr);
        sceneNode->GetTransform().setTranslation(startPos);
        sceneNode->GetTransform().setScale(glm::vec3(scale));
        sceneManager.GetRoot()->AddChild(sceneNode);

        SceneNode * meshNode = new SceneNode(renderingModel);
        meshNode->GetTransform().setTranslation(glm::vec3(0.f, -1.f, 0.f));
        sceneNode->AddChild(meshNode);


        EmptyShape* shape = new EmptyShape();
        // 设置物理模型上
        physicsModel = new PhysicsModel(shape, mass, startPos);

        // 同步状态
        SyncTransform();
    }

    ~NormalObjet() {
        delete sceneNode;
    }

};

#endif //NORMALOBJET_HPP