#ifndef WATERSYSTEM_HPP
#define WATERSYSTEM_HPP

#include "SceneManager.hpp"
#include "SceneNode.hpp"
#include "RenderingModel.hpp"
#include <vector>
#include <random>

#include "ColliderShape.hpp"
#include "GameObjet.hpp"

class WaterSystem : public GameObjet{
public:
    RenderingModel* waterModel;
    SceneNode* waterNode;

    int resolution;
    float heightScale = 1.0f;

    std::vector<Texture> m_textures;
    std::vector<unsigned char> m_heightmapData;
    int m_hmWidth = 0, m_hmHeight = 0;

    float m_water_level = 0.f;
    float p_water = 1000;
    float p_air = 1;

    // 构造函数
    WaterSystem(SceneManager& sceneManager, Shader* shader, int initialResolution = 16) : GameObjet() {
        resolution = initialResolution;
        waterModel = new RenderingModel();
        waterModel->shader = shader;

        // 加载纹理和图片数据
        LoadTexturesAndHeightmap();

        // 统一构建Mesh
        Mesh waterMesh = BuildWaterMesh(resolution);
        waterModel->meshes.push_back(waterMesh);

        // 挂在到场景树
        waterNode = new SceneNode(waterModel);
        waterNode->GetTransform().setScale(glm::vec3(50.0f,20.f,50.0f));
        waterNode->GetTransform().setTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
        sceneManager.GetRoot()->AddChild(waterNode);

        PlanShape* plan_shape = new PlanShape(1.f);
        physicsModel = new PhysicsModel(plan_shape, 0.0f, waterNode->GetTransform().getTranslation());

        SyncTransform();
    }

    // 析构函数
    ~WaterSystem() {
        delete waterNode;
    }

    Mesh BuildWaterMesh(int res) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // 生成顶点
        for (int i = 0; i < res; i++) {
            for (int j = 0; j < res; j++) {
                Vertex vertex;
                // 计算标准化坐标和 UV 坐标
                float x = -1.0f + (float)i * (2.0f / (float)(res - 1));
                float z = -1.0f + (float)j * (2.0f / (float)(res - 1));
                float u = (float)i / (float)(res - 1);
                float v = (float)j / (float)(res - 1);

                vertex.Position = glm::vec3(x, 0.f, z);
                vertex.TexCoords = glm::vec2(u, v);
                vertices.push_back(vertex);
            }
        }

        // 创建三角形索引
        for (int i = 0; i < res - 1; i++) {
            for (int j = 0; j < res - 1; j++) {
                int v_ij = i * res + j;
                int v_i1_j = (i + 1) * res + j;
                int v_ij1 = i * res + (j + 1);
                int v_i1_j1 = (i + 1) * res + (j + 1);

                // 左下三角形
                indices.push_back(v_ij);
                indices.push_back(v_i1_j1);
                indices.push_back(v_ij1);

                // 右上三角形
                indices.push_back(v_ij);
                indices.push_back(v_i1_j);
                indices.push_back(v_i1_j1);
            }
        }

        // 使用缓存的纹理数据创建 Mesh
        return Mesh(vertices, indices, m_textures);
    }

    // 生成地形
    RenderingModel* generatePlane(int nbX, int nbY) {
        RenderingModel* waterModel = new RenderingModel();

        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 手动创建纹理
        Texture texturesWater;
        texturesWater.id = TextureFromFile("water.jpg","./resources/textures/");
        texturesWater.type = "texture_diffuse";
        textures.push_back(texturesWater);

        // 生成顶点
        for (int i = 0; i < nbX; i++) {
            for (int j = 0; j < nbY; j++) {
                Vertex vertex;
                // 计算顶点位置
                float x = -1.0f + (float)i * (2.0f / (float)(nbX - 1));
                float z = -1.0f + (float)j * (2.0f / (float)(nbY - 1));
                float u = (float)i / (float)(nbX - 1);
                float v = (float)j / (float)(nbY - 1);

                vertex.Position = glm::vec3(x, 0.f, z);
                vertex.TexCoords = glm::vec2(u, v);
                vertices.push_back(vertex);
            }
        }

        // 创建三角形索引
        for (int i = 0; i < nbX -1; i++) {
            for (int j = 0; j < nbY -1; j++) {
                int vertice_i_j = i * nbY +j;
                int vertice_i1_j = (i+1) * nbY +j;
                int vertice_i_j1 = i * nbY + (j+1);
                int vertice_i1_j1 = (i+1) * nbY + (j+1);

                indices.push_back(vertice_i_j);
                indices.push_back(vertice_i1_j1);
                indices.push_back(vertice_i_j1);

                indices.push_back(vertice_i_j);
                indices.push_back(vertice_i1_j);
                indices.push_back(vertice_i1_j1);
            }
        }

        // 将顶点和三角形加入模型
        Mesh waterMesh = Mesh(vertices, indices, textures);
        waterModel->meshes.push_back(waterMesh);

        return waterModel;
    }

    void updateResolution(int newResolution) {
        resolution = newResolution;

        // 清除旧GPU缓冲和数据
        if (!waterModel->meshes.empty()) {
            waterModel->meshes[0].clearBuffers();
            waterModel->meshes.clear();
        }

        Mesh newMesh = BuildWaterMesh(resolution);
        waterModel->meshes.push_back(newMesh);
    }


private:
    void LoadTexturesAndHeightmap() {
        Texture tWater;
        tWater.id = TextureFromFile("water.jpg","./resources/textures/");
        tWater.type = "texture_diffuse"; m_textures.push_back(tWater);

        int nrChannels;
        unsigned char* data = stbi_load("./resources/textures/Heightmap_Mountain.png", &m_hmWidth, &m_hmHeight, &nrChannels, 1); // 强制单通道
        if (data) {
            m_heightmapData.assign(data, data + m_hmWidth * m_hmHeight);
            stbi_image_free(data);
            std::cout << "Heightmap loaded to CPU memory successfully!" << std::endl;
        } else {
            std::cout << "ERROR: Failed to load heightmap to CPU memory!" << std::endl;
        }
    }

};

#endif // WATERSYSTEM_HPP