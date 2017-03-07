#include <iostream>
#include <cmath>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
Camera  camera(glm::vec3(0.0f, 2.5f, 12.0f));
GLfloat lastX  =  WIDTH;
GLfloat lastY  =  HEIGHT;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;   // Time between current frame and last frame
GLfloat lastFrame = 0.0f;   // Time of last frame

// Spheres
const int MAX_SPHERE_NUM = 125;
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
    
//    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();
    
    GLfloat g_vertex_buffer_data[] = {
        -1,-1,
        -1, 1,
        1, 1,
        1,-1,
        1, 1,
        -1,-1};
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
//     Define the viewport dimensions
    glViewport(0, 0, 2 * WIDTH, 2 * HEIGHT);
    
    // OpenGL options
    glEnable(GL_DEPTH_TEST);
    
    // Build and compile our shader program
    Shader testShader("/Users/moderato/Desktop/EEC277/EEC277_Project/EEC277_Project/test.vs", "/Users/moderato/Desktop/EEC277/EEC277_Project/EEC277_Project/test.frag");
    
    int num_spheres = 3;
    if(argc > 1)
        num_spheres = atoi(argv[1]);
    num_spheres = 25;
    if(num_spheres > MAX_SPHERE_NUM) {
        fprintf(stderr, "Too many spheres!\n");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < num_spheres; i++) {
        sp_pos[i] = glm::vec3(-3.0f + 1.5f * (i % 5), 0.5f + 1.5f * (i / 25), 0.0f + 1.5f * ((i % 25) / 5));
    }
    
    int frames = 0;
    float start = glfwGetTime();
    
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Clear the colorbuffer
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        testShader.Use();
        glBindVertexArray(VAO);
        
        // Create camera transformations
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(testShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(testShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(glGetUniformLocation(testShader.Program, "iGlobalTime"), glfwGetTime());
        glUniform3f(glGetUniformLocation(testShader.Program, "iResolution"), WIDTH * MUL, HEIGHT * MUL, 0);
        glUniform1i(glGetUniformLocation(testShader.Program, "num_spheres"), num_spheres);
        glUniform3f(glGetUniformLocation(testShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        
        for(int i = 0; i < num_spheres; i++) {
            std::string sphere = "spheres[" + std::to_string(i) + "]";
            glUniform1f(glGetUniformLocation(testShader.Program, (sphere + ".radius").c_str()), 0.5f);
            glUniform3f(glGetUniformLocation(testShader.Program, (sphere + ".position").c_str()), sp_pos[i].x, sp_pos[i].y, sp_pos[i].z);
            glUniform3f(glGetUniformLocation(testShader.Program, (sphere + ".material.color").c_str()), 0.2f, 0.3, 0.8f);
            glUniform1f(glGetUniformLocation(testShader.Program, (sphere + ".material.diffuse").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(testShader.Program, (sphere + ".material.specular").c_str()), 0.5f);
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        frames++;
        float fps = frames * 1.0f / (glfwGetTime() - start);
//        std::cout << fps << " frames per second" << std::endl;

        glfwPollEvents();
        do_movement();
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

//// Is called whenever a key is pressed/released via GLFW
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
//    std::cout << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << std::endl;
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
