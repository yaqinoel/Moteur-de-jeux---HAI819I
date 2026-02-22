#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
    Transform(glm::vec3 translation = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    glm::mat4 getLocalModelMatrix() const;

    // 获取属性
    glm::vec3 getTranslation() const { return m_translation; }
    glm::vec3 getRotation() const { return m_rotation; }
    glm::vec3 getScale() const { return m_scale; }

    // 设置属性
    void setTranslation(const glm::vec3& translation) { m_translation = translation; }
    void setRotation(const glm::vec3& rotation) { m_rotation = rotation; }
    void setScale(const glm::vec3& scale) { m_scale = scale; }

    // 更新属性
    void Translate(const glm::vec3& delta);
    void Rotate(const glm::vec3& delta);
    void Scale(const glm::vec3& delta);

private:
    glm::vec3 m_translation;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
};

#endif // TRANSFORM_HPP