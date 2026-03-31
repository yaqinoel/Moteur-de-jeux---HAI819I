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

    const glm::vec3 GRAVITY = glm::vec3(0.0f, -9.81f, 0.0f);

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

        // 碰撞检测与响应和摩擦力
        for (GameObjet* dynObj : dynamicObjects) {
            for (GameObjet* statObj : staticObjects) {
                if (statObj->physicsModel->m_shape->GetType() == ShapeType::TERRAIN) {
                    TerrainSystem* terrain = static_cast<TerrainSystem*>(statObj);
                    CubeShape* dynCube = static_cast<CubeShape*>(dynObj->physicsModel->m_shape);

                    glm::vec3 centerPosition = dynObj->physicsModel->m_physicsPosition;

                    // 获取地形处法线
                    float groundY = terrain->GetHeightAt(centerPosition.x, centerPosition.z);
                    glm::vec3 groundNormal = terrain->GetNormalAt(centerPosition.x, centerPosition.z);

                    // 中心点到地面垂直距离
                    float distToPlane = (centerPosition.y - groundY) * groundNormal.y;

                    float radius = dynCube->m_halfExtent;
                    float contactTolerance = 0.01f;

                    // 发生碰撞
                    if (distToPlane < radius + contactTolerance) {

                        // 修正位置
                        if (distToPlane < radius) {
                            float penetrationDepth = radius - distToPlane;
                            dynObj->physicsModel->m_physicsPosition += groundNormal * penetrationDepth;
                        }
                        float penetrationDepth = radius - distToPlane;
                        // dynObj->physicsModel->m_velocity = glm::vec3(0.f);

                        // 消除砸向地面的速度
                        float vDotN = glm::dot(dynObj->physicsModel->m_velocity, groundNormal);
                        if (vDotN < 0.0f) {
                            dynObj->physicsModel->m_velocity -= vDotN * groundNormal;
                        }

                        // 更新角度
                        glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);
                        glm::quat alignQuat = glm::rotation(WorldUp, groundNormal);
                        glm::vec3 targetEuler = glm::degrees(glm::eulerAngles(alignQuat));
                        dynObj->sceneNode->GetTransform().setRotation(targetEuler);

                        // 计算合力
                        // 计算力在法线方向
                        glm::vec3 forceGravity = GRAVITY * dynObj->physicsModel->m_mass;
                        glm::vec3 forceNormal = -(glm::dot(forceGravity, groundNormal)) * groundNormal;
                        dynObj->physicsModel->AddForce(forceNormal);

                        // 计算阻力
                        float us = terrain->m_us;
                        float uk = terrain->m_uk;
                        float velocityEpsilon = 0.01f;
                        float forceNormalMagnitude = glm::length(forceNormal);
                        glm::vec3 currentSpeed = dynObj->physicsModel->m_velocity;
                        float speedMagnitude = glm::length(currentSpeed);

                        // 动摩擦力
                        if (speedMagnitude > velocityEpsilon) {
                            glm::vec3 moveDir = glm::normalize(currentSpeed);
                            glm::vec3 forceFriction = -moveDir * (forceNormalMagnitude * uk);
                            float frictionAccel = (forceNormalMagnitude * uk) / dynObj->physicsModel->m_mass;
                            if (frictionAccel * deltaTime >= speedMagnitude) {
                                dynObj->physicsModel->m_velocity = glm::vec3(0.0f);
                            } else {
                                dynObj->physicsModel->AddForce(forceFriction);
                            }
                        }
                        // 静摩擦力
                        else {
                            glm::vec3 forceTangential = forceGravity + forceNormal;
                            float tangentialMagnitude = glm::length(forceTangential);

                            // 计算最大静摩擦力极限
                            float maxStaticFriction = forceNormalMagnitude * us;

                            // 比较下滑力与极限静摩擦力
                            if (tangentialMagnitude <= maxStaticFriction) {
                                dynObj->physicsModel->AddForce(-forceTangential);
                                dynObj->physicsModel->m_velocity = glm::vec3(0.0f);
                            } else {
                                if (tangentialMagnitude > 0.0001f) {
                                    glm::vec3 slideDir = forceTangential / tangentialMagnitude;
                                    glm::vec3 kineticFriction = -slideDir * (forceNormalMagnitude * uk);
                                    dynObj->physicsModel->AddForce(kineticFriction);
                                }
                            }
                        }
                    }
                }
            }
        }

        // 浮力检测
        float water_level = 0.f;
        float rou_water = 1000.f;
        float rou_air = 1.f;
        glm::vec3 water_normal = glm::vec3(0.f,1.f,0.f);
        for (GameObjet* dynObj : dynamicObjects) {
            // 检测物体是否入水
            CubeShape* dynCube = static_cast<CubeShape*>(dynObj->physicsModel->m_shape);
            glm::vec3 objetPosition = dynObj->physicsModel->m_physicsPosition;
            float half_extends = dynCube->m_halfExtent;
            // 物体中心落水
            if (objetPosition.y - half_extends < water_level) {
                // 计算浮力
                float volume = 0.f;
                float height_in_water = 2 * half_extends;
                if (objetPosition.y + half_extends > water_level) {
                    height_in_water = half_extends + (water_level - objetPosition.y);
                }
                volume = 4 * half_extends * half_extends * height_in_water;
                glm::vec3 flottaisonForce = -rou_water * GRAVITY * volume *  water_normal;
                dynObj->physicsModel->AddForce(flottaisonForce);

                // 计算水的阻力
                glm::vec3 currentVelocity = dynObj->physicsModel->m_velocity;
                float submergedRatio = height_in_water / (2.0f * half_extends);
                // 流体阻力系数
                float fluidDragCoefficient = 100.0f;
                // 简单的线性阻力模型阻力方向与速度相反
                glm::vec3 dragForce = -fluidDragCoefficient * submergedRatio * currentVelocity;
                dynObj->physicsModel->AddForce(dragForce);

                // 调整旋转姿态
                glm::vec3 currentEuler = dynObj->sceneNode->GetTransform().getRotation();
                glm::quat currentQuat = glm::quat(glm::radians(currentEuler));
                glm::vec3 targetEuler = glm::vec3(0.0f, currentEuler.y, 0.0f);
                glm::quat targetQuat = glm::quat(glm::radians(targetEuler));

                // 根据浸入深度计算插值权重
                float alignSpeed = 5.0f;
                float blendFactor = glm::clamp(submergedRatio * alignSpeed * deltaTime, 0.0f, 1.0f);
                glm::quat newQuat = glm::slerp(currentQuat, targetQuat, blendFactor);

                // 将新姿态应用回物体
                dynObj->sceneNode->GetTransform().setRotation(glm::degrees(glm::eulerAngles(newQuat)));
            }
        }

        // 添加万有引力和重力
        for (GameObjet* obj : dynamicObjects) {
            if (obj->physicsModel && obj->physicsModel->isDynamic() && !obj->physicsModel->m_isGravity && !obj->physicsModel->m_isFixed) {
                for (GameObjet* statObj : staticObjects) {
                    float mass = statObj->physicsModel->m_mass;
                    glm::vec3 direction = obj->physicsModel->m_physicsPosition - statObj->physicsModel->m_physicsPosition;
                    glm::vec3 universalForce = GRAVITY.y * mass * obj->physicsModel-> m_mass * direction;
                    obj->physicsModel->AddForce(universalForce);
                }
            }

            // 添加重力
            if (obj->physicsModel && obj->physicsModel->isDynamic() && obj->physicsModel->m_isGravity) {
                obj->physicsModel->AddForce(GRAVITY * obj->physicsModel->m_mass);
            }
        }

        // 物理更新
        for (GameObjet* obj : dynamicObjects) {
            datalogger->sample(deltaTime);
            // 迭代位置
            obj->physicsModel->Integrate(deltaTime);
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