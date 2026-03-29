#ifndef GUIMANAGER_HPP
#define GUIMANAGER_HPP

#include "camera.hpp"
#include "CubeObjet.hpp"
#include "TerrainSystem.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class GUIManager {
public:

    void initImgui(GLFWwindow *window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void setTarget(CubeObjet* cube, TerrainSystem* terrain, Camera* camera) {
        m_cube = cube;
        m_terrain = terrain;
        m_camera = camera;
    }

    void draw() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Global Debug Console");

        // 必须先开启 TabBar
        if (ImGui::BeginTabBar("MyTabBar")) {

            if (m_camera && ImGui::BeginTabItem("Camera")) {
                DrawCameraGUI(*m_camera); // 传入引用
                ImGui::EndTabItem();
            }

            if (m_cube && ImGui::BeginTabItem("Cube")) {
                DrawCubeGUI(*m_cube);
                ImGui::EndTabItem();
            }

            if (m_terrain && ImGui::BeginTabItem("Terrain")) {
                DrawTerrainGUI(*m_terrain);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void DrawCameraGUI(Camera& camera) {

        // 空间位置
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", &camera.m_Position.x, 0.1f);
            ImGui::TextDisabled("Front: %.2f, %.2f, %.2f", camera.m_Front.x, camera.m_Front.y, camera.m_Front.z);
        }

        // 旋转控制 (欧拉角)
        if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Yaw 通常是 0-360，Pitch 限制在 -89 到 89 防止万向锁翻转
            if (ImGui::SliderFloat("Yaw", &camera.m_Yaw, -180.0f, 180.0f));
            if (ImGui::SliderFloat("Pitch", &camera.m_Pitch, -89.0f, 89.0f));

            ImGui::Separator();
            ImGui::Text("Orientation (Quaternion):");
            ImGui::TextDisabled("W:%.2f X:%.2f Y:%.2f Z:%.2f",
                camera.m_Orientation.w, camera.m_Orientation.x, camera.m_Orientation.y, camera.m_Orientation.z);
        }

        // 镜头参数 (Lens / Projection)
        if (ImGui::CollapsingHeader("Lens Settings")) {
            ImGui::SliderFloat("FOV (Zoom)", &camera.m_Zoom, 1.0f, 90.0f);
            ImGui::DragFloat("Near Plane", &camera.m_ZNear, 0.01f, 0.001f, 1.0f);
            ImGui::DragFloat("Far Plane", &camera.m_ZFar, 1.0f, 10.0f, 10000.0f);
        }

        // 控制感官
        if (ImGui::CollapsingHeader("Movement Settings")) {
            ImGui::DragFloat("Move Speed", &camera.m_MovementSpeed, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Sensitivity", &camera.m_MouseSensitivity, 0.001f, 0.0f, 1.0f);
        }

        // 快速操作
        ImGui::Separator();
        if (ImGui::Button("Reset to Origin")) {
            camera.m_Position = glm::vec3(0.0f, 5.0f, 10.0f);
            camera.m_Yaw = -90.0f;
            camera.m_Pitch = 0.0f;
        }

    }

    void DrawCubeGUI(CubeObjet& cube) {
        ImGui::PushID(&cube);

        ImGui::Text("GameObject: Cube");
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& transform = cube.sceneNode->GetTransform();

            // 位置同步
            glm::vec3 pos = transform.getTranslation();
            if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                transform.setTranslation(pos);
                cube.physicsModel->m_physicsPosition = pos;
            }

            // 旋转同步
            glm::vec3 euler = transform.getRotation();
            if (ImGui::DragFloat3("Rotation", &euler.x, 1.0f, -180.0f, 180.0f)) {
                transform.setRotation(euler);
            }

            // 缩放同步
            glm::vec3 scale = transform.getScale();
            if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
                transform.setScale(scale);
                cube.m_halfWidth = scale.x;
            }
        }

        if (ImGui::CollapsingHeader("Physics Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            // 质量
            ImGui::SliderFloat("Mass", &cube.m_mass, 0.1f, 50.0f, "%.1f kg");
            cube.physicsModel->m_mass = cube.m_mass;

            // 实时速度监控与修改
            glm::vec3 vel = cube.physicsModel->m_velocity;
            if (ImGui::DragFloat3("Velocity", &vel.x, 0.1f)) {
                cube.SetVelocity(vel);
            }
        }

        if (ImGui::CollapsingHeader("Status & Reset")) {
            // 编辑初始位置
            ImGui::DragFloat3("Start Position", &cube.m_startPosition.x, 0.1f);

            ImGui::Spacing();
            if (ImGui::Button("Reset Status", ImVec2(-FLT_MIN, 0))) {
                cube.ResetStatus();
                cube.SetVelocity(glm::vec3(0.0f));
                cube.SyncTransform();
            }
        }

        ImGui::PopID();
    }

    void DrawTerrainGUI(TerrainSystem& terrain) {
        ImGui::PushID(&terrain);
        ImGui::Text("GameObject: Terrain System");
        ImGui::Separator();

        // 几何与分辨率
        if (ImGui::CollapsingHeader("Geometry Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            static int tempRes = terrain.resolution;
            ImGui::InputInt("Resolution", &tempRes);
            if (ImGui::Button("Apply New Resolution")) {
                if (tempRes > 1 && tempRes < 512) {
                    terrain.updateResolution(tempRes);
                }
            }

            if (ImGui::DragFloat("Height Scale", &terrain.heightScale, 0.1f, 0.0f, 10.0f)) {
                terrain.terrainNode->GetTransform().setScale(
                    glm::vec3(50.0f, 15.0f * terrain.heightScale, 50.0f)
                );
            }
        }

        // 变换属性
        if (ImGui::CollapsingHeader("World Transform")) {
            auto& transform = terrain.terrainNode->GetTransform();

            // 地形位置
            glm::vec3 pos = transform.getTranslation();
            if (ImGui::DragFloat3("Position", &pos.x, 0.5f)) {
                transform.setTranslation(pos);
                terrain.physicsModel->m_physicsPosition = pos;
            }

            glm::vec3 scale = transform.getScale();
            if (ImGui::DragFloat3("Global Scale", &scale.x, 0.5f, 0.1f, 500.0f)) {
                transform.setScale(scale);
            }
        }

        // 可视化辅助
        if (ImGui::CollapsingHeader("Visualization")) {
            static bool wireframe = false;
            if (ImGui::Checkbox("Wireframe Mode", &wireframe)) {
                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            }
        }

        ImGui::PopID();
    }

private:
    CubeObjet* m_cube = nullptr;
    TerrainSystem* m_terrain = nullptr;
    Camera* m_camera = nullptr;
};


#endif //GUIMANAGER_HPP