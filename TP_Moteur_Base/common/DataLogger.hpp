#ifndef DATALOGGER_HPP
#define DATALOGGER_HPP
#include "GameObjet.hpp"
#include "SimulationData.hpp"

class DataLogger {
public:
    DataLogger(const GameObjet * target) : m_target(target) {}

    void sample(float deltaTime) {
        if (m_target) {
            auto p = m_target->physicsModel->m_physicsPosition;
            auto v = m_target->physicsModel->m_velocity;
            m_data.addRow(
                current_time,
                p.x,p.y,p.z,
                v.x,v.y,v.z
            );
            current_time+= deltaTime;
        }
    }

    void exportToCSV(const std::string& filename) {
        m_data.saveToCSV(filename);
    }

private:
    const GameObjet* m_target;
    SimulationData m_data;
    float current_time = 0.0f;
};

#endif //DATALOGGER_HPP