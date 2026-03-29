#ifndef COLLIDERSHAPE_HPP
#define COLLIDERSHAPE_HPP
#include "glm/detail/type_vec.hpp"

enum class ShapeType {
    CUBE,
    BOX,
    SPHERE,
    PLAN,
    TERRAIN
};

class ColliderShape {
public:
    ShapeType m_type;

    ColliderShape(ShapeType type) : m_type(type) {}

    virtual ~ColliderShape() = default;

    ShapeType GetType() const { return m_type; }

};

class CubeShape : public ColliderShape {
public:
    float m_halfExtent;

    CubeShape(const float& halfExtents)
        : ColliderShape(ShapeType::CUBE), m_halfExtent(halfExtents) {}

    float GetHalfExtent() const { return m_halfExtent; }
};

class BoxShape : public ColliderShape {
public:
    glm::vec3 m_halfExtents;

    BoxShape(const glm::vec3& halfExtents)
        : ColliderShape(ShapeType::BOX), m_halfExtents(halfExtents) {}

    glm::vec3 GetHalfExtents() const { return m_halfExtents; }
};

class PlanShape : public ColliderShape {
public:
    float m_halfExtents;

    PlanShape(const float& halfExtents)
        : ColliderShape(ShapeType::PLAN), m_halfExtents(halfExtents) {}

    float GetHalfExtents() const { return m_halfExtents; }
};



#endif //COLLIDERSHAPE_HPP