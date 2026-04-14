#ifndef DATALOGGER_HPP
#define DATALOGGER_HPP
#include "GameObjet.hpp"
#include "SimulationData.hpp"

const glm::vec3 GRAVITY = glm::vec3(0.0f, -9.81f, 0.0f);
float k = 1.f;       // 刚度
float m = 10.f; // 质量
float P_y = 0.0f;    // 固定点 P 的高度
float l0 = 10.0f;

class DataLogger {
public:
    DataLogger(const GameObjet * target) : m_target(target) {
        if (target != NULL) {
            startPos = target->physicsModel->m_physicsPosition;
            startVel = target->physicsModel->m_velocity;
        }
    }

    void sample(float deltaTime) {
        if (m_target) {
            auto p = m_target->physicsModel->m_physicsPosition;
            auto v = m_target->physicsModel->m_velocity;
            glm::vec3 p_a = getAnalytiquePosition();
            glm::vec3 v_a =  getAnalytiqueVelocity();
            m_data.addRow(
                current_time,
                p,v,p_a,v_a
            );
            current_time+= deltaTime;
        }
    }

    void exportToCSV(const std::string& filename) {
        m_data.saveToCSV(filename);
    }

    glm::vec3 getAnalytiqueVelocity() {
        glm::vec3 va(0.f);
        return va = startVel + current_time * GRAVITY;
    }

    glm::vec3 getAnalytiquePosition() {
        // glm::vec3 pa(0.f);
        // return pa = startPos + startVel * current_time + (1/2.f) * GRAVITY * current_time * current_time;
        float omega = std::sqrt(k / m);
        float p_eq = P_y - l0;
        float A = startPos.y - p_eq;
        float current_y = p_eq + A * std::cos(omega * current_time);
        glm::vec3 pa(startPos.x, current_y, startPos.z);
        return pa;
    }

private:
    const GameObjet* m_target;
    glm::vec3 startPos;
    glm::vec3 startVel;
    SimulationData m_data;
    float current_time = 0.0f;
};

#endif //DATALOGGER_HPP