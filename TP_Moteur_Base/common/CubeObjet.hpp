#ifndef CUBEOBJET_HPP
#define CUBEOBJET_HPP
#include "GameObjet.hpp"

class CubeObjet : public GameObjet {
public:
    float m_halfWidth;
    float m_mass;
    glm::vec3 m_startPosition;

    CubeObjet(SceneManager& sceneManager, Shader* shader, float halfWidth, float mass, glm::vec3 startPos) :GameObjet() {
        m_halfWidth = halfWidth;
        m_mass = mass;
        m_startPosition = startPos;

        renderingModel = new RenderingModel();
        renderingModel->shader = shader;

        // 构建Mesh数据
        Mesh objectMesh = BuildCubeMesh();
        renderingModel->meshes.push_back(objectMesh);

        // 挂载到场景树上
        sceneNode = new SceneNode(renderingModel);
        sceneNode->GetTransform().setTranslation(m_startPosition);
        sceneManager.GetRoot()->AddChild(sceneNode);

        // 物理数据初始化
        CubeShape* cubeShape = new CubeShape(m_halfWidth);
        physicsModel = new PhysicsModel(cubeShape, m_mass,sceneNode->GetTransform().getTranslation() );

        // 同步状态
        SyncTransform();
    }


    void SetVelocity(const glm::vec3& velocity) {
        if (physicsModel->isDynamic()) {
            physicsModel->m_velocity = velocity;
        }
    }

    void ResetStatus() {
        physicsModel->m_physicsPosition = m_startPosition;
    }



private:
    Mesh BuildCubeMesh() {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        float halfWidth = m_halfWidth;

        // 遍历 3 个轴 (0:x, 1:y, 2:z)
        for (int i = 0; i < 3; i++) {
            // 每个轴有两个面 (负方向 -1, 正方向 1)
            for (int dir : {-1, 1}) {
                glm::vec3 normal(0);
                normal[i] = (float)dir;

                // 确定另外两个轴，用于构建平面的 4 个角
                int axis1 = (i + 1) % 3;
                int axis2 = (i + 2) % 3;

                unsigned int startIdx = vertices.size();

                // 生成平面的 4 个顶点
                for (int a = -1; a <= 1; a += 2) {
                    for (int b = -1; b <= 1; b += 2) {
                        Vertex v;
                        v.Normal = normal;
                        v.Position[i] = normal[i] * halfWidth;
                        v.Position[axis1] = (float)a * halfWidth;
                        v.Position[axis2] = (float)b * halfWidth * (float)dir;
                        vertices.push_back(v);
                    }
                }

                // 针对这 4 个顶点生成 2 个三角形索引
                indices.push_back(startIdx + 0);
                indices.push_back(startIdx + 1);
                indices.push_back(startIdx + 3);

                indices.push_back(startIdx + 0);
                indices.push_back(startIdx + 3);
                indices.push_back(startIdx + 2);
            }
        }

        return Mesh(vertices, indices, std::vector<Texture>());
    }
};

#endif // CUBEOBJET_HPP