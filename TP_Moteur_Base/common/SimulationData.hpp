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
};
class SimulationData {
public:
    void addRow(float t, float px, float py, float pz, float vx, float vy, float vz) {
        m_rows.push_back({t, px, py, pz, vx, vy, vz});
    }

    void saveToCSV(const std::string& filename) {
        std::ofstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }

        file << "Time,PosX,PosY,PosZ,VelX,VelY,VelZ\n";

        for (const auto& row : m_rows) {
            file << row.time << ","
                 << row.px << "," << row.py << "," << row.pz << ","
                 << row.vx << "," << row.vy << "," << row.vz << "\n";
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