#include "Transform.hpp"

Transform::Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    : m_translation(translation), m_rotation(rotation), m_scale(scale) {}

glm::mat4 Transform::getLocalModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, m_translation);

    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));

    model = glm::scale(model, m_scale);

    return model;
}



void Transform::Translate(const glm::vec3 &delta) { m_translation += delta; }

void Transform::Rotate(const glm::vec3 &delta) { m_rotation += delta; }

void Transform::Scale(const glm::vec3 &delta) { m_scale *= delta; }
