#ifndef INFINITESCENEMANAGER_HPP
#define INFINITESCENEMANAGER_HPP

#include "SceneManager.hpp"
#include "SceneNode.hpp"
#include "model.hpp"
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <tuple>
#include <vector>

class InfiniteScene {
public:

  // 构造无限场景
  InfiniteScene(SceneManager &sceneManager, Model *objectModel,
                float chunkSize = 200.0f, int renderDistance = 4)
      : m_sceneManager(sceneManager), m_objectModel(objectModel),
        m_chunkSize(chunkSize), m_renderDistance(renderDistance) {}

  // 销毁无限场景
  ~InfiniteScene() {
    for (auto &pair : m_activeChunks) {
      m_sceneManager.GetRoot()->RemoveChild(pair.second);
      delete pair.second;
    }
    m_activeChunks.clear();
  }

  // 对场景进行更新
  void Update(const glm::vec3 &cameraPosition) {
    // 根据相机位置计算所属格子
    int currentX = static_cast<int>(std::floor(cameraPosition.x / m_chunkSize));
    int currentY = static_cast<int>(std::floor(cameraPosition.y / m_chunkSize));
    int currentZ = static_cast<int>(std::floor(cameraPosition.z / m_chunkSize));

    // 计算哪些格子应该被激活
    std::vector<std::tuple<int, int, int>> neededChunks;
    for (int x = currentX - m_renderDistance; x <= currentX + m_renderDistance; ++x) {
      for (int y = currentY - m_renderDistance; y <= currentY + m_renderDistance; ++y) {
        for (int z = currentZ - m_renderDistance; z <= currentZ + m_renderDistance; ++z) {
          neededChunks.push_back({x, y, z});
        }
      }
    }

    // 移除不在范围内的格子
    auto it = m_activeChunks.begin();
    while (it != m_activeChunks.end()) {
      bool needed = false;
      for (const auto &neededChunk : neededChunks) {
        if (it->first == neededChunk) {
          needed = true;
          break;
        }
      }

      if (!needed) {
        m_sceneManager.GetRoot()->RemoveChild(it->second);
        delete it->second;
        it = m_activeChunks.erase(it);
      } else {
        ++it;
      }
    }

    // 创建新进入范围的格子
    for (const auto &neededChunk : neededChunks) {
      if (m_activeChunks.find(neededChunk) == m_activeChunks.end()) {
        m_activeChunks[neededChunk] = GenerateChunk(neededChunk);
        m_sceneManager.GetRoot()->AddChild(m_activeChunks[neededChunk]);
      }
    }

  }

private:
  SceneManager &m_sceneManager;
  Model *m_objectModel;
  float m_chunkSize;
  int m_renderDistance;

  std::map<std::tuple<int, int, int>, SceneNode *> m_activeChunks;

  // 随机生成格子内的场景
  SceneNode *GenerateChunk(const std::tuple<int, int, int> &chunkCoords) {
    SceneNode *chunkNode = new SceneNode(nullptr);

    auto [cx, cy, cz] = chunkCoords;
    float halfSize = m_chunkSize * 0.5f;
    glm::vec3 chunkCenter(cx * m_chunkSize + halfSize,
                        cy * m_chunkSize + halfSize,
                        cz * m_chunkSize + halfSize);
    chunkNode->GetTransform().setTranslation(chunkCenter);

    // 根据格子的坐标生成随机数种子
    unsigned int seed = static_cast<unsigned int>(
        cx * 73856093 ^ cy * 19349663 ^ cz * 83492791);
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> distPos(-halfSize, halfSize);
    std::uniform_real_distribution<float> distScale(0.5f, 2.0f);
    std::uniform_real_distribution<float> distAngle(0.0f, 360.0f);
    std::uniform_int_distribution<int> distCount(5, 15);
    int numObjects = distCount(generator);

    for (int i = 0; i < numObjects; ++i) {
      SceneNode *objectNode = new SceneNode(m_objectModel);
      objectNode->m_isCullable = true; // 开启裁剪

      // 随机偏移位置和缩放以及轴偏转
      objectNode->GetTransform().setTranslation({distPos(generator), distPos(generator), distPos(generator)});
      objectNode->GetTransform().setScale(glm::vec3(distScale(generator)));
      objectNode->GetTransform().setRotation({distAngle(generator), distAngle(generator), distAngle(generator)});

      chunkNode->AddChild(objectNode);
    }

    return chunkNode;
  }
};

#endif // INFINITESCENEMANAGER_HPP
