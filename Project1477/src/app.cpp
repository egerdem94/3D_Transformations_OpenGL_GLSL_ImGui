//2328649  - Ege Erdem   2476984   -  AARON AMINU BANDADO 
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <ext/vector_float3.hpp>
#include <gtc/type_ptr.hpp>

// Vertex and Face
struct Vertex {
    float x, y, z;
    glm::vec3 normal;
};

struct Face {
    std::vector<int> vertices;
};

// .off files load function
bool loadOFF(const std::string& filename, std::vector<Vertex>& vertices, std::vector<Face>& faces) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error open off file: " << filename << std::endl;
        return false;
    }

    std::string header;
    file >> header;
    if (header != "OFF") {
        std::cerr << "Not valid off file: " << filename << std::endl;
        return false;
    }

    int numVertices, numFaces, numEdges;
    file >> numVertices >> numFaces >> numEdges;

    vertices.clear();
    faces.clear();

    vertices.resize(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        file >> vertices[i].x >> vertices[i].y >> vertices[i].z;
        vertices[i].normal = glm::vec3(0.0f); // Initialize normal
    }

    faces.resize(numFaces);
    for (int i = 0; i < numFaces; ++i) {
        int n;
        file >> n;
        faces[i].vertices.resize(n);
        for (int j = 0; j < n; ++j) {
            file >> faces[i].vertices[j];
        }
    }

    // normals
    for (const auto& face : faces) {
        glm::vec3 v0 = glm::vec3(vertices[face.vertices[0]].x, vertices[face.vertices[0]].y, vertices[face.vertices[0]].z);
        glm::vec3 v1 = glm::vec3(vertices[face.vertices[1]].x, vertices[face.vertices[1]].y, vertices[face.vertices[1]].z);
        glm::vec3 v2 = glm::vec3(vertices[face.vertices[2]].x, vertices[face.vertices[2]].y, vertices[face.vertices[2]].z);

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        for (int idx : face.vertices) {
            vertices[idx].normal += normal;
        }
    }

    for (auto& vertex : vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }

    return true;
}

// Read shaders 
std::string readShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Error open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Shader compile
GLuint compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

 
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error shader compile\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error links\n" << infoLog << std::endl;
    }

    return program;
}

// Setup ImGui
void setupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// Cleanup ImGui
void cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void setupViewport(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double aspect = (double)width / height;
    glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 100.0);
}

// Global variables for transformations
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
float arbitraryAngle = 0.0f;
glm::vec3 axis(1.0f, 0.0f, 0.0f); // Default rotation axis (x-axis)
glm::vec3 translation(0.0f, 0.0f, 0.0f); // Default translation
glm::vec3 scaleFactors(1.0f, 1.0f, 1.0f); // Default scaling factors
glm::vec3 fixedPoint(0.0f, 0.0f, 0.0f); // Fixed point for scaling
glm::vec3 reflectionPlaneNormal(1.0f, 0.0f, 0.0f); // Default plane normal for reflection
glm::vec3 reflectionPlanePoint(0.0f, 0.0f, 0.0f); // Default point on the plane
float shearXY = 0.0f, shearXZ = 0.0f, shearYX = 0.0f, shearYZ = 0.0f, shearZX = 0.0f, shearZY = 0.0f;
int transformationMode = 0; // 0: Euler angles, 1: Arbitrary axis, 2: Scaling, 3: Translation, 4: Reflection, 5: Shearing
int shadingMode = 0; // 0: Toon Shading, 1: Gooch Shading, 2: Phong Shading

std::vector<std::string> offFiles;
int currentFileIndex = 0;
std::vector<Vertex> vertices;
std::vector<Face> faces;

glm::mat4 computeReflectionMatrix(const glm::vec3& normal, const glm::vec3& point) {
    glm::vec3 n = glm::normalize(normal);
    float d = -glm::dot(n, point);

    glm::mat4 reflectionMatrix = glm::mat4(1.0f);

    reflectionMatrix[0][0] = 1 - 2 * n.x * n.x;
    reflectionMatrix[0][1] = -2 * n.x * n.y;
    reflectionMatrix[0][2] = -2 * n.x * n.z;
    reflectionMatrix[0][3] = 0;

    reflectionMatrix[1][0] = -2 * n.y * n.x;
    reflectionMatrix[1][1] = 1 - 2 * n.y * n.y;
    reflectionMatrix[1][2] = -2 * n.y * n.z;
    reflectionMatrix[1][3] = 0;

    reflectionMatrix[2][0] = -2 * n.z * n.x;
    reflectionMatrix[2][1] = -2 * n.z * n.y;
    reflectionMatrix[2][2] = 1 - 2 * n.z * n.z;
    reflectionMatrix[2][3] = 0;

    reflectionMatrix[3][0] = -2 * d * n.x;
    reflectionMatrix[3][1] = -2 * d * n.y;
    reflectionMatrix[3][2] = -2 * d * n.z;
    reflectionMatrix[3][3] = 1;

    return reflectionMatrix;
}

