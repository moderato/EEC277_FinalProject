#include <stdio.h>
#include <stdlib.h>
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

#define MAX_SPHERE_NUM       339    // Maximum number of spheres allowed under 4096 uniform constrains
#define MAX_ITERATION_NUM    16
#define INIT_SPHERE_NUM      125
#define INIT_ITERATION_NUM   10

// Define a struct storing test parameters
typedef struct {
    int nums;
    int iterations;
    
    bool withPlane;
    bool lightMoving;
    bool canRefract;
    bool turnOffRayCalculation;
    bool doNumberTest;
    bool doIterationTest;
    bool doDistanceTest;
} TestStruct;

TestStruct testStruct;

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera  camera(glm::vec3(0.0f, 4.0f, 20.0f));
GLfloat lastX  =  WIDTH;
GLfloat lastY  =  HEIGHT;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;   // Time between current frame and last frame
GLfloat lastFrame = 0.0f;   // Time of last frame

// Sphere array
glm::vec3 sp_pos[MAX_SPHERE_NUM];

// Test parameter arrays
const int numbers[] = {1, 8, 27, 64, 125, 216};
const int iterations[] = {2, 4, 6, 8, 10, 12, 14, 16};
const float distances[] = {2.0f, 5.0f, 8.0f, 11.0f};

const char usageString[] = {"\
[-n]\tSet number of spheres\n \
[-i]\tSet number of iterations\n \
[-p]\tAdd a plane to the scene\n \
[-m]\tEnable light movement\n \
[-r]\tEnable refraction calculation\n \
[-o]\tTurn off ray rate calculation\n \
+[-nt]\tDo number test \
+[-it]\tDo iteration test \
+[-dt]\tDo distance test\n\n"};

void usage(const char *progName)
{
  fprintf(stderr," %s usage:\n %s \n", progName, usageString);
  fflush(stderr);
}

void parseArgs(int argc, char **argv, TestStruct *testStruct) {
    int i=1;
    argc--;
    while (argc > 0)
    {
        if (strcmp(argv[i],"-n") == 0) // Change sphere number
        {
            i++;
            argc--;
            testStruct->nums = atoi(argv[i]);
        }
        else if (strcmp(argv[i],"-i") == 0) // Change iteration number
        {
            i++;
            argc--;
            testStruct->iterations = atoi(argv[i]);
        }
        else if (strcmp(argv[i],"-p") == 0) // With plane
        {
            testStruct->withPlane = true;
        }
        else if (strcmp(argv[i],"-m") == 0) // Moving light
        {
            testStruct->lightMoving = true;
        }
        else if (strcmp(argv[i],"-r") == 0) // Use refraction
        {
            testStruct->canRefract = true;
        }
        else if (strcmp(argv[i],"-o") == 0) // Turn off ray calculation
        {
            testStruct->turnOffRayCalculation = true;
        }
	else if (strcmp(argv[i],"-nt") == 0) // Do number testing
        {
            if(!testStruct->doIterationTest && !testStruct->doDistanceTest)
                testStruct->doNumberTest = true;
        }
        else if (strcmp(argv[i],"-it") == 0) // Do iteration testing
        {
            if(!testStruct->doNumberTest && !testStruct->doDistanceTest)
                testStruct->doIterationTest = true;
        }
        else if (strcmp(argv[i],"-dt") == 0) // Do distance testing
        {
            if(!testStruct->doNumberTest && !testStruct->doIterationTest)
                testStruct->doDistanceTest = true;
        }
        else
        {
            fprintf(stderr,"Unrecognized argument: %s \n", argv[i]);
            usage(argv[0]);
            exit(-1);
        }
        i++;
        argc--;
    }
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

int main(int argc, char **argv)
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

    // Initialize parameters and parse arguments
    testStruct.nums = INIT_SPHERE_NUM;
    testStruct.iterations = INIT_ITERATION_NUM;
    testStruct.withPlane = false;
    testStruct.lightMoving = false;
    testStruct.canRefract = false;
    testStruct.turnOffRayCalculation = false;

    testStruct.doNumberTest = false;
    testStruct.doDistanceTest = false;
    testStruct.doIterationTest = false;
    
    parseArgs(argc, argv, &testStruct);
    
    if(testStruct.nums > MAX_SPHERE_NUM) { // Check if sphere number exceeds limit
        fprintf(stderr, "Too many spheres!\n");
        exit(EXIT_FAILURE);
    }
    if(testStruct.iterations > MAX_ITERATION_NUM) { // Check if sphere number exceeds limit
        fprintf(stderr, "Too many iterations!\n");
        exit(EXIT_FAILURE);
    }
    
    // Positions for each spheres
    for(int i = 0; i < testStruct.nums; i++) {
        sp_pos[i] = glm::vec3(-3.0f + 1.5f * (i % 5), 0.5f + 1.5f * (i / 25), 0.0f + 1.5f * ((i % 25) / 5));
    }
    
    // Initialize frame number and start time;
    //int frames = 0;
    //float start = glfwGetTime();
    
    // Array to store ray calculation count data
    GLfloat* rayRateArray = new GLfloat[WIDTH * MUL * HEIGHT * MUL];
    
    
    
    // Two arrays both containing two triangles to cover the whole window for the first pass and second pass, respectively
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
    
    
    
    /******************** Vertex arrays and buffers for two passes ********************/
    // First pass VBO and VAO
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
    
    // Second pass VBO and VAO
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
    
    
    
    /******************** Frame Buffer Object and textures ********************/
    // FBO
    GLuint FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    
    // Generate two textures for drawing and ray calculation count data
    GLuint image, data;
    glGenTextures(1, &image);
    glGenTextures(1, &data);
    
    // Image texture
    glBindTexture(GL_TEXTURE_2D, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH * MUL, HEIGHT * MUL, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image, 0);
    
    // Data texture
    glBindTexture(GL_TEXTURE_2D, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH * MUL, HEIGHT * MUL, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, data, 0);
    
    // Render to multiple textures
    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, DrawBuffers);
    
    // Check FBO validity
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    
    // Define the viewport dimensions
    glViewport(0, 0, MUL * WIDTH, MUL * HEIGHT);
    
    // OpenGL options
    glEnable(GL_DEPTH_TEST);
    
    // Build and compile our shader programs
    // On Mac OS with Xcode we use absolute path, while on other IDE relative path is allowed
    Shader firstPassShader("first_pass.vs",
                           "first_pass.frag");
    Shader secondPassShader("second_pass.vs",
                            "second_pass.frag");
    
    
    std::cout << "Tested on " << glGetString(GL_RENDERER) << 
                " using " << glGetString(GL_VERSION) << std::endl;
    
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    while (!glfwWindowShouldClose(window)) {
        GLfloat current = glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;
        
        // Clear the colorbuffer
        glfwPollEvents();
        do_movement();
        
        /******************** First pass. Render to two textures attached to FBO. ********************/
        // Bind self-created FBO
        if(testStruct.turnOffRayCalculation)
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        else
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        
        // Clear window
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use the first pass shader and bind first pass VAO
        firstPassShader.Use();
        glBindVertexArray(first_pass_VAO);
        
        // Create camera transformations
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        
        // Pass uniforms to first pass fragment shader
        glUniform3f(glGetUniformLocation(firstPassShader.Program, "resolution"), WIDTH * MUL, HEIGHT * MUL, 0);
        glUniform1i(glGetUniformLocation(firstPassShader.Program, "num_spheres"), testStruct.nums);
        glUniform1i(glGetUniformLocation(firstPassShader.Program, "iterations"), testStruct.iterations);
        glUniform3f(glGetUniformLocation(firstPassShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3f(glGetUniformLocation(firstPassShader.Program, "light_direction"),
                    -1.0f + 4.0f * cos(current) * testStruct.lightMoving, 1.5f, 1.0f + 4.0f * sin(current) * testStruct.lightMoving);
        glUniform1i(glGetUniformLocation(firstPassShader.Program, "withPlane"), testStruct.withPlane);
        glUniform1i(glGetUniformLocation(firstPassShader.Program, "canRefract"), testStruct.canRefract);
        double xpos, ypos;
	glfwGetCursorPos(window,&xpos, &ypos);
	glUniform4f(glGetUniformLocation(firstPassShader.Program, "cursor"), xpos, ypos, xpos, ypos);

        //Cursor rotation matrix calculate//1.3089 and 0.65 are mearsured number sutable for my machine
	glm::vec2 mouse = (glm::vec2(xpos,ypos)/glm::vec2(WIDTH * MUL, HEIGHT * MUL)*glm::vec2(1.3089)-glm::vec2(0.65))*glm::vec2(WIDTH * MUL/HEIGHT * MUL,1.0)*glm::vec2(2.0);
        glm::mat3 rot = glm::mat3(glm::vec3(sin(mouse.x+3.14159/2.0),0,sin(mouse.x)),glm::vec3(0,1,0),glm::vec3(sin(mouse.x+3.14159),0,sin(mouse.x+3.14159/2.0)));
	glUniformMatrix3fv(glGetUniformLocation(firstPassShader.Program, "rot"), 1, GL_FALSE, glm::value_ptr(rot));

        // Pass sphere array info to fragment shader
        for(int i = 0; i < testStruct.nums; i++) {
            std::string sphere = "spheres[" + std::to_string(i) + "]";
            glUniform4f(glGetUniformLocation(firstPassShader.Program, (sphere + ".position_r").c_str()),
                        sp_pos[i].x, sp_pos[i].y, sp_pos[i].z, 0.5f);
            glUniform3f(glGetUniformLocation(firstPassShader.Program, (sphere + ".material.color").c_str()), 1.0f, 1.0, 0.8f);
            glUniform3f(glGetUniformLocation(firstPassShader.Program, (sphere + ".material.diff_spec_ref").c_str()), 1.0f, 0.5f, 1.1);
        }
        
        // Draw two triangle to cover the window and detach vertex array
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        // No second pass if ray calculation turned off.
        /******************** Second pass. Draw image texture to default frame buffer  ********************/
        if(!testStruct.turnOffRayCalculation) {
            // Bind default frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // Clear window
            glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Draw Screen with image texture
            secondPassShader.Use();
            glBindVertexArray(second_pass_VAO);
            glBindTexture(GL_TEXTURE_2D, image);    // Use the color attachment texture as the texture of the quad plane
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            
            // Read data from data texture
            glBindTexture(GL_TEXTURE_2D, data);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, rayRateArray);
            glBindTexture(GL_TEXTURE_2D, 0);
            
            // Sum up ray calculation count
            float sum = 0;
            for(int i = 0; i < WIDTH * HEIGHT * MUL * MUL; i++) {
                sum += rayRateArray[i];
            }
            std::cout << int(sum * 255) << " ray calculations per frame"<< std::endl;
        }

        // Calculate frame rates
	     double currentTime = glfwGetTime();
	     nbFrames++;
	     if ( currentTime - lastTime >= 5.0f ){ // If last prinf() was more than 1 sec ago
		 // printf and reset timer
		 float fps = nbFrames/(currentTime - lastTime);
		 std::cout << fps << " frames per second" << std::endl;
		 nbFrames = 0.0;
		 lastTime += (currentTime - lastTime);
	     }
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    
    // Delete all arrays and buffers and free pointers
    glDeleteVertexArrays(1, &first_pass_VAO);
    glDeleteVertexArrays(1, &second_pass_VAO);
    glDeleteBuffers(1, &first_pass_VBO);
    glDeleteBuffers(1, &second_pass_VBO);
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &image);
    glDeleteTextures(1, &data);
    free(rayRateArray);
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

