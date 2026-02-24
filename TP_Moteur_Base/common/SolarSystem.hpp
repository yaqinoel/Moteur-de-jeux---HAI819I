#pragma once

#include "common/SceneManager.hpp"
#include "common/SceneNode.hpp"
#include "common/Transform.hpp"
#include "common/model.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class SolarSystem {
public:
  // 模型
  Model *sunModel;
  Model *earthModel;
  Model *moonModel;

  // 太阳面模型节点
  SceneNode *sunMeshNode;

  // 地球相关节点
  SceneNode *earthOrbitNode;
  SceneNode *earthPositionNode;
  SceneNode *earthTiltNode;
  SceneNode *earthMeshNode;

  // 月球相关节点
  SceneNode *moonOrbitNode;
  SceneNode *moonPositionNode;
  SceneNode *moonMeshNode;

  // 构造太阳系系统
  SolarSystem(SceneManager &sceneManager) {
    sunModel = new Model(std::string("./resources/models/planete/sun.obj"));
    earthModel = new Model(std::string("./resources/models/planete/earth.obj"));
    moonModel = new Model(std::string("./resources/models/planete/moon.obj"));

    SceneNode *root = sceneManager.GetRoot();

    // 太阳节点
    sunMeshNode = new SceneNode(sunModel);
    sunMeshNode->GetTransform().setScale(glm::vec3(1.0f));
    root->AddChild(sunMeshNode);

    // 地球轨道节点
    earthOrbitNode = new SceneNode(nullptr);
    root->AddChild(earthOrbitNode);

    // 设置地球轨道节点
    earthPositionNode = new SceneNode(nullptr);
    earthPositionNode->GetTransform().setTranslation(glm::vec3(10.0f, 0.0f, 0.0f));
    earthOrbitNode->AddChild(earthPositionNode);

    // 设置地球偏转轴节点
    earthTiltNode = new SceneNode(nullptr);
    earthTiltNode->GetTransform().setRotation(glm::vec3(23.0f, 0.0f, 0.0f));
    earthPositionNode->AddChild(earthTiltNode);

    // 设置地球面节点
    earthMeshNode = new SceneNode(earthModel);
    earthMeshNode->GetTransform().setScale(glm::vec3(0.5f));
    earthTiltNode->AddChild(earthMeshNode);

    // 月球轨道节点
    moonOrbitNode = new SceneNode(nullptr);
    earthPositionNode->AddChild(moonOrbitNode);

    // 月球位置节点
    moonPositionNode = new SceneNode(nullptr);
    moonPositionNode->GetTransform().setTranslation(glm::vec3(3.0f, 0.0f, 0.0f));
    moonOrbitNode->AddChild(moonPositionNode);

    // 月球面节点
    moonMeshNode = new SceneNode(moonModel);
    moonMeshNode->GetTransform().setScale(glm::vec3(0.3f));
    moonPositionNode->AddChild(moonMeshNode);
  }

  void update(float dt) {
    // 太阳自转
    sunMeshNode->GetTransform().Rotate(glm::vec3(0.0f, 10.0f * dt, 0.0f));

    // 地球公转
    earthOrbitNode->GetTransform().Rotate(glm::vec3(0.0f, 50.0f * dt, 0.0f));

    // 地球自转
    earthMeshNode->GetTransform().Rotate(glm::vec3(0.0f, 20.0f * dt, 0.0f));

    // 月球公转
    moonOrbitNode->GetTransform().Rotate(glm::vec3(0.0f, 50.0f * dt, 0.0f));
  }

  ~SolarSystem() {
    // 模型清理
    delete sunModel;
    delete earthModel;
    delete moonModel;
  }
};