glm::mat4 computeShearingMatrix() {
    glm::mat4 shearingMatrix = glm::mat4(1.0f);
    shearingMatrix[0][1] = shearXY; // Shear X in Y direction
    shearingMatrix[0][2] = shearXZ; // X in Z 
    shearingMatrix[1][0] = shearYX; // Y in X 
    shearingMatrix[1][2] = shearYZ; // Y in Z 
    shearingMatrix[2][0] = shearZX; // Z in X 
    shearingMatrix[2][1] = shearZY; // Z in Y 
    return shearingMatrix;
}

void applyTransformations(GLuint shaderProgram) {
    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translation);
    model = glm::rotate(model, glm::radians(arbitraryAngle), axis);
    model = glm::scale(model, scaleFactors);
    model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)640 / 480, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    if (transformationMode == 4) {
        // Reflection over an arbitrary plane
        glm::mat4 reflectionMatrix = computeReflectionMatrix(reflectionPlaneNormal, reflectionPlanePoint);
        model = reflectionMatrix * model;
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    }
    else if (transformationMode == 5) {
        // Shearing
        glm::mat4 shearingMatrix = computeShearingMatrix();
        model = shearingMatrix * model;
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    }
}

void loadOffFiles(const std::string& directory) {
    offFiles.clear();
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".off") {
            offFiles.push_back(entry.path().string());
        }
    }
    if (!offFiles.empty()) {
        loadOFF(offFiles[currentFileIndex], vertices, faces);
    }
}

int main(void) {
    // Initialize GLFW
    if (!glfwInit())
        return -1;

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(640, 480, "3D Transformations with ImGui", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Setup ImGui
    setupImGui(window);

    // Load .off files this part and shaders arrenged for my computer need to change
    loadOffFiles("C:/Users/egerd/Desktop/Project1477/off");

    // Shader source codes
    std::string vertexShaderSource = readShaderSource("C:/Users/egerd/Desktop/Project1477/Project1477/src/vertex_shader.glsl");
    std::string toonFragmentShaderSource = readShaderSource("C:/Users/egerd/Desktop/Project1477/Project1477/src/fragment_shader.glsl");
    std::string goochFragmentShaderSource = readShaderSource("C:/Users/egerd/Desktop/Project1477/Project1477/src/gooch_shading.glsl");
    std::string phongFragmentShaderSource = readShaderSource("C:/Users/egerd/Desktop/Project1477/Project1477/src/phong_shading.glsl");

    // Compile and link shaders
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint toonFragmentShader = compileShader(toonFragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint goochFragmentShader = compileShader(goochFragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint phongFragmentShader = compileShader(phongFragmentShaderSource, GL_FRAGMENT_SHADER);

    GLuint toonShaderProgram = linkProgram(vertexShader, toonFragmentShader);
    GLuint goochShaderProgram = linkProgram(vertexShader, goochFragmentShader);
    GLuint phongShaderProgram = linkProgram(vertexShader, phongFragmentShader);

    // Cleanup shaders
    glDeleteShader(vertexShader);
    glDeleteShader(toonFragmentShader);
    glDeleteShader(goochFragmentShader);
    glDeleteShader(phongFragmentShader);

    // Set up vertex data, buffers, and configure vertex attributes
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    std::vector<GLuint> indices;
    for (const auto& face : faces) {
        for (size_t i = 1; i < face.vertices.size() - 1; ++i) {
            indices.push_back(face.vertices[0]);
            indices.push_back(face.vertices[i]);
            indices.push_back(face.vertices[i + 1]);
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    glm::vec3 viewPos(0.0f, 0.0f, 5.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor(1.0f, 0.5f, 0.2f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Viewport
        setupViewport(window);

        // Select shader
        GLuint shaderProgram;
        if (shadingMode == 0) {
            shaderProgram = toonShaderProgram;
        }
        else if (shadingMode == 1) {
            shaderProgram = goochShaderProgram;
        }
        else {
            shaderProgram = phongShaderProgram;
        }

        
        applyTransformations(shaderProgram);

        
        glUseProgram(shaderProgram);

        // Set lighting
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(objectColor));

        // Render .off 
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui transformation controls
        ImGui::Begin("Transformation Controls");

        // Because there are 400 .off files
        if (ImGui::BeginCombo("Select .off file", offFiles[currentFileIndex].c_str())) {
            for (int n = 0; n < offFiles.size(); n++) {
                bool is_selected = (currentFileIndex == n);
                if (ImGui::Selectable(offFiles[n].c_str(), is_selected)) {
                    currentFileIndex = n;
                    loadOFF(offFiles[currentFileIndex], vertices, faces);

                    // Update VBO
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

                    // Update EBO
                    indices.clear();
                    for (const auto& face : faces) {
                        for (size_t i = 1; i < face.vertices.size() - 1; ++i) {
                            indices.push_back(face.vertices[0]);
                            indices.push_back(face.vertices[i]);
                            indices.push_back(face.vertices[i + 1]);
                        }
                    }
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::RadioButton("Euler Angles", &transformationMode, 0);
        ImGui::RadioButton("Arbitrary Axis", &transformationMode, 1);
        ImGui::RadioButton("Scaling", &transformationMode, 2);
        ImGui::RadioButton("Translation", &transformationMode, 3);
        ImGui::RadioButton("Reflection", &transformationMode, 4);
        ImGui::RadioButton("Shearing", &transformationMode, 5);
        if (transformationMode == 0) {
            ImGui::SliderFloat("Rotate X", &angleX, -180.0f, 180.0f);
            ImGui::SliderFloat("Rotate Y", &angleY, -180.0f, 180.0f);
            ImGui::SliderFloat("Rotate Z", &angleZ, -180.0f, 180.0f);
        }
        else if (transformationMode == 1) {
            ImGui::SliderFloat3("Axis", glm::value_ptr(axis), -1.0f, 1.0f);
            ImGui::SliderFloat("Angle", &arbitraryAngle, -180.0f, 180.0f);
        }
        else if (transformationMode == 2) {
            ImGui::SliderFloat3("Scale Factors", glm::value_ptr(scaleFactors), 0.1f, 2.0f);
            ImGui::SliderFloat3("Fixed Point", glm::value_ptr(fixedPoint), -2.0f, 2.0f);
        }
        else if (transformationMode == 3) {
            ImGui::SliderFloat3("Translation", glm::value_ptr(translation), -2.0f, 2.0f);
        }
        else if (transformationMode == 4) {
            ImGui::SliderFloat3("Plane Normal", glm::value_ptr(reflectionPlaneNormal), -1.0f, 1.0f);
            ImGui::SliderFloat3("Point on Plane", glm::value_ptr(reflectionPlanePoint), -1.0f, 1.0f);
        }
        else if (transformationMode == 5) {
            ImGui::SliderFloat("Shear X in Y", &shearXY, -1.0f, 1.0f);
            ImGui::SliderFloat("Shear X in Z", &shearXZ, -1.0f, 1.0f);
            ImGui::SliderFloat("Shear Y in X", &shearYX, -1.0f, 1.0f);
            ImGui::SliderFloat("Shear Y in Z", &shearYZ, -1.0f, 1.0f);
            ImGui::SliderFloat("Shear Z in X", &shearZX, -1.0f, 1.0f);
            ImGui::SliderFloat("Shear Z in Y", &shearZY, -1.0f, 1.0f);
        }
        if (ImGui::Button("Reset")) {
            angleX = angleY = angleZ = arbitraryAngle = 0.0f;
            axis = glm::vec3(1.0f, 0.0f, 0.0f);
            scaleFactors = glm::vec3(1.0f, 1.0f, 1.0f);
            fixedPoint = glm::vec3(0.0f, 0.0f, 0.0f);
            translation = glm::vec3(0.0f, 0.0f, 0.0f);
            reflectionPlaneNormal = glm::vec3(1.0f, 0.0f, 0.0f);
            reflectionPlanePoint = glm::vec3(0.0f, 0.0f, 0.0f);
            shearXY = shearXZ = shearYX = shearYZ = shearZX = shearZY = 0.0f;
        }

        ImGui::RadioButton("Toon Shading", &shadingMode, 0);
        ImGui::RadioButton("Gooch Shading", &shadingMode, 1);
        ImGui::RadioButton("Phong Shading", &shadingMode, 2);

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui
    cleanupImGui();

    glfwTerminate();
    return 0;
}
