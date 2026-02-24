#ifndef CONE_HPP
#define CONE_HPP

#include <cmath>
#include <glm/glm.hpp>

struct Cone {
  glm::vec3 apex;
  glm::vec3 direction;
  float angleCos;
  float length;

  // 简单的点在圆锥内测试
  bool containsPoint(const glm::vec3 &point) const {
    glm::vec3 toPoint = point - apex;
    float dist = glm::length(toPoint);

    // 超过渲染距离ZFAR
    if (dist > length) {
      return false;
    }

    // 如果靠得很近，直接算在视野内避免除以0或奇怪的浮点截断
    if (dist < 0.1f) {
      return true;
    }

    toPoint /= dist;

    // 检查方向的点积
    float dotProduct = glm::dot(direction, toPoint);

    // 如果夹角大于圆锥的阈值，说明在外边
    return dotProduct >= angleCos;
  }
};

#endif // CONE_HPP
