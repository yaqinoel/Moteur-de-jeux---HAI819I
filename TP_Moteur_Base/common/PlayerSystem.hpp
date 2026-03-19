#ifndef PLAYERNODE_HPP
#define PLAYERNODE_HPP

#include "SceneManager.hpp"
#include "SceneNode.hpp"
#include "RenderingModel.hpp"
#include <vector>

#include "TerrainSystem.hpp"
#include "Time.hpp"

enum class Player_Movement { FORWARD, BACKWARD, LEFT, RIGHT };
const float PLAYER_SPEED = 10.0f;
const float PLAYER_HEIGHT = 2.0f;

const float VISIBLE_DISTANCE = 50.0f;

class PlayerSystem {
public:
    // 模型
    std::vector<RenderingModel*> m_lodModels;
    RenderingModel* m_playerModel = nullptr;
    float m_maxVisibleDistance;

    // 节点
    SceneNode* m_playerNode;
    SceneNode* m_visuaNode;

    // 位置属性
    glm::vec3 m_Position;
    float m_height;

    // 构造玩家系统
    PlayerSystem(SceneManager &sceneManager) {
        // 设置模型
        m_lodModels.push_back(new RenderingModel(std::string("./resources/models/bunny_lod/bunny0.off")));
        m_lodModels.push_back(new RenderingModel(std::string("./resources/models/bunny_lod/bunny1.off")));
        m_lodModels.push_back(new RenderingModel(std::string("./resources/models/bunny_lod/bunny2.off")));
        m_lodModels.push_back(new RenderingModel(std::string("./resources/models/bunny_lod/bunny3.off")));

        m_playerModel = m_lodModels[0];

        // 初始化节点
        m_playerNode = new SceneNode(nullptr);

        // 初始化位置
        m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
        m_playerNode->GetTransform().setTranslation(m_Position);
        m_height = PLAYER_HEIGHT;

        m_visuaNode = new SceneNode(m_playerModel);
        m_visuaNode->GetTransform().setScale(glm::vec3(30.0f));
        m_visuaNode->GetTransform().setRotation(glm::vec3(-90.0f, -90.0f, 0.0f));

        m_playerNode->AddChild(m_visuaNode);

        // 将玩家节点挂载到场景根节点
        sceneManager.GetRoot()->AddChild(m_playerNode);
    }

    // 析构函数
    ~PlayerSystem() {
        delete m_playerNode;
    }

    // 根据地形更新玩家高度
    void updateHeight(TerrainSystem *terrainSystem) {
        glm::vec3 playerPos = m_Position;
        float terrainHeight = terrainSystem->GetHeightAt(playerPos.x, playerPos.z);
        m_Position.y = terrainHeight + m_height;
        m_playerNode->GetTransform().setTranslation(m_Position);
    }

    // 处理键盘输入
    void ProcessKeyboard(Player_Movement direction) {
        float velocity = m_MovementSpeed * Time::DeltaTime;

        if (direction == Player_Movement::FORWARD) {
            m_Position.z -= velocity;
        }
        if (direction == Player_Movement::BACKWARD) {
            m_Position.z += velocity;
        }
        if (direction == Player_Movement::LEFT) {
            m_Position.x -= velocity;
        }
        if (direction == Player_Movement::RIGHT) {
            m_Position.x += velocity;
        }
    }

    // 动态更新 LOD
    void UpdateLOD(const glm::vec3& cameraPosition) {
        if (m_lodModels.empty() || !m_visuaNode) return;

        // 获取模型总数
        int lodCount = m_lodModels.size();

        // 计算玩家与相机的绝对距离
        float distance = glm::distance(m_Position, cameraPosition);

        // 根据总距离和模型数量，划分距离区间
        float segmentSize = VISIBLE_DISTANCE / (float)lodCount;

        // 计算当前距离对应的数组索引
        int targetIndex = static_cast<int>(distance / segmentSize);

        // 防止数组越界（如果玩家跑得比 200 还远，就强制用最粗糙的模型）
        if (targetIndex >= lodCount) {
            targetIndex = lodCount - 1;
        } else if (targetIndex < 0) {
            targetIndex = 0;
        }

        RenderingModel* targetModel = m_lodModels[targetIndex];

        if (targetModel != m_playerModel) {
            m_playerModel = targetModel;             // 更新当前记录
            m_visuaNode->SetModel(m_playerModel);    // 真正替换视觉节点的模型
            std::cout << "Switched to Bunny LOD " << targetIndex << " at distance " << distance << std::endl;
        }
    }

private:
    float m_MovementSpeed = PLAYER_SPEED;

};



#endif // PLAYERNODE_HPP