// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <algorithm>

#include <fstream>
using namespace glm;

#include <common/shader.hpp>
#include "common/camera.hpp"
#include "common/Time.hpp"
#include "common/model.hpp"
#include "common/shader.hpp"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouseClick_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// 随机数
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(-0.5f, 0.5f);

// 窗口设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool isWireframe = false;

// 相机初始化参数
Camera camera(glm::vec3(0.0f, 3.f, 3.f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseClickDown = false;

// 地形相关参数
Model terrainModel;
int resolution = 16;


// 生成一个地形平面
Model generatePlane(int nbX, int nbY) {
    Model terrainModel = Model();
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

            vertex.Position = vec3(x, 0.f, z);
            vertex.TexCoords = vec2(u, v);
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
    terrainModel.meshes.push_back(terrainMesh);

    return terrainModel;
}

// 随机改变地形高度
void changeAltitudeRandom(Model &model) {
    if (model.meshes.size() > 0) {
        Mesh &mesh = model.meshes[0];
       for (size_t i = 0; i < mesh.vertices.size(); i++) {
           Vertex &vertex = mesh.vertices[i];
           vertex.Position.y = dis(gen);
       }
        mesh.updateMesh();
    }
}

// 更新地形
void updateTerrainMesh(Model &terrainModel, int nbX, int nbY) {

    // 清除旧数据
    if (terrainModel.meshes.size() > 0) {
        Mesh &terrainMesh = terrainModel.meshes[0];
        terrainMesh.indices.clear();
        terrainMesh.vertices.clear();
        terrainModel.meshes[0].clearBuffers();
        terrainModel.meshes.clear();
    }

    // 创建新的数据
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // 生成顶点数据
    for (int i = 0; i < nbX; i++) {
        for (int j = 0; j < nbY; j++) {
            Vertex vertex;

            float x = -1 + i * (2.0 / (nbX-1));
            float z = -1 + j * (2.0 / (nbY-1));
            float u = 0 + i * (1.0 / (nbX-1));
            float v = 0 + j * (1.0 / (nbY-1));

            vertex.Position = vec3(x, 0.f, z);
            vertex.TexCoords = vec2(u, v);
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
    terrainModel.meshes.push_back(terrainMesh);
}



int main( void )
{

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP1 - Moteur de Jeux", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }

    // 创建窗口上下文
    glfwMakeContextCurrent(window);

    // 注册回调函数
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window,mouseClick_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    // stbi_set_flip_vertically_on_load(true);

    // Create and compile our GLSL program from the shaders
    // -------------------------
    Shader shader("./resources/shaders/vertex_shader.glsl", "./resources/shaders/fragment_shader.glsl");

    // Model model("resources/models/chair.off");

    terrainModel = generatePlane(resolution,resolution);
    // changeAltitudeRandom(terrainModel);

    Time::intialize();
    int nbFrames = 0;

    do{
        Time::Update();

        if (camera.m_IsOrbital) {
            camera.UpdateOrbital(Time::DeltaTime);
        }else {
            processInput(window);
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        /*****************TODO***********************/
        // Model matrix : an identity matrix (model will be at the origin) then change
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        glm::mat4 viewMatrix = camera.GetViewMatrix();

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 projectionMatrix = camera.GetProjectiveMatrix();

        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

        shader.setMat4("MVP", MVP);

        // model.Draw(shader);
        terrainModel.Draw(shader);


        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Check if the ESC key was pressed or the window was closed
    }  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup buffers and shader
    terrainModel.meshes[0].clearBuffers();
    glDeleteProgram(shader.m_ID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //TODO add translations
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


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.m_Aspect = (float)width / (float)height;
}

// 鼠标滚轮事件回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 鼠标移动事件回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    //判断是否是第一次移入窗口
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if(mouseClickDown){
        //操作相机
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// 鼠标按键点击事件回调函数
void mouseClick_callback(GLFWwindow* window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        double x,y;
        glfwGetCursorPos(window,&x,&y);
        //如果鼠标左键事件
        if(action == GLFW_PRESS){
            mouseClickDown = true;
            lastX = x;
            lastY = y;
        }else{
            mouseClickDown = false;
        }
    }
}

// 键盘回调函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // 只有在按下的一瞬间触发
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_EQUAL|| key == GLFW_KEY_KP_ADD) {
            resolution++;
            resolution = std::clamp(resolution, 2, 256);
            updateTerrainMesh(terrainModel, resolution, resolution);
            std::cout << "Resolution: " << resolution << std::endl;
        }
        if (key == GLFW_KEY_MINUS) {
            resolution--;
            resolution = std::clamp(resolution, 2, 256);
            updateTerrainMesh(terrainModel, resolution, resolution);
            std::cout << "Resolution: " << resolution << std::endl;
        }

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
                camera.EnableOrbitalMode(glm::vec3(0.0f, 0.0f, 0.0f), 3.0f, 45.0f);
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
