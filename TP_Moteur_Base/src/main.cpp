// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

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
#include "common/model.hpp"
#include "common/shader.hpp"
#include <common/shader.hpp>


// Scene Graph Includes
#include "common/SceneManager.hpp"
#include "common/SceneNode.hpp"
#include "common/Transform.hpp"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouseClick_callback(GLFWwindow *window, int button, int action, int mods);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

// 窗口设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
std::string windowTitle = "TP3 - Scene Graph - Solar System";

bool isWireframe = false;

// 相机初始化参数
Camera
    camera(glm::vec3(0.0f, 20.f,
                     20.f)); // Adjusted camera for better view of solar system
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseClickDown = false;

SceneManager sceneManager;

SceneNode *sunMeshNode;

SceneNode *earthOrbitNode;
SceneNode *earthPositionNode;
SceneNode *earthTiltNode;
SceneNode *earthMeshNode;

SceneNode *moonOrbitNode;
SceneNode *moonPositionNode;
SceneNode *moonMeshNode;

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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, windowTitle.c_str(), NULL,
                            NULL);
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

  // 创建并编译Shader
  Shader planeteShader("./resources/shaders/vertex_shader_planete.glsl",
                       "./resources/shaders/fragment_shader_planete.glsl");

  // 加载模型
  Model *sunModel = new Model(string("./resources/models/planete/sun.obj"));
  Model *earthModel = new Model(string("./resources/models/planete/earth.obj"));
  Model *moonModel = new Model(string("./resources/models/planete/moon.obj"));

  // 设置场景图
  // 根节点
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

  Time::intialize();

  do {
    Time::Update();

    windowTitle = "TP3 - Scene Graph - Solar System | FPS: " + std::to_string(Time::FPS);
    glfwSetWindowTitle(window, windowTitle.c_str());

    if (camera.m_IsOrbital) {
      camera.UpdateOrbital(Time::DeltaTime);
    } else {
      processInput(window);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    planeteShader.use();

    float dt = Time::DeltaTime;

    // 太阳自转
    sunMeshNode->GetTransform().Rotate(glm::vec3(0.0f, 10.0f * dt, 0.0f));

    // 地球公转
    earthOrbitNode->GetTransform().Rotate(
        glm::vec3(0.0f, 50.0f * dt, 0.0f));

    // 地球自转
    earthMeshNode->GetTransform().Rotate(
        glm::vec3(0.0f, 20.0f * dt, 0.0f));

    // 月球公转
    moonOrbitNode->GetTransform().Rotate(
        glm::vec3(0.0f, 50.0f * dt, 0.0f));

    // 更新场景
    sceneManager.Update();

    // 绘制场景
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    glm::mat4 projectionMatrix = camera.GetProjectiveMatrix();
    sceneManager.Draw(planeteShader, viewMatrix, projectionMatrix);

    glfwSwapBuffers(window);
    glfwPollEvents();

  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glDeleteProgram(planeteShader.m_ID);

  delete sunModel;
  delete earthModel;
  delete moonModel;

  glfwTerminate();

  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT);
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    camera.ProcessKeyboard(UP);
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    camera.ProcessKeyboard(DOWN);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  camera.m_Aspect = (float)width / (float)height;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  if (mouseClickDown) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
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

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
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
      } else {
        camera.EnableOrbitalMode(glm::vec3(0.0f, 0.0f, 0.0f), 9.0f, 45.0f);
      }
    }

    if (key == GLFW_KEY_UP) {
      camera.ChangeOrbitalSpeed(0.1f);
    }
    if (key == GLFW_KEY_DOWN) {
      camera.ChangeOrbitalSpeed(-0.1f);
    }
  }
}
