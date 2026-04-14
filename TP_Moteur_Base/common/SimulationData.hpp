#ifndef CSVDATA_HPP
#define CSVDATA_HPP

#include <fstream>
#include <iostream>
#include <vector>
#include <utility>  // std::pair
#include <string>

struct DataRow {
    float time;
    float px, py, pz;
    float vx, vy, vz;
    float px_a, py_a, pz_a;
    float vx_a, vy_a, vz_a;
};
class SimulationData {
public:
    void addRow(float t, glm::vec3 pos, glm::vec3 vel, glm::vec3 pos_a, glm::vec3 vel_a) {
        m_rows.push_back({t,
            pos.x, pos.y, pos.z,
            vel.x, vel.y, vel.z,
            pos_a.x, pos_a.y, pos_a.z,
            vel_a.x, vel_a.y, vel_a.z
        });
    }

    void saveToCSV(const std::string& filename) {
        std::ofstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }

        file << "Time,PosX,PosY,PosZ,VelX,VelY,VelZ,PosX_A,PosY_A,PosZ_A,VelX_A,VelY_A,VelZ_A\n";

        for (const auto& row : m_rows) {
            file << row.time << ","
                 << row.px << "," << row.py << "," << row.pz << ","
                 << row.vx << "," << row.vy << "," << row.vz << ","
                 << row.px_a << "," << row.py_a << "," << row.pz_a << ","
                 << row.vx_a << "," << row.vy_a << "," << row.vz_a << "\n";
        }

        file.close();
        std::cout << "3D Data saved to " << filename << std::endl;
    }

    void clear() {
        m_rows.clear();
    }

private:
    std::vector<DataRow> m_rows;
};


#endif //CSVDATA_HPP