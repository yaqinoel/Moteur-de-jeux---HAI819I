#ifndef PHYSIQUEMODEL_HPP
#define PHYSIQUEMODEL_HPP
#include "glm/detail/type_vec.hpp"
#include "common/ColliderShape.hpp"

class ColliderShape;

class PhysicsModel {
public:

    // -- 物理属性
    float m_mass;

    // -- 空间状态
    glm::vec3 m_physicsPosition;

    // -- 动力学状态
    glm::vec3 m_velocity;
    glm::vec3 m_force;

    // 碰撞形状
    ColliderShape* m_shape;

    bool m_isFixed;
    bool m_isGravity;

    // 构造函数
    PhysicsModel(ColliderShape* shape, float mass, glm::vec3 startPos)
        : m_shape(shape),
          m_mass(mass),
          m_physicsPosition(startPos),
          m_velocity(glm::vec3(0.0f)),
          m_force(glm::vec3(0.0f)),
          m_isFixed(false),
          m_isGravity(true){}

    ~PhysicsModel() {
        delete m_shape;
    }

    bool isDynamic() const {
        return m_mass > 0.0f;
    }

    void AddForce(const glm::vec3& force) {
        if (isDynamic()) {
            m_force += force;
        }
    }

    void AddVelocity(const glm::vec3& direction, const float amplitude) {
        if (isDynamic()) {
            m_velocity += direction * amplitude;
        }
    }

    void SetVelocity(const glm::vec3& velocity) {
        if (isDynamic()) {
            m_velocity = velocity;
        }
    }

    void Integrate(float deltaTime){
        if (isDynamic()) {
            // 更新速度
            glm::vec3 acceleation = m_force / m_mass;
            m_velocity += acceleation * deltaTime;
            // 更新位置
            m_physicsPosition += m_velocity * deltaTime;
            m_force = glm::vec3(0.0f);
        }
    }



};

#endif // PHYSIQUEMODEL_HPP