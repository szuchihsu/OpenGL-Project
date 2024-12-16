#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <sstream>

// Screen dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera parameters
float cameraYaw = -90.0f;
float cameraPitch = 0.0f;
glm::vec3 cameraPos(0.0f, 1.5f, 3.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Function to compile a shader and check for errors
unsigned int compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader Compilation Error:\n" << infoLog << std::endl;
    }

    return shader;
}

// Function to read a shader file and return its source code
std::string readShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to link shaders into a program
unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexSource = readShaderSource(vertexPath);
    std::string fragmentSource = readShaderSource(fragmentPath);

    unsigned int vertexShader = compileShader(vertexSource.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource.c_str(), GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader Program Linking Error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;

    glm::vec3 cameraFrontXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 rightXZ = glm::normalize(glm::cross(cameraFrontXZ, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFrontXZ;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFrontXZ;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraSpeed * rightXZ;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraSpeed * rightXZ;
    // user height
    cameraPos.y = 1.5f;
}

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static const float sensitivity = 0.1f;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // Reversed: y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    cameraYaw += xOffset;
    cameraPitch += yOffset;

    // Constrain pitch to avoid screen flip
    if (cameraPitch > 89.0f)
        cameraPitch = 89.0f;
    if (cameraPitch < -89.0f)
        cameraPitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(front);
}


float fov = 45.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Art Gallery", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Load shaders, textures, and other resources here
    // 
    // Load shaders
    unsigned int shaderProgram = createShaderProgram("shader.vert", "shader.frag");
    glUseProgram(shaderProgram);

    // Load textures
    unsigned int wallTexture = loadTexture("textures/wall.jpg");
    unsigned int floorTexture = loadTexture("textures/floor.jpg");
    unsigned int paintingTexture = loadTexture("textures/painting.png");
    unsigned int painting2Texture = loadTexture("textures/painting2.jpg");
    unsigned int painting3Texture = loadTexture("textures/painting3.jpg");
    unsigned int painting4Texture = loadTexture("textures/painting4.jpg");
    unsigned int ceilingTexture = loadTexture("textures/ceiling.jpg");
    unsigned int cubeTexture = loadTexture("textures/cube.jpg");

    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };

    // Define lights above each painting
    PointLight lights[5] = {
        { glm::vec3(0.0f, 3.5f, -18.0f), glm::vec3(1.0f, 0.8f, 0.8f), 1.2f }, // Slightly higher
        { glm::vec3(-18.0f, 3.5f, 0.0f), glm::vec3(1.0f, 0.8f, 0.8f), 1.2f },
        { glm::vec3(0.0f, 3.5f, 18.0f), glm::vec3(1.0f, 0.8f, 0.8f), 1.2f },
        { glm::vec3(18.0f, 3.5f, 0.0f), glm::vec3(1.0f, 0.8f, 0.8f), 1.2f },
        { glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 0.8f, 0.8f), 1.0f }
    };


    // Pass light data to shaders
    for (int i = 0; i < 5; ++i) {
        std::string lightPosUniform = "lights[" + std::to_string(i) + "].position";
        std::string lightColorUniform = "lights[" + std::to_string(i) + "].color";
        std::string lightIntensityUniform = "lights[" + std::to_string(i) + "].intensity";

        glUniform3fv(glGetUniformLocation(shaderProgram, lightPosUniform.c_str()), 1, glm::value_ptr(lights[i].position));
        glUniform3fv(glGetUniformLocation(shaderProgram, lightColorUniform.c_str()), 1, glm::value_ptr(lights[i].color));
        glUniform1f(glGetUniformLocation(shaderProgram, lightIntensityUniform.c_str()), lights[i].intensity);
    }

    // Pass the camera (view) position to the shader
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

    // Set texture uniforms in the shader
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    float vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom-left
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top-right
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top-right
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f, // top-left
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f  // bottom-left
    };

    // Create VAO, VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // CUBE
    float cubeVertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    unsigned int cubeVAO, cubeVBO;

    // Generate VAO and VBO for the cube
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    // Bind the cube's VAO
    glBindVertexArray(cubeVAO);

    // Bind and upload vertex data to the cube's VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Configure vertex attributes for the cube
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime for smooth movement
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process user input
        processInput(window);

        // Clear the color and depth buffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set camera view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Render the floor 
        // Floor 1
        glBindTexture(GL_TEXTURE_2D, floorTexture);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Position the floor
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the floor to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the floor (6 vertices for 2 triangles)

        // Floor 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, -1.0f, 0.0f)); // Position the floor
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the floor to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the floor (6 vertices for 2 triangles)

        // Floor 3
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 10.0f)); // Position the floor
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the floor to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the floor (6 vertices for 2 triangles)

        // Floor 4
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, -1.0f, 0.0f)); // Position the floor
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the floor to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the floor (6 vertices for 2 triangles)

        // Floor 5
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, -10.0f)); // Position the floor
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the floor to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the floor (6 vertices for 2 triangles)

        // Render the ceiling
        // Ceiling 1
        glBindTexture(GL_TEXTURE_2D, ceilingTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.0f)); // Position the ceiling above
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the ceiling to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the ceiling

        // Ceiling 2
        glBindTexture(GL_TEXTURE_2D, ceilingTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 4.0f, 0.0f)); // Position the ceiling above
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the ceiling to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the ceiling

        // Ceiling 3
        glBindTexture(GL_TEXTURE_2D, ceilingTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, 10.0f)); // Position the ceiling above
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the ceiling to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the ceiling

        // Ceiling 4
        glBindTexture(GL_TEXTURE_2D, ceilingTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, 4.0f, 0.0f)); // Position the ceiling above
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the ceiling to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the ceiling

        // Ceiling 5
        glBindTexture(GL_TEXTURE_2D, ceilingTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, -10.0f)); // Position the ceiling above
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to align with XZ-plane
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 1.0f)); // Scale the ceiling to cover the desired area
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the ceiling

        // Render the walls
        glBindTexture(GL_TEXTURE_2D, wallTexture);

        // Side wall 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 1.5f, -5.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 2
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0f, 1.5f, 10.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 3
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 1.5f, 10.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 4
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 1.5f, 5.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 5
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, 1.5f, -5.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 6
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0f, 1.5f, -10.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 7
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 1.5f, -10.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Side wall 8
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, 1.5f, 5.0f));
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render the cross-shaped extensions

        // Back extension
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, -15.0f)); // Move further back
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f)); // Same size as original walls
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Left extension
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-15.0f, 1.5f, 0.0f)); // Move further left
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate for side wall
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Right extension
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(15.0f, 1.5f, 0.0f)); // Move further right
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate for side wall
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Front extension
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 15.0f)); // Move further front
        model = glm::scale(model, glm::vec3(10.0f, 5.0f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render a painting
        // Painting 1
        glBindTexture(GL_TEXTURE_2D, paintingTexture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, -14.9f)); // Position the painting slightly in front of the back wall
        model = glm::scale(model, glm::vec3(3.0f, 2.0f, 0.1f));       // Scale to match painting dimensions
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Painting 2
        glBindTexture(GL_TEXTURE_2D, painting2Texture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-14.9f, 1.5f, 0.0f)); // Position the painting slightly in front of the back wall
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 2.0f, 0.1f));       // Scale to match painting dimensions
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Painting 3
        glBindTexture(GL_TEXTURE_2D, painting3Texture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 14.9f)); // Position the painting slightly in front of the back wall
        model = glm::scale(model, glm::vec3(3.0f, 2.0f, 0.1f));       // Scale to match painting dimensions
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Painting 4
        glBindTexture(GL_TEXTURE_2D, painting4Texture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(14.9f, 1.5f, 0.0f)); // Position the painting slightly in front of the back wall
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 2.0f, 0.1f));       // Scale to match painting dimensions
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Use the cube's VAO
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        // Apply transformations for the rotating cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, sqrt(3.0f) / 2.0f, 0.0f)); // Position and lift the cube
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Tilt on X-axis
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Tilt on Z-axis
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, -1.0f)); // Rotate over time
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // Scale to desired size

        // Pass the transformation matrix to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Draw the cube
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Unbind the VAO
        glBindVertexArray(0);


        // Swap buffers and poll for I/O events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}



