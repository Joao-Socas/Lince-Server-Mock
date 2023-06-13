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

glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
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

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glm::vec3 light_1_position(1.2f, 1.0f, 2.0f);
    glm::vec3 light_1_color(1.0f, 1.0f, 1.0f);
    float light_1_strength(100.f);

    glm::vec3 directional_light_direction(-0.2f, 1.0f, -0.3f);
    glm::vec3 directional_light_color(1.0f, 1.0f, 0.8f);
    directional_light_color = 2.f * directional_light_color;

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

    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetCursorPosCallback(window, mouse_callback);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 10000.0f);

    ShaderProgram screen_shader_program(VertexShader("Engine/Shaders/screen.vs.glsl"), FragmentShader("Engine/Shaders/screen.fs.glsl"));
    ShaderProgram framebuffer_shader_program(VertexShader("Engine/Shaders/VS_01.glsl"), FragmentShader("Engine/Shaders/FS_01.glsl"));
    Model model("Application/Models/backpack.obj");
    Model model2("Application/Models/Cube.obj");
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);


    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    unsigned int frame_buffer;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);


    unsigned int render_buffer_depth;
    glGenRenderbuffers(1, &render_buffer_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_depth);


    unsigned int pixel_buffer;
    glGenBuffers(1, &pixel_buffer);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer);
    glBufferData(GL_PIXEL_PACK_BUFFER, SCR_WIDTH * SCR_HEIGHT * 4, NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const char* output_file_path = "encoded.h264";
    EncodeNVENC encoder(0, pixel_buffer, SCR_WIDTH, SCR_HEIGHT, output_file_path);
    unsigned char* data_buffer = new unsigned char[SCR_WIDTH * SCR_HEIGHT * 4];
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.f, 0.f, 0.f, 1.f); //(VUYA)
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

         //render the loaded model
        glm::mat4 model_transform = glm::mat4(1.0f);
        model_transform = glm::translate(model_transform, glm::vec3(0.0f, 5.0f, -8.f)); // translate it down so it's at the center of the scene
        framebuffer_shader_program.setUniform("model", model_transform);
        model.Draw(framebuffer_shader_program);

        //model_transform = glm::mat4(1.0f);
        //model_transform = glm::translate(model_transform, glm::vec3(-5.f, 5.f, -5.f));
        //model_transform = glm::scale(model_transform, glm::vec3(1.0f, 10.f, 10.f));
        //framebuffer_shader_program.setUniform("model", model_transform);
        //model2.Draw(framebuffer_shader_program);

        //model_transform = glm::mat4(1.0f);
        //model_transform = glm::translate(model_transform, glm::vec3(0.f, 0.f, -5.f));
        //model_transform = glm::scale(model_transform, glm::vec3(10.0f, 1.0f, 10.f)); // translate it down so it's at the center of the scene
        //framebuffer_shader_program.setUniform("model", model_transform);
        //model2.Draw(framebuffer_shader_program);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer);
        glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glFinish();
        encoder.Encode();
        glFinish();
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer);
        GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        std::memcpy(data_buffer, ptr, SCR_WIDTH * SCR_HEIGHT * 4);

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.f, 0.f, 0.f, 1.f); //(BGRA)
        screen_shader_program.use();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_buffer);

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
        glFinish();
    }
    encoder.CleanupEncoder();

    delete[] data_buffer;
    glfwTerminate();
    return 0;
}