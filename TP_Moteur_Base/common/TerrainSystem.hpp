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

    // 构造函数
    TerrainSystem(SceneManager& sceneManager, int initialResolution = 16) {
        resolution = initialResolution;

        terrainModel = generatePlane(resolution, resolution);

        terrainNode = new SceneNode(terrainModel);
        terrainNode->GetTransform().setScale(glm::vec3(10.0f));
        terrainNode->GetTransform().setTranslation(glm::vec3(0.0f, -2.0f, 0.0f));

        sceneManager.GetRoot()->AddChild(terrainNode);
    }

    // 析构函数
    ~TerrainSystem() {
        delete terrainNode;
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

    // 更新分辨率
    void updateResolution(int newResolution) {
        // 清除旧数据
        if (terrainModel->meshes.size() > 0) {
            Mesh &terrainMesh = terrainModel->meshes[0];
            terrainMesh.indices.clear();
            terrainMesh.vertices.clear();
            terrainModel->meshes[0].clearBuffers();
            terrainModel->meshes.clear();
        }

        // 创建新的数据
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        int nbX = newResolution;
        int nbY = newResolution;

        // 生成顶点数据
        for (int i = 0; i < nbX; i++) {
            for (int j = 0; j < nbY; j++) {
                Vertex vertex;

                float x = -1 + i * (2.0 / (nbX-1));
                float z = -1 + j * (2.0 / (nbY-1));
                float u = 0 + i * (1.0 / (nbX-1));
                float v = 0 + j * (1.0 / (nbY-1));

                vertex.Position = glm::vec3(x, 0.f, z);
                vertex.TexCoords = glm::vec2(u, v);
                vertices.push_back(vertex);
            }
        }

        // 生成三角形
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

        // 更新Mesh和Model
        Mesh terrainMesh = Mesh(vertices, indices, textures);
        terrainModel->meshes.push_back(terrainMesh);
    }
};

#endif TERRAINSYSTEM_HPP