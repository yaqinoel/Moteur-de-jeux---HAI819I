#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "SceneManager.hpp"
#include "GameObjet.hpp"

class GameManager {
public:
    SceneManager sceneManager;
    std::vector<GameObjet*> dynamicObjects;
    std::vector<GameObjet*> staticObjects;

    bool m_isSimulating = false;

    // 初始化世界
    GameManager() {
    }

    ~GameManager() {
        for (GameObjet* obj : dynamicObjects) {
            delete obj;
        }
        dynamicObjects.clear();
        for (GameObjet* obj : staticObjects) {
            delete obj;
        }
        staticObjects.clear();
    }

    // 添加游戏对象
    void AddDynamicGameObject(GameObjet* obj) {
        dynamicObjects.push_back(obj);
    }

    void AddStaticGameObject(GameObjet* obj) {
        staticObjects.push_back(obj);
    }

    // 更新状态
    void Update(float deltaTime) {
        if(!m_isSimulating) {
            for (GameObjet* obj : dynamicObjects) {
                obj->SyncTransform();
            }
            sceneManager.Update();
            return;
        }
        // 物理更新
        glm::vec3 gravity(0.0f, -9.81f, 0.0f);
        for (GameObjet* obj : dynamicObjects) {
            if (obj->physicsModel && obj->physicsModel->isDynamic()) {
                obj->physicsModel->AddForce(gravity * obj->physicsModel->m_mass);
                obj->physicsModel->Integrate(deltaTime);
            }
        }

        // 碰撞检测与响应
        for (GameObjet* dynObj : dynamicObjects) {
            for (GameObjet* statObj : staticObjects) {

                if (statObj->physicsModel->m_shape->GetType() == ShapeType::PLAN) {
                    float groundY = statObj->physicsModel->m_physicsPosition.y; // 假设是 0
                    CubeShape* dynCube = static_cast<CubeShape*>(dynObj->physicsModel->m_shape);
                    float bottomY = dynObj->physicsModel->m_physicsPosition.y - dynCube->m_halfExtent;

                    if (bottomY < groundY) {
                        // 修正位置
                        dynObj->physicsModel->m_physicsPosition.y = groundY + dynCube->m_halfExtent;
                        // dynObj->physicsModel->m_velocity = glm::vec3(0.f);

                        // 反弹
                        dynObj->physicsModel->m_velocity.y *= -0.7f;
                        dynObj->physicsModel->m_velocity.x *= 0.95f;
                        dynObj->physicsModel->m_velocity.z *= 0.95f;
                    }
                }
            }
        }

        // 状态同步
        for (GameObjet* obj : dynamicObjects) {
            obj->SyncTransform();
        }

        // 场景图与矩阵更新
        sceneManager.Update();
    }

    // 渲染世界
    void Draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const Cone &cone) {
        sceneManager.Draw( viewMatrix, projectionMatrix, cone);
    }

    void SwitchSimulationStatus() {
        if (m_isSimulating) {
            m_isSimulating = false;
            std::cout << "Physics Simulation Stopped!" << std::endl;
        }else {
            m_isSimulating = true;
            std::cout << "Physics Simulation Started!" << std::endl;
        }
    }

    void StopSimulationStatus() {
        m_isSimulating = false;
        std::cout << "Physics Simulation Stopped!" << std::endl;
    }


};

#endif //GAMEMANAGER_HPP