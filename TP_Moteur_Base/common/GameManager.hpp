#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "SceneManager.hpp"
#include "GameObjet.hpp"
#include "Ressort.hpp"

struct ContactInfo {
    bool hasCollision = false;
    glm::vec3 normal;
    float penetrationDepth = 0.f;
    float us = 0.f;
    float uk = 0.f;

    // PhysicsModel* objA;
    // PhysicsModel* objB;
};

class GameManager {
public:
    SceneManager sceneManager;
    std::vector<GameObjet*> dynamicObjects;
    std::vector<GameObjet*> staticObjects;
    std::vector<Ressort*> resorts;
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

    void AddResort(Ressort* ressort) {
        resorts.push_back(ressort);
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

                // 调用统一碰撞检测接口
                ContactInfo contact = CheckCollision(dynObj, statObj);

                // 如果发生碰撞
                if (contact.hasCollision) {
                    // 修正位置
                    if (contact.penetrationDepth > 0.f) {
                        dynObj->physicsModel->m_physicsPosition += contact.normal * contact.penetrationDepth;
                    }

                    // 消除砸向地面的速度
                    float vDotN = glm::dot(dynObj->physicsModel->m_velocity, contact.normal);
                    if (vDotN < 0.0f) {
                        dynObj->physicsModel->m_velocity -= vDotN * contact.normal;
                    }

                    // 计算支持力
                    glm::vec3 forceGravity = GRAVITY * dynObj->physicsModel->m_mass;
                    glm::vec3 forceNormal = -(glm::dot(forceGravity, contact.normal)) * contact.normal;
                    dynObj->physicsModel->AddForce(forceNormal);

                    // 计算摩擦力
                    float us = contact.us;
                    float uk = contact.uk;
                    float velocityEpsilon = 0.01f;
                    float forceNormalMagnitude = glm::length(forceNormal);
                    glm::vec3 currentSpeed = dynObj->physicsModel->m_velocity;
                    float speedMagnitude = glm::length(currentSpeed);

                    // 动摩擦力
                    if (speedMagnitude > velocityEpsilon) {
                        glm::vec3 moveDir = glm::normalize(currentSpeed);glm::vec3 forceFriction = -moveDir * (forceNormalMagnitude * uk);
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

        // 浮力检测和计算
        for (GameObjet* dynObj : dynamicObjects) {
            CheckFloatage(dynObj, deltaTime);
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

        // 添加弹簧力
        for (Ressort* ressort : resorts) {
            GameObjet* objetStart = ressort->m_objetStart;
            GameObjet* objetEnd = ressort->m_objetEnd;
            glm::vec3 objetStartPosition = objetStart->physicsModel->m_physicsPosition;
            glm::vec3 objetEndPosition = objetEnd->physicsModel->m_physicsPosition;

            // 计算弹力方向, 从端点指向原点
            glm::vec3 forceDirection = objetEndPosition - objetStartPosition;
            forceDirection = glm::normalize(forceDirection);
            float currentLength = glm::length(forceDirection);  //弹簧现长
            float distortLength = currentLength - ressort->m_length;

            // 计算弹力大小
            float forceMagnitude = ressort->m_k * distortLength;

            // 应用弹力
            if (!objetStart->physicsModel->m_isFixed) {
                objetStart->physicsModel->AddForce(-forceDirection * forceMagnitude);
            }
            if (!objetEnd->physicsModel->m_isFixed) {
                objetEnd->physicsModel->AddForce(forceDirection * forceMagnitude);
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

    ContactInfo CheckCollision(GameObjet* dynObj, GameObjet* statObj) {
        ContactInfo contact_info;
        ShapeType typeDyn = dynObj->physicsModel->m_shape->GetType();
        ShapeType typeStat = statObj->physicsModel->m_shape->GetType();

        float contactTolerance = 0.01f;
        glm::vec3 WorldUp(0.f,1.f,0.f);

        // 方块与地形
        if (typeDyn == ShapeType::CUBE && typeStat == ShapeType::TERRAIN) {
            CubeShape* cube  = static_cast<CubeShape*>(dynObj->physicsModel->m_shape);
            TerrainSystem* terrain = static_cast<TerrainSystem *>(statObj);

            float groundY = terrain->GetHeightAt(dynObj->physicsModel->m_physicsPosition.x, dynObj->physicsModel->m_physicsPosition.z);
            glm::vec3 groundNormal = terrain->GetNormalAt(dynObj->physicsModel->m_physicsPosition.x, dynObj->physicsModel->m_physicsPosition.z);

            float distToPlane = (dynObj->physicsModel->m_physicsPosition.y - groundY) * groundNormal.y;
            float radius = cube->m_halfExtent;

            if (distToPlane < radius + contactTolerance) {
                contact_info.hasCollision = true;
                contact_info.normal = groundNormal;
                contact_info.penetrationDepth = (distToPlane < radius) ? (radius - distToPlane) : 0.0f;
                contact_info.us = terrain->m_us;
                contact_info.uk = terrain->m_uk;

                // 更新方块的角度
                glm::quat alignQuat = glm::rotation(WorldUp, groundNormal);
                dynObj->sceneNode->GetTransform().setRotation(glm::degrees(glm::eulerAngles(alignQuat)));
            }
        }

        // 球体与地形
        else if (typeDyn == ShapeType::SPHERE && typeStat == ShapeType::TERRAIN) {
            SphereShape* sphere = static_cast<SphereShape*>(dynObj->physicsModel->m_shape);
            TerrainSystem* terrain = static_cast<TerrainSystem *>(statObj);

            float groundY = terrain->GetHeightAt(dynObj->physicsModel->m_physicsPosition.x, dynObj->physicsModel->m_physicsPosition.z);
            glm::vec3 groundNormal = terrain->GetNormalAt(dynObj->physicsModel->m_physicsPosition.x, dynObj->physicsModel->m_physicsPosition.z);

            float distToPlane = (dynObj->physicsModel->m_physicsPosition.y - groundY) * groundNormal.y;
            float radius = sphere->m_radius;

            if (distToPlane < radius + contactTolerance) {
                contact_info.hasCollision = true;
                contact_info.normal = groundNormal;
                contact_info.penetrationDepth = (distToPlane < radius) ? (radius - distToPlane) : 0.0f;
                contact_info.us = terrain->m_us;
                contact_info.uk = terrain->m_uk;
            }
        }

        return contact_info;
    }

    void CheckFloatage(GameObjet* dynObj, float deltaTime) {
        float water_level = 0.f;
        float rou_water = 1000.f;
        glm::vec3 water_normal = glm::vec3(0.f,1.f,0.f);

        glm::vec3 objetPosition = dynObj->physicsModel->m_physicsPosition;
        ShapeType type = dynObj->physicsModel->m_shape->GetType();

        // 核心物理数据
        bool isTouchingWater = false;
        float volume = 0.f;
        float submergedRatio = 0.f;

        // 根据不同物体计算排开水体积
        if (type == ShapeType::CUBE) {
            CubeShape* cube = static_cast<CubeShape*>(dynObj->physicsModel->m_shape);
            float r = cube->m_halfExtent;
            float bottomY = objetPosition.y - r;

            if (bottomY < water_level) {
                isTouchingWater = true;
                float height_in_water = 2.0f * r; // 默认全淹没
                if (objetPosition.y + r > water_level) {
                    height_in_water = water_level - bottomY; // 部分淹没
                }
                // 立方体排开水体积 = 底面积 * 高
                volume = (4.0f * r * r) * height_in_water;
                submergedRatio = height_in_water / (2.0f * r);
            }
        }
        else if (type == ShapeType::SPHERE) {
            SphereShape* sphere = static_cast<SphereShape*>(dynObj->physicsModel->m_shape);
            float R = sphere->m_radius;
            float bottomY = objetPosition.y - R;

            if (bottomY < water_level) {
                isTouchingWater = true;
                float h = 2.0f * R; // 默认全淹没
                if (objetPosition.y + R > water_level) {
                    h = water_level - bottomY; // 浸入水下的深度
                }
                float PI = glm::pi<float>();
                volume = (PI * h * h / 3.0f) * (3.0f * R - h);
                submergedRatio = h / (2.0f * R);
            }
        }

        // 统一计算浮力
        if (isTouchingWater) {
            float g_magnitude = std::abs(GRAVITY.y);

            // 阿基米德浮力
            float buoyancyMagnitude = rou_water * g_magnitude * volume;
            glm::vec3 flottaisonForce = buoyancyMagnitude * water_normal;
            dynObj->physicsModel->AddForce(flottaisonForce);

            // 流体阻力
            float fluidDragCoefficient = 1000.0f;
            glm::vec3 currentVelocity = dynObj->physicsModel->m_velocity;
            glm::vec3 dragForce = -fluidDragCoefficient * submergedRatio * currentVelocity;
            dynObj->physicsModel->AddForce(dragForce);

            // 姿态扶正 (如果需要的话
            if (type == ShapeType::CUBE) {
                glm::vec3 currentEuler = dynObj->sceneNode->GetTransform().getRotation();
                glm::quat currentQuat = glm::quat(glm::radians(currentEuler));
                glm::vec3 targetEuler = glm::vec3(0.0f, currentEuler.y, 0.0f);
                glm::quat targetQuat = glm::quat(glm::radians(targetEuler));

                float alignSpeed = 5.0f;
                float blendFactor = glm::clamp(submergedRatio * alignSpeed * deltaTime, 0.0f, 1.0f);
                glm::quat newQuat = glm::slerp(currentQuat, targetQuat, blendFactor);
                dynObj->sceneNode->GetTransform().setRotation(glm::degrees(glm::eulerAngles(newQuat)));
            }
        }
    }

};

#endif //GAMEMANAGER_HPP