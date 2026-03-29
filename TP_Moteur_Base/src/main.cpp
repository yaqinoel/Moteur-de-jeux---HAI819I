// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include "common/CubeObjet.hpp"
#include "common/DataLogger.hpp"
#include "common/GameObjet.hpp"
#include "common/PlayerSystem.hpp"
#include "common/TerrainSystem.hpp"

// Include IMGUI
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "common/GUIManager.hpp"

GLFWwindow *window;

// Include GLM
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>

#include <fstream>
using namespace glm;

#include "common/Time.hpp"
#include "common/camera.hpp"
#include "common/RenderingModel.hpp"
#include "common/shader.hpp"
#include <common/shader.hpp>

// Scene Graph Includes
#include "common/Cone.hpp"
#include "common/InfiniteScene.hpp"
#include "common/SceneManager.hpp"
#include "common/SceneNode.hpp"
#include "common/SolarSystem.hpp"
#include "common/Transform.hpp"
#include "common/GameManager.hpp"

void processPlayerInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouseClick_callback(GLFWwindow *window, int button, int action, int mods);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

// 窗口设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
std::string windowTitle = "Moteur de jeux";

bool isWireframe = false;

// 相机初始化参数
Camera camera(glm::vec3(8.f, 5.f, 25.f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseClickDown = false;

// 输入管理器
InputManager inputManager;

GameManager gameManager;
TerrainSystem* terrainSystem = nullptr;
CubeObjet* fallingCube = nullptr;

// 日志记录器
DataLogger* dataLogger = nullptr;

// GUI管理器
GUIManager * guiManager = nullptr;

int main(void) {
  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, windowTitle.c_str(), NULL, NULL);
  if (window == NULL) {
    fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    getchar();
    glfwTerminate();
    return -1;
  }

  // 创建窗口上下文
  glfwMakeContextCurrent(window);

  // 注册回调函数
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mouseClick_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return -1;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  glfwPollEvents();
  glfwSetCursorPos(window, 1024 / 2, 768 / 2);

  glClearColor(0.1f, 0.1f, 0.1f, 0.0f); // Space color

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Init ImGUI
  guiManager = new GUIManager();
  guiManager->initImgui(window);

  camera.SetupInput(inputManager);
  inputManager.SetContext("FreeMode");

  // 创建并编译Shader
  Shader terrainShader("./resources/shaders/vertex_shader_terrain.glsl",
                       "./resources/shaders/fragment_shader_terrain.glsl");
  Shader objetShader("./resources/shaders/vertex_shader_bunny.glsl",
                       "./resources/shaders/fragment_shader_bunny.glsl");

  // 初始化地形系统
  terrainSystem = new TerrainSystem(gameManager.sceneManager, &terrainShader,16);
  fallingCube = new CubeObjet(gameManager.sceneManager, &objetShader, 0.5f, 1.0f, glm::vec3(2.0f, 7.5f, -3.0f));
  gameManager.AddStaticGameObject(terrainSystem);
  gameManager.AddDynamicGameObject(fallingCube);

  // fallingCube->SetVelocity(5.0f * glm::normalize((-camera.m_Right + glm::vec3(0.0f, 1.0f, 0.0f))));
  dataLogger = new DataLogger(fallingCube);
  gameManager.setDataLogger(dataLogger);

  guiManager->setTarget(fallingCube, terrainSystem, &camera);


  Time::intialize();

  do {
    Time::Update();
    float dt = Time::DeltaTime;

    inputManager.Update(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    windowTitle =
        "TP5 - Physique Cube | FPS: " + std::to_string(Time::FPS);
    glfwSetWindowTitle(window, windowTitle.c_str());

    guiManager->draw();

    // 更新场景
    gameManager.Update(dt);

    // 更新相机
    if (camera.m_IsOrbital) {
      camera.UpdateOrbital(Time::DeltaTime);
    } else if (camera.m_IsFollowing) {
      camera.UpdateFollow(Time::DeltaTime);
    }

    // 获得View和Projection矩阵
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectiveMatrix();
    Cone cone;

    objetShader.setVec3("viewPos", camera.m_Position);

    gameManager.Draw(view, projection, cone);

    // Renders the ImGUI elements
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();

  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glDeleteProgram(terrainShader.m_ID);
  glDeleteProgram(objetShader.m_ID);

  delete terrainSystem;
  delete fallingCube;

  glfwTerminate();

  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  camera.m_Aspect = (float)width / (float)height;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  if (ImGui::GetCurrentContext() != nullptr && !ImGui::GetIO().WantCaptureMouse) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
  }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  if (ImGui::GetCurrentContext() != nullptr && !ImGui::GetIO().WantCaptureMouse) {
    if (mouseClickDown) {
      camera.ProcessMouseMovement(xoffset, yoffset);
    }
  }
}

void mouseClick_callback(GLFWwindow *window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS) {
      mouseClickDown = true;
      lastX = x;
      lastY = y;
    } else {
      mouseClickDown = false;
    }
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_L) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      isWireframe = !isWireframe;
      if (isWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    }

    if (key == GLFW_KEY_C) {
      if (camera.m_IsOrbital) {
        camera.DisableOrbitalMode();
        inputManager.SetContext("FreeMode");
      } else {
        camera.DisableFollowMode();
        camera.EnableOrbitalMode(terrainSystem->terrainNode, 2.5f, 45.0f);
        inputManager.SetContext("OrbitalMode");
      }
    }

    if (key == GLFW_KEY_SPACE) {
      gameManager.SwitchSimulationStatus();
    }

    if (key == GLFW_KEY_R) {
      fallingCube->ResetStatus();
      // fallingCube->SetVelocity(5.0f * glm::normalize((-camera.m_Right + glm::vec3(0.0f, 1.0f, 0.0f))));
      gameManager.StopSimulationStatus();
    }

    // 保存模拟数据
    if (key == GLFW_KEY_K) {
      dataLogger->exportToCSV("./outputs/simulationData/data.csv");
    }

  }
}


