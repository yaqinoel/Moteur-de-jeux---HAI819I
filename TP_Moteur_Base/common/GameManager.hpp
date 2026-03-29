#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "SceneManager.hpp"
#include "GameObjet.hpp"

class GameManager {
public:
    SceneManager sceneManager;
    std::vector<GameObjet*> dynamicObjects;
    std::vector<GameObjet*> staticObjects;
    DataLogger* datalogger;

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
            datalogger->sample(deltaTime);
            if (obj->physicsModel && obj->physicsModel->isDynamic()) {
                obj->physicsModel->AddForce(gravity * obj->physicsModel->m_mass);
                obj->physicsModel->Integrate(deltaTime);
            }
        }

        // 碰撞检测与响应
        for (GameObjet* dynObj : dynamicObjects) {
            for (GameObjet* statObj : staticObjects) {
                if (statObj->physicsModel->m_shape->GetType() == ShapeType::TERRAIN) {
                    glm::vec3 dynamicPosition = dynObj->physicsModel->m_physicsPosition;
                    TerrainSystem* terrain = static_cast<TerrainSystem*>(statObj);

                    float groundY = terrain->GetHeightAt(dynamicPosition.x, dynamicPosition.z);
                    glm::vec3 groundNormal = terrain->GetNormalAt(dynamicPosition.x, dynamicPosition.z);

                    CubeShape* dynCube = static_cast<CubeShape*>(dynObj->physicsModel->m_shape);
                    float bottomY = dynObj->physicsModel->m_physicsPosition.y - dynCube->m_halfExtent;

                    if (bottomY < groundY) {
                        // 修正位置
                        dynObj->physicsModel->m_physicsPosition.y = groundY + dynCube->m_halfExtent;
                        // dynObj->physicsModel->m_velocity = glm::vec3(0.f);

                        glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

                        glm::quat alignQuat = glm::rotation(WorldUp, groundNormal);
                        glm::vec3 targetEuler = glm::degrees(glm::eulerAngles(alignQuat));
                        dynObj->sceneNode->GetTransform().setRotation(targetEuler);

                        // 反弹
                        // dynObj->physicsModel->m_velocity.y *= -0.7f;
                        dynObj->physicsModel->m_velocity.x *= 0.999f;
                        dynObj->physicsModel->m_velocity.z *= 0.999f;
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

    void setDataLogger(DataLogger* _dataLogger) {
        datalogger = _dataLogger;
    }

};

#endif //GAMEMANAGER_HPP