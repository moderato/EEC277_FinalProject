#include <iostream>
#include <cmath>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "shader.h"
#include "camera.h"

// Correctly set resolution for Macbook Retina
#ifdef __APPLE__
#define MUL 2
#else
#define MUL 1
#endif

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera  camera(glm::vec3(0.0f, 1.5f, 12.0f));
GLfloat lastX  =  WIDTH;
GLfloat lastY  =  HEIGHT;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;   // Time between current frame and last frame
GLfloat lastFrame = 0.0f;   // Time of last frame

// Spheres
const int MAX_SPHERE_NUM = 225;
glm::vec3 sp_pos[MAX_SPHERE_NUM];

// The MAIN function, from here we start the application and run the game loop
int main(int argc, char const *argv[])
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if(!window) {
        fprintf(stderr, "Failed to create GLFW window.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();
    
    GLfloat first_pass_quad[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f};
    GLfloat second_pass_quad[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f};
    GLuint first_pass_VBO, first_pass_VAO;
    glGenBuffers(1, &first_pass_VBO);
    glGenVertexArrays(1, &first_pass_VAO);
    glBindVertexArray(first_pass_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, first_pass_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(first_pass_quad), first_pass_quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    GLuint second_pass_VBO, second_pass_VAO;
    glGenBuffers(1, &second_pass_VBO);
    glGenVertexArrays(1, &second_pass_VAO);
    glBindVertexArray(second_pass_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, second_pass_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(second_pass_quad), second_pass_quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    GLuint FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    
    GLuint image, data;
    glGenTextures(1, &image);
    glGenTextures(1, &data);
    glBindTexture(GL_TEXTURE_2D, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH * MUL, HEIGHT * MUL, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image, 0);

    
    glBindTexture(GL_TEXTURE_2D, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH * MUL, HEIGHT * MUL, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, data, 0);
    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, DrawBuffers);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    // Define the viewport dimensions
    glViewport(0, 0, 2 * WIDTH, 2 * HEIGHT);
    
    // OpenGL options
    glEnable(GL_DEPTH_TEST);
    
    // Build and compile our shader programs
    Shader firstPassShader("first_pass.vs", "first_pass.frag");
    Shader secondPassShader("second_pass.vs", "second_pass.frag");
    
    int num_spheres = 3;
    if(argc > 1)
        num_spheres = atoi(argv[1]);
    num_spheres = 25;
//    if(num_spheres > MAX_SPHERE_NUM) {
//        fprintf(stderr, "Too many spheres!\n");
//        exit(EXIT_FAILURE);
//    }
    
    for(int i = 0; i < num_spheres; i++) {
        sp_pos[i] = glm::vec3(-3.0f + 1.5f * (i % 5), 0.5f + 1.5f * (i / 25), 0.0f + 1.5f * ((i % 25) / 5));
    }
    
    int frames = 0;
    float start = glfwGetTime();
    
    GLfloat* rayRateArray = new GLfloat[WIDTH * MUL * HEIGHT * MUL];
    
    while (!glfwWindowShouldClose(window))
    {
        GLfloat current = glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;
        
        // Clear the colorbuffer
        glfwPollEvents();
        do_movement();
        
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        firstPassShader.Use();
        glBindVertexArray(first_pass_VAO);
        
        // Create camera transformations
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(firstPassShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(firstPassShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        glUniform1f(glGetUniformLocation(firstPassShader.Program, "iGlobalTime"), glfwGetTime());
        glUniform3f(glGetUniformLocation(firstPassShader.Program, "resolution"), WIDTH * MUL, HEIGHT * MUL, 0);
        glUniform1i(glGetUniformLocation(firstPassShader.Program, "num_spheres"), num_spheres);
        glUniform1i(glGetUniformLocation(firstPassShader.Program, "iterations"), 6);
        glUniform3f(glGetUniformLocation(firstPassShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3f(glGetUniformLocation(firstPassShader.Program, "light_direction"), -1.0 + 4.0 * cos(current), 1.5f, 1.0 + 4.0 * sin(current));
        double xpos, ypos;
	glfwGetCursorPos(window,&xpos, &ypos);
	glUniform4f(glGetUniformLocation(firstPassShader.Program, "cursor"), xpos, ypos, xpos, ypos);
        
        for(int i = 0; i < num_spheres; i++) {
            std::string sphere = "spheres[" + std::to_string(i) + "]";
            glUniform4f(glGetUniformLocation(firstPassShader.Program, (sphere + ".position_r").c_str()), sp_pos[i].x, sp_pos[i].y, sp_pos[i].z, 0.5f);
            glUniform3f(glGetUniformLocation(firstPassShader.Program, (sphere + ".material.color").c_str()), 0.2f, 0.3, 0.8f);
            glUniform3f(glGetUniformLocation(firstPassShader.Program, (sphere + ".material.diff_spec_ref").c_str()), 0.5f, 0.5f, 1.1f);
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw Screen
        secondPassShader.Use();
        glBindVertexArray(second_pass_VAO);
        glBindTexture(GL_TEXTURE_2D, image);	// Use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glBindTexture(GL_TEXTURE_2D, data);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_BLUE, GL_FLOAT, rayRateArray);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        float sum = 0;
        for(int i = 0; i < WIDTH * HEIGHT * MUL * MUL; i++) {
            sum += rayRateArray[i];
        }
        
        std::cout << int(sum * 255) << " ray calculations per frame"<< std::endl;
        
        frames++;
        float fps = frames * 1.0f / (current - start);
        std::cout << fps << " frames per second" << std::endl;
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        keys[key] = true;
        else if (action == GLFW_RELEASE)
        keys[key] = false;
    }
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
