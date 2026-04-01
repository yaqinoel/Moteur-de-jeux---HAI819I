#ifndef PHYSIQUEMODEL_HPP
#define PHYSIQUEMODEL_HPP
#include "glm/detail/type_vec.hpp"
#include "common/ColliderShape.hpp"

class ColliderShape;

class PhysicsModel {
public:

    // -- 物理属性
    float m_mass;
    bool m_isFixed;
    bool m_isGravity;

    // -- 空间状态
    glm::vec3 m_physicsPosition;
    glm::quat m_orientation;

    // -- 动力学状态
    glm::vec3 m_velocity;
    glm::vec3 m_force;
    glm::vec3 m_angularVelocity;
    glm::vec3 m_torque;

    // -- 转动惯量
    glm::mat3 m_inertiaTensorBody;       // 局部转动惯量矩阵
    glm::mat3 m_invInertiaTensorBody;    // 局部逆转动惯量矩阵
    glm::mat3 m_invInertiaTensorWorld;   // 世界逆转动惯量矩阵

    // 碰撞形状
    ColliderShape* m_shape;



    // 构造函数
    PhysicsModel(ColliderShape* shape, float mass, glm::vec3 startPos)
        : m_shape(shape),
          m_mass(mass),
          m_physicsPosition(startPos),
          m_velocity(glm::vec3(0.0f)),
          m_force(glm::vec3(0.0f)),
          m_isFixed(false),
          m_isGravity(true),
          m_orientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
          m_angularVelocity(glm::vec3(0.0f)),
          m_torque(glm::vec3(0.0f))
    {
        CalculateInertia();
    }

    ~PhysicsModel() {
        delete m_shape;
    }

    bool isDynamic() const {
        return m_mass > 0.0f && !m_isFixed;
    }

    void AddForce(const glm::vec3& force) {
        if (isDynamic()) {
            m_force += force;
        }
    }

    void AddForceAtPoint(const glm::vec3& force, const glm::vec3& worldPoint) {
        if (isDynamic()) {
            m_force += force;
            // 计算力臂向量 r (从质心指向受力点)
            glm::vec3 r = worldPoint - m_physicsPosition;
            // 力矩 = 力臂 x 力
            m_torque += glm::cross(r, force);
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
            // 平移运动积分
            glm::vec3 acceleation = m_force / m_mass;
            m_velocity += acceleation * deltaTime;
            // m_velocity *= 0.99f;
            // 更新位置
            m_physicsPosition += m_velocity * deltaTime;

            // 旋转运动积分
            glm::mat3 rotationMatrix = glm::mat3_cast(m_orientation);
            m_invInertiaTensorWorld = rotationMatrix * m_invInertiaTensorBody * glm::transpose(rotationMatrix);
            glm::vec3 angularAcceleration = m_invInertiaTensorWorld * m_torque;
            m_angularVelocity += angularAcceleration * deltaTime;
            m_angularVelocity *= 0.98f;
            float angularSpeed = glm::length(m_angularVelocity);
            if (angularSpeed > 0.0001f) { // 避免零除
                glm::vec3 rotationAxis = m_angularVelocity / angularSpeed;
                float rotationAngle = angularSpeed * deltaTime;

                glm::quat deltaQuat = glm::angleAxis(rotationAngle, rotationAxis);
                m_orientation = deltaQuat * m_orientation;
                m_orientation = glm::normalize(m_orientation);
            }

            m_force = glm::vec3(0.0f);
            m_torque = glm::vec3(0.0f);
        }
    }

private:
    void CalculateInertia() {
        if (m_mass <= 0.0f) {
            m_inertiaTensorBody = glm::mat3(0.0f);
            m_invInertiaTensorBody = glm::mat3(0.0f);
            return;
        }

        float ix = 0.0f, iy = 0.0f, iz = 0.0f;

        if (m_shape->GetType() == ShapeType::CUBE) {
            CubeShape* cube = static_cast<CubeShape*>(m_shape);
            float s = cube->m_halfExtent * 2.0f;
            float i = (1.0f / 6.0f) * m_mass * (s * s);
            ix = i; iy = i; iz = i;
        }
        else if (m_shape->GetType() == ShapeType::SPHERE) {
            SphereShape* sphere = static_cast<SphereShape*>(m_shape);
            float r = sphere->m_radius;
            float i = 0.4f * m_mass * (r * r);
            ix = i; iy = i; iz = i;
        }
        else {
            float i = 0.4f * m_mass * 1.0f;
            ix = i; iy = i; iz = i;
        }

        m_inertiaTensorBody = glm::mat3(
            ix, 0.0f, 0.0f,
            0.0f, iy, 0.0f,
            0.0f, 0.0f, iz
        );

        m_invInertiaTensorBody = glm::inverse(m_inertiaTensorBody);
    }

};

#endif // PHYSIQUEMODEL_HPP