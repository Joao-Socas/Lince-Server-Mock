// OpenGL Includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STD Includes
#include <iostream>

// Engine Includes
#include <Shader.hpp>
#include <Model.hpp>
#include <BufferController.hpp>
#include <ServerSocket.hpp>
#include <TransactionManager.hpp>
#include <EncodeNVENC.hpp>

// Windows Includes
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<sys/types.h>
#pragma comment(lib,"WS2_32")

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int PORT = 27015;

bool firstMouse = true;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 3.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    const float cameraSpeed = 5.f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    stbi_set_flip_vertically_on_load(true);

    glm::vec3 light_1_position(1.2f, 1.0f, 2.0f);
    glm::vec3 light_1_color(1.0f, 1.0f, 1.0f);
    float light_1_strength(100.f);

    glm::vec3 directional_light_direction(-0.2f, 1.0f, -0.3f);
    glm::vec3 directional_light_color(1.0f, 1.0f, 0.8f);
    directional_light_color = 2.f * directional_light_color;

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };


    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "server", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 10000.0f);

    ShaderProgram framebuffer_shader_program(VertexShader("Engine/Shaders/VS_01.glsl"), FragmentShader("Engine/Shaders/FS_01.glsl"));
    Model model("Application/Models/backpack.obj");
    Model model2("Application/Models/Cube.obj");

    unsigned int frame_buffer;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    unsigned int render_buffer;
    glGenRenderbuffers(1, &render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB10_A2, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, frame_buffer);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    BufferController buffer_controller(SCR_WIDTH, SCR_HEIGHT);
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cout << "ERROR: Falha ao inciar o controlador de sockets do windows! (" << iResult << ")" << std::endl;
        return 0;
    }
    ServerSocketIPV4 server_socket(PORT);
    server_socket.Connect();
    TransactionManager transaction_manager(&server_socket, &buffer_controller);

    EncodeNVENC encoder(0, render_buffer, SCR_WIDTH, SCR_HEIGHT);

    while (!glfwWindowShouldClose(window) && server_socket.b_connected)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        framebuffer_shader_program.use();
        framebuffer_shader_program.setUniform("view", view);
        framebuffer_shader_program.setUniform("projection", projection);
        framebuffer_shader_program.setUniform("shininess", 32.0f);
        framebuffer_shader_program.setUniform("specular_strength", 32.0f);
        framebuffer_shader_program.setUniform("light_1_position", light_1_position);
        framebuffer_shader_program.setUniform("light_1_color", light_1_color);
        framebuffer_shader_program.setUniform("light_1_strength", light_1_strength);
        framebuffer_shader_program.setUniform("view_position", cameraPos);
        framebuffer_shader_program.setUniform("directional_light_direction", directional_light_direction);
        framebuffer_shader_program.setUniform("directional_light_color", directional_light_color);

        // Seta o buffer gerado pelo cuda para o framebuffer
       // encoder.SetUpFrameBuffer();

        // render the loaded model
        glm::mat4 model_transform = glm::mat4(1.0f);
        model_transform = glm::translate(model_transform, glm::vec3(0.0f, 5.0f, -8.f)); // translate it down so it's at the center of the scene
        framebuffer_shader_program.setUniform("model", model_transform);
        model.Draw(framebuffer_shader_program);

        model_transform = glm::mat4(1.0f);
        model_transform = glm::translate(model_transform, glm::vec3(-5.f, 5.f, -5.f));
        model_transform = glm::scale(model_transform, glm::vec3(1.0f, 10.f, 10.f));
        framebuffer_shader_program.setUniform("model", model_transform);
        model2.Draw(framebuffer_shader_program);

        model_transform = glm::mat4(1.0f);
        model_transform = glm::translate(model_transform, glm::vec3(0.f, 0.f, -5.f));
        model_transform = glm::scale(model_transform, glm::vec3(10.0f, 1.0f, 10.f)); // translate it down so it's at the center of the scene
        framebuffer_shader_program.setUniform("model", model_transform);
        model2.Draw(framebuffer_shader_program);

        encoder.Encode(buffer_controller.drawing_buffer);
        while (buffer_controller.new_on_wait) {};
        buffer_controller.SwapDrawingBuffer();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    encoder.CleanupEncoder(buffer_controller.drawing_buffer);
    glfwTerminate();
    return 0;
} 