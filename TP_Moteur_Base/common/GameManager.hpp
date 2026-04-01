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
    glm::vec3 worldContactPoint = glm::vec3(0.0f);

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
                        float restitution = 0.4f;
                        float impulseMagnitude = -(1.0f + restitution) * vDotN;
                        dynObj->physicsModel->m_velocity += impulseMagnitude * contact.normal;
                    }

                    // 计算支持力
                    glm::vec3 forceGravity = GRAVITY * dynObj->physicsModel->m_mass;
                    glm::vec3 forceNormal = -(glm::dot(forceGravity, contact.normal)) * contact.normal;
                    dynObj->physicsModel->AddForceAtPoint(forceNormal, contact.worldContactPoint);

                    // 计算摩擦力
                    float us = contact.us;
                    float uk = contact.uk;

                    // 获取接触点的真实相对速度
                    glm::vec3 r = contact.worldContactPoint - dynObj->physicsModel->m_physicsPosition;
                    glm::vec3 velocityAtPoint = dynObj->physicsModel->m_velocity +
                                                glm::cross(dynObj->physicsModel->m_angularVelocity, r);

                    float speedMagnitude = glm::length(velocityAtPoint);

                    // 只有接触点在滑动时，才产生摩擦力
                    if (speedMagnitude > 0.0001f) {
                        glm::vec3 moveDir = velocityAtPoint / speedMagnitude;
                        float forceNormalMagnitude = glm::length(forceNormal);

                        // 计算“完全阻止滑动”所需要的理想力大小
                        float requiredForceToStop = (speedMagnitude * dynObj->physicsModel->m_mass) / deltaTime;
                        // 极限静摩擦力
                        float maxStaticFriction = forceNormalMagnitude * us;
                        float appliedFrictionMagnitude = 0.0f;

                        // 判断是静摩擦还是动摩擦
                        if (requiredForceToStop <= maxStaticFriction) {
                            // 提供刚好足够的静摩擦力来维持这个状态
                            appliedFrictionMagnitude = requiredForceToStop;
                        } else {
                            // 只能提供最大动摩擦力
                            appliedFrictionMagnitude = forceNormalMagnitude * uk;
                        }

                        // 将摩擦力施加到接触点上
                        glm::vec3 forceFriction = -moveDir * appliedFrictionMagnitude;
                        dynObj->physicsModel->AddForceAtPoint(forceFriction, contact.worldContactPoint);
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

            float h = cube->m_halfExtent;
            // 1. 定义局部坐标系下的 8 个顶点
            glm::vec3 localVertices[8] = {
                glm::vec3( h,  h,  h), glm::vec3( h,  h, -h),
                glm::vec3( h, -h,  h), glm::vec3( h, -h, -h),
                glm::vec3(-h,  h,  h), glm::vec3(-h,  h, -h),
                glm::vec3(-h, -h,  h), glm::vec3(-h, -h, -h)
            };

            float maxPenetration = -9999.0f; // 记录最大穿透深度
            glm::vec3 deepestContactPoint;   // 记录扎得最深的点
            glm::vec3 deepestNormal;

            for (int i = 0; i < 8; i++) {
                glm::vec3 worldVertex = dynObj->physicsModel->m_physicsPosition +
                                    (dynObj->physicsModel->m_orientation * localVertices[i]);

                // 查询该点对应的地形高度
                float groundY = terrain->GetHeightAt(worldVertex.x, worldVertex.z);

                // 计算穿透深度
                float penetration = groundY - worldVertex.y;

                // 寻找陷得最深的那个角
                if (penetration > maxPenetration) {
                    maxPenetration = penetration;
                    deepestContactPoint = worldVertex;
                    // 获取该处的法线
                    deepestNormal = terrain->GetNormalAt(worldVertex.x, worldVertex.z);
                }
            }

            if (maxPenetration > -contactTolerance) {
                contact_info.hasCollision = true;
                contact_info.penetrationDepth = maxPenetration > 0.0f ? maxPenetration : 0.0f;
                contact_info.worldContactPoint = deepestContactPoint;
                contact_info.normal = deepestNormal;
                contact_info.us = terrain->m_us;
                contact_info.uk = terrain->m_uk;
            }

        }

        // 球体与地形
        else if (typeDyn == ShapeType::SPHERE && typeStat == ShapeType::TERRAIN) {
            SphereShape* sphere = static_cast<SphereShape*>(dynObj->physicsModel->m_shape);
            TerrainSystem* terrain = static_cast<TerrainSystem *>(statObj);

            glm::vec3 pos = dynObj->physicsModel->m_physicsPosition;
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
                contact_info.worldContactPoint = pos - groundNormal * radius;
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