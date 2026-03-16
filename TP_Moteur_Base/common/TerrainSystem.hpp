#ifndef TERRAINSYSTEM_HPP
#define TERRAINSYSTEM_HPP

#include "SceneManager.hpp"
#include "SceneNode.hpp"
#include "model.hpp"
#include <vector>
#include <random>

class TerrainSystem {
public:
    Model* terrainModel;
    SceneNode* terrainNode;

    int resolution;
    float heightScale = 1.0f;

    std::vector<Texture> m_textures;
    std::vector<unsigned char> m_heightmapData;
    int m_hmWidth = 0, m_hmHeight = 0;

    // 构造函数
    TerrainSystem(SceneManager& sceneManager, int initialResolution = 16) {
        resolution = initialResolution;
        terrainModel = new Model();

        // 加载纹理和图片数据
        LoadTexturesAndHeightmap();

        // 统一构建Mesh
        Mesh terrainMesh = BuildTerrainMesh(resolution);
        terrainModel->meshes.push_back(terrainMesh);

        // 挂在到场景树
        terrainNode = new SceneNode(terrainModel);
        terrainNode->GetTransform().setScale(glm::vec3(100.0f));
        terrainNode->GetTransform().setTranslation(glm::vec3(0.0f, -30.0f, 0.0f));

        sceneManager.GetRoot()->AddChild(terrainNode);
    }

    // 析构函数
    ~TerrainSystem() {
        delete terrainNode;
    }

    Mesh BuildTerrainMesh(int res) {
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

                // 直接在几何生成阶段，从缓存的高度图数据中读取并计算真实高度！
                float y = GetHeightFromImage(u, v);

                vertex.Position = glm::vec3(x, y, z);
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
    Model* generatePlane(int nbX, int nbY) {
        Model* terrainModel = new Model();

        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 手动创建纹理
        Texture texturesGrass;
        texturesGrass.id = TextureFromFile("grass.png","./resources/textures/");
        texturesGrass.type = "texture_diffuse";
        textures.push_back(texturesGrass);

        Texture texturesRock;
        texturesRock.id = TextureFromFile("rock.png","./resources/textures/");
        texturesRock.type = "texture_diffuse";
        textures.push_back(texturesRock);

        Texture texturesSnow;
        texturesSnow.id = TextureFromFile("snowrocks.png","./resources/textures/");
        texturesSnow.type = "texture_diffuse";
        textures.push_back(texturesSnow);

        Texture texturesHeight;
        texturesHeight.id = TextureFromFile("Heightmap_Mountain.png","./resources/textures/");
        texturesHeight.type = "texture_height";
        textures.push_back(texturesHeight);

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
        Mesh terrainMesh = Mesh(vertices, indices, textures);
        terrainModel->meshes.push_back(terrainMesh);

        return terrainModel;
    }

    void updateResolution(int newResolution) {
        resolution = newResolution;

        // 清除旧GPU缓冲和数据
        if (!terrainModel->meshes.empty()) {
            terrainModel->meshes[0].clearBuffers();
            terrainModel->meshes.clear();
        }

        Mesh newMesh = BuildTerrainMesh(resolution);
        terrainModel->meshes.push_back(newMesh);
    }

    float barycentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {
        float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
        float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
        float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
        float l3 = 1.0f - l1 - l2;
        return l1 * p1.y + l2 * p2.y + l3 * p3.y;
    }

    float GetHeightAt(float worldX, float worldZ) {
        if (terrainModel->meshes.empty()) return 0.0f;
        Mesh& mesh = terrainModel->meshes[0];

        glm::vec3 scale = terrainNode->GetTransform().getScale();
        glm::vec3 pos = terrainNode->GetTransform().getTranslation();

        float localX = (worldX - pos.x) / scale.x;
        float localZ = (worldZ - pos.z) / scale.z;

        float gridCoordX = (localX + 1.0f) / 2.0f * (resolution - 1);
        float gridCoordZ = (localZ + 1.0f) / 2.0f * (resolution - 1);

        int gridX = (int)std::floor(gridCoordX);
        int gridZ = (int)std::floor(gridCoordZ);

        if (gridX < 0 || gridX >= resolution - 1 || gridZ < 0 || gridZ >= resolution - 1) {
            return pos.y;
        }

        float tx = gridCoordX - gridX;
        float tz = gridCoordZ - gridZ;

        int v00 = gridX * resolution + gridZ;
        int v10 = (gridX + 1) * resolution + gridZ;
        int v01 = gridX * resolution + (gridZ + 1);
        int v11 = (gridX + 1) * resolution + (gridZ + 1);

        glm::vec3 p00 = mesh.vertices[v00].Position * scale + pos;
        glm::vec3 p10 = mesh.vertices[v10].Position * scale + pos;
        glm::vec3 p01 = mesh.vertices[v01].Position * scale + pos;
        glm::vec3 p11 = mesh.vertices[v11].Position * scale + pos;

        if (tx <= tz) {
            return barycentric(p00, p01, p11, glm::vec2(worldX, worldZ));
        } else {
            return barycentric(p00, p10, p11, glm::vec2(worldX, worldZ));
        }
    }


private:
    void LoadTexturesAndHeightmap() {
        Texture tGrass;
        tGrass.id = TextureFromFile("grass.png","./resources/textures/");
        tGrass.type = "texture_diffuse"; m_textures.push_back(tGrass);
        Texture tRock;
        tRock.id = TextureFromFile("rock.png","./resources/textures/");
        tRock.type = "texture_diffuse"; m_textures.push_back(tRock);
        Texture tSnow;
        tSnow.id = TextureFromFile("snowrocks.png","./resources/textures/");
        tSnow.type = "texture_diffuse"; m_textures.push_back(tSnow);
        Texture tHeight;
        tHeight.id = TextureFromFile("Heightmap_Mountain.png","./resources/textures/");
        tHeight.type = "texture_height"; m_textures.push_back(tHeight);

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

    float GetHeightFromImage(float u, float v) {
        if (m_heightmapData.empty()) return 0.0f;
        int px = std::max(0, std::min(m_hmWidth - 1, (int)(u * m_hmWidth)));
        int py = std::max(0, std::min(m_hmHeight - 1, (int)(v * m_hmHeight)));

        unsigned char pixelValue = m_heightmapData[py * m_hmWidth + px];
        return (pixelValue / 255.0f) * heightScale - 0.5;
    }
};

#endif TERRAINSYSTEM_HPP