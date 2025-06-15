//Graphics Programming.cpp

#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

void createGeometry(GLuint& vao, GLuint& EBO, int& size, int& numTriangles);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
GLuint loadTexture(const char* path, int comp = 0);
void renderSkyBox();
void renderTerrain();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID);
void renderToBuffer(unsigned int frameBufferTo, unsigned int colorBufferFrom, unsigned int shader);
void renderQuad();
unsigned int GeneratePlane(const char* heightmap, unsigned char*& data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID);

// bloom functions
void createBloomFramebuffers();
void createBloomShaders();
void renderBloom();

// window callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);

bool keys[1024];

void loadFile(const char* filename, char*& output);

GLuint simpleProgram, skyProgram, terrainProgram, modelProgram, blitProgram, extractProgram, blurProgram, bloomProgram;

// world data
glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(100.0f, 125.5f, 100.0f);

GLuint boxVAO, boxEBO;
int boxSize, boxIndexCount;
glm::mat4 view, projection;

float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

// terrain data
GLuint terrainVAO, terrainIndexCount, heightmapID, heightNormalID;
unsigned char* heightmapTexture;

GLuint dirt, sand, grass, rock, snow;

Model* backpack;
Model* rum;
Model* watchtower;
Model* apple;

//bloom data
unsigned int hdrFBO, colorBuffers[2];
unsigned int rboDepth;
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];

int main()
{
    // glfw init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // make window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hello Window :)", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // register callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    // context current
    glfwMakeContextCurrent(window);

    // load GLAD functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Error loading GLAD" << std::endl;
        glfwTerminate();
        return -2;
    }

    glEnable(GL_DEPTH_TEST);

    createGeometry(boxVAO, boxEBO, boxSize, boxIndexCount);
    createShaders();
    createBloomFramebuffers();
    createBloomShaders();

    terrainVAO = GeneratePlane("textures/heightmap.png", heightmapTexture, GL_RGBA, 4, 100.0f, 5.0f, terrainIndexCount, heightmapID);
    heightNormalID = loadTexture("textures/heightnormal.png");

    GLuint boxTex = loadTexture("textures/container2.png");
    GLuint boxNormal = loadTexture("textures/container2_normal.png");

    dirt = loadTexture("textures/dirt.jpg");
    sand = loadTexture("textures/sand.jpg");
    grass = loadTexture("textures/grass.jpg", 4);
    rock = loadTexture("textures/rock.jpg");
    snow = loadTexture("textures/snow.jpg");


    // flips so that textures are aligned
    stbi_set_flip_vertically_on_load(true);
    backpack = new Model("models/backpack/backpack.obj");

    // flips back so that textures are aligned here too
    stbi_set_flip_vertically_on_load(false);
    rum = new Model("models/rum/rum.obj");
    watchtower = new Model("models/watchtower/watchtower.obj");
    apple = new Model("models/apple/apple.obj");

    // creates OpenGL viewport
    glViewport(0, 0, WIDTH, HEIGHT);

    view = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projection = glm::perspective(glm::radians(45.0f), WIDTH / (float)HEIGHT, 0.1f, 5000.0f);

    // run loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window);

        float t = glfwGetTime();
        float red = std::sinf(t);

        // render with framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderSkyBox();
        renderTerrain();

        // models
        renderModel(backpack, glm::vec3(1334.5, 208, 1384.5), glm::vec3(0, 50, 0), glm::vec3(1.2, 1.2, 1.2));
        renderModel(rum, glm::vec3(1335, 210.35, 1382.2), glm::vec3(-90, 0, -35), glm::vec3(0.07, 0.1, 0.07));
        renderModel(watchtower, glm::vec3(1350, 140, 1400), glm::vec3(0, 180, 0), glm::vec3(10, 10, 10));
        renderModel(apple, glm::vec3(1337, 210.35, 1382.2), glm::vec3(-90, 0, 0), glm::vec3(0.005, 0.005, 0.005));

        // applies bloom
        renderBloom();

        // renders bloom to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(bloomProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glUniform1i(glGetUniformLocation(bloomProgram, "scene"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
        glUniform1i(glGetUniformLocation(bloomProgram, "bloomBlur"), 1);

        renderQuad();
        glEnable(GL_DEPTH_TEST);

        // swap
        glfwSwapBuffers(window);
    }

    // cleanup
    delete backpack;
    delete rum;
    delete watchtower;
    delete apple;

    // terminate
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    float x = (float)xpos;
    float y = (float)ypos;

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float dx = x - lastX;
    float dy = y - lastY;
    lastX = x;
    lastY = y;

    camYaw -= dx;
    camPitch = glm::clamp(camPitch + dy, -90.0f, 90.0f);
    if (camYaw > 180.0f) camYaw -= 360.0f;
    if (camYaw < -180.0f) camYaw += 360.0f;

    camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

    glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
    glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
    view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

void renderSkyBox() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(skyProgram);

    // matrices
    glm::mat4 world = glm::mat4(1.0f);
    world = glm::translate(world, cameraPosition);
    world = glm::scale(world, glm::vec3(10, 10, 10));

    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    // rendering
    glBindVertexArray(boxVAO);
    glDrawElements(GL_TRIANGLES, boxIndexCount, GL_UNSIGNED_INT, 0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void renderTerrain() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(terrainProgram);

    glm::mat4 world = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(glGetUniformLocation(terrainProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightNormalID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dirt);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sand);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grass);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rock);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, snow);

    // rendering
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
}

unsigned int GeneratePlane(const char* heightmap, unsigned char*& data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID) {
    int width, height, channels;
    data = nullptr;
    if (heightmap != nullptr) {
        data = stbi_load(heightmap, &width, &height, &channels, comp);
        if (data) {
            glGenTextures(1, &heightmapID);
            glBindTexture(GL_TEXTURE_2D, heightmapID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    int stride = 8;
    float* vertices = new float[(width * height) * stride];
    unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];

    int index = 0;
    for (int i = 0; i < (width * height); i++) {
        int x = i % width;
        int z = i / width;

        float texHeight = (float)data[i * comp];

        vertices[index++] = x * xzScale;
        vertices[index++] = (texHeight / 255.0f) * hScale;
        vertices[index++] = z * xzScale;

        vertices[index++] = 0;
        vertices[index++] = 1;
        vertices[index++] = 0;

        vertices[index++] = x / (float)width;
        vertices[index++] = z / (float)height;
    }

    index = 0;
    for (int i = 0; i < (width - 1) * (height - 1); i++) {
        int x = i % width;
        int z = i / width;

        int vertex = z * width + x;

        indices[index++] = vertex;
        indices[index++] = vertex + width;
        indices[index++] = vertex + width + 1;

        indices[index++] = vertex;
        indices[index++] = vertex + width + 1;
        indices[index++] = vertex + 1;
    }

    unsigned int vertSize = (width * height) * stride * sizeof(float);
    indexCount = ((width - 1) * (height - 1) * 6);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    delete[] vertices;
    delete[] indices;

    return VAO;
}

void processInput(GLFWwindow* window)
{
    bool camChanged = false;

    if (keys[GLFW_KEY_W]) {
        cameraPosition += camQuat * glm::vec3(0, 0, 2);
        camChanged = true;
    }
    if (keys[GLFW_KEY_S]) {
        cameraPosition += camQuat * glm::vec3(0, 0, -2);
        camChanged = true;
    }
    if (keys[GLFW_KEY_A]) {
        cameraPosition += camQuat * glm::vec3(2, 0, 0);
        camChanged = true;
    }
    if (keys[GLFW_KEY_D]) {
        cameraPosition += camQuat * glm::vec3(-2, 0, 0);
        camChanged = true;
    }

    if (camChanged) {
        glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
        glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
        view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
    }
}

void createGeometry(GLuint& vao, GLuint& EBO, int& size, int& numIndices) {
    // need 24 vertices for normal/uv-mapped Cube
    float vertices[] = {
        // positions            //colors            // tex coords   // normals          //tangents      //bitangents
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
    };

    unsigned int indices[] = {  // note that we start from 0!
        // DOWN
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // UP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };

    size = sizeof(vertices) / sizeof(float);
    numIndices = sizeof(indices) / sizeof(int);
    int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));
    glEnableVertexAttribArray(5);
}

void createShaders() {

    createProgram(simpleProgram, "shaders/simpleVertex.shader", "shaders/simpleFragment.shader");

    // set texture channels
    glUseProgram(simpleProgram);
    glUniform1i(glGetUniformLocation(simpleProgram, "mainTex"), 0);
    glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"), 1);
    

    createProgram(skyProgram, "shaders/skyVertex.shader", "shaders/skyFragment.shader");
    createProgram(terrainProgram, "shaders/terrainVertex.shader", "shaders/terrainFragment.shader");
    createProgram(blitProgram, "shaders/imageVertex.shader", "shaders/imageFragment.shader");

    glUseProgram(terrainProgram);
    glUniform1i(glGetUniformLocation(terrainProgram, "mainTex"), 0); 
    glUniform1i(glGetUniformLocation(terrainProgram, "normalTex"), 1);

    glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
    glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
    glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 4);
    glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 5);
    glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);

    createProgram(modelProgram, "shaders/model.vs", "shaders/model.fs");
    glUseProgram(modelProgram);

    glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"), 1);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_normal1"), 2);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"), 3);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"), 4);
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment) {
    char* vertexSrc;
    char* fragmentSrc;
    loadFile(vertex, vertexSrc);
    loadFile(fragment, fragmentSrc);

    GLuint vertexShaderID, fragmentShaderID;

    vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShaderID);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShaderID, 512, nullptr, infoLog);
        std::cout << "ERROR COMPILING VERTEX SHADER\n" << infoLog << std::endl;
    }

    fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShaderID);

    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infoLog);
        std::cout << "ERROR COMPILING FRAGMENT SHADER\n" << infoLog << std::endl;
    }

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(fragmentShaderID, 512, nullptr, infoLog);
        std::cout << "ERROR LINKING PROGRAM\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    delete vertexSrc;
    delete fragmentSrc;
}

void loadFile(const char* filename, char*& output) {

    std::ifstream file(filename, std::ios::binary);

    if (file.is_open()) {
        file.seekg(0, file.end);
        int length = file.tellg();
        file.seekg(0, file.beg);

        output = new char[length + 1];

        file.read(output, length);

        output[length] = '\0';

        file.close();

    }

    else {
        output = NULL;
    }
}

GLuint loadTexture(const char* path, int comp)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, numChannels;
    unsigned char* data = stbi_load(path, &width, &height, &numChannels, comp);
    if (data) {
        if (comp != 0) numChannels = comp;

        if (numChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        if (numChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Error loading texture: " << path << std::endl;
    }

    stbi_image_free(data);


    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
{
    //glEnable(GL_BLEND);

    //Alpha blend
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Additive blend
    //glBlendFunc(GL_ONE, GL_ONE);

    //Soft additive blend
    //glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);

    //Multiply blend
    //glBlendFunc(GL_DST_COLOR, GL_ZERO);

    //Double multiply blend
    //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(modelProgram);

    glm::mat4 world = glm::mat4(1.0f);
    world = glm::translate(world, pos);
    world = glm::scale(world, scale);
    world = glm::rotate(world, glm::radians(rot.x), glm::vec3(1, 0, 0));
    world = glm::rotate(world, glm::radians(rot.y), glm::vec3(0, 1, 0));
    world = glm::rotate(world, glm::radians(rot.z), glm::vec3(0, 0, 1));

    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    model->Draw(modelProgram);

    // glDisable(GL_BLEND);
}

void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID) {
    
    // generate frame buffer
    glGenFramebuffers(1, &frameBufferID);


    // generate color buffer
    glGenTextures(1, &colorBufferID);
    glBindTexture(GL_TEXTURE_2D, colorBufferID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    // generate depth buffer
    glGenRenderbuffers(1, &depthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  colorBufferID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "framebuffer not complete"<< std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderToBuffer(unsigned int frameBufferTo, unsigned int colorBufferFrom, unsigned int shader) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferTo);

    glUseProgram(shader);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBufferFrom);

    // rendering
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int  quadVAO = 0;
unsigned int quadVBO;

void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void createBloomFramebuffers()
{
    // HDR framebuffer
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    // creates depth buffer
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "HDR Framebuffer not complete!" << std::endl;

    // pingpong buffers
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Ping-pong Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createBloomShaders()
{
    // creates extract shader
    createProgram(extractProgram, "shaders/extractVertex.shader", "shaders/extractFragment.shader");
    glUseProgram(extractProgram);
    glUniform1i(glGetUniformLocation(extractProgram, "image"), 0);

    // creates blur shader
    createProgram(blurProgram, "shaders/blurVertex.shader", "shaders/blurFragment.shader");
    glUseProgram(blurProgram);
    glUniform1i(glGetUniformLocation(blurProgram, "image"), 0);

    // creates combined bloom shader 
    createProgram(bloomProgram, "shaders/bloomVertex.shader", "shaders/bloomFragment.shader");
    glUseProgram(bloomProgram);
    glUniform1i(glGetUniformLocation(bloomProgram, "scene"), 0);
    glUniform1i(glGetUniformLocation(bloomProgram, "bloomBlur"), 1);
}

void renderBloom()
{
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glViewport(0, 0, WIDTH, HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(extractProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
    renderQuad();

    bool horizontal = true;
    unsigned int amount = 10;
    glUseProgram(blurProgram);

    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glUniform1i(glGetUniformLocation(blurProgram, "horizontal"), horizontal);
        glActiveTexture(GL_TEXTURE0);

        // first uses the normal result, then goes for pingpong
        if (i == 0) {
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        }

        renderQuad();
        horizontal = !horizontal;
    }

    glEnable(GL_DEPTH_TEST);
}