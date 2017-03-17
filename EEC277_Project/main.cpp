#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <string>
#include <array>

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
#define INIT_ITERATION_NUM   6
#define INIT_DISTANCE        10.0f
#define PI                   3.14159

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
    bool doStandardTest;
} TestStruct;

TestStruct testStruct;

// Window dimensions
const GLuint WIDTH = 1024, HEIGHT = 768;

// Camera
Camera  camera(glm::vec3(0.0f, 4.0f, INIT_DISTANCE));
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
const float distances[] = {10.0f, 13.0f, 16.0f, 19.0f, 22.0f, 25.0f, 28.0f, 31.0f};

const char usageString[] = {"\
[-n]\tSet number of spheres\n \
[-i]\tSet number of iterations\n \
[-p]\tRemove a plane from the scene\n \
[-m]\tDisable light movement\n \
[-r]\tDisable refraction\n \
[-o]\tTurn off ray rate calculation\n \
[-nt]\tDo number test\n \
[-it]\tDo iteration test\n \
[-dt]\tDo distance test\n \
[-st]\tDo standard test\n\n"};

void usage(const char *progName)
{
    fprintf(stderr," %s usage:\n %s \n", progName, usageString);
    fflush(stderr);
}

void parseArgs(int argc, char **argv, TestStruct *testStruct) {
    int i = 1;
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
        else if (strcmp(argv[i],"-p") == 0) // Without plane
        {
            testStruct->withPlane = false;
        }
        else if (strcmp(argv[i],"-m") == 0) // Fixed light
        {
            testStruct->lightMoving = false;
        }
        else if (strcmp(argv[i],"-r") == 0) // No refraction
        {
            testStruct->canRefract = false;
        }
        else if (strcmp(argv[i],"-o") == 0) // Turn off ray calculation
        {
            if(!(testStruct->doIterationTest || testStruct->doDistanceTest || testStruct->doNumberTest || testStruct->doStandardTest))
                testStruct->turnOffRayCalculation = true;
        }
        else if (strcmp(argv[i],"-nt") == 0) // Do number testing
        {
            // Do one test at a time
            if(!testStruct->doIterationTest && !testStruct->doDistanceTest && !testStruct->doStandardTest)
                testStruct->doNumberTest = true;
        }
        else if (strcmp(argv[i],"-it") == 0) // Do iteration testing
        {
            // Do one test at a time
            if(!testStruct->doNumberTest && !testStruct->doDistanceTest && !testStruct->doStandardTest)
                testStruct->doIterationTest = true;
        }
        else if (strcmp(argv[i],"-dt") == 0) // Do distance testing
        {
            // Do one test at a time
            if(!testStruct->doNumberTest && !testStruct->doIterationTest && !testStruct->doStandardTest)
                testStruct->doDistanceTest = true;
        }
        else if (strcmp(argv[i],"-st") == 0) // Do standard testing
        {
            // Do one test at a time
            if(!testStruct->doNumberTest && !testStruct->doIterationTest && !testStruct->doDistanceTest)
                testStruct->doStandardTest = true;
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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ray Tracing", nullptr, nullptr);
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
    testStruct.withPlane = true;
    testStruct.lightMoving = true;
    testStruct.canRefract = true;
    testStruct.turnOffRayCalculation = false;
    testStruct.doNumberTest = false;
    testStruct.doDistanceTest = false;
    testStruct.doIterationTest = false;
    testStruct.doStandardTest = false;
    
    parseArgs(argc, argv, &testStruct);
    
    int num_of_test = 0;
    
    if(testStruct.nums > MAX_SPHERE_NUM) { // Check if sphere number exceeds limit
        fprintf(stderr, "Too many spheres!\n");
        exit(EXIT_FAILURE);
    }
    if(testStruct.iterations > MAX_ITERATION_NUM) { // Check if sphere number exceeds limit
        fprintf(stderr, "Too many iterations!\n");
        exit(EXIT_FAILURE);
    }
    
    if(testStruct.doNumberTest) {
        testStruct.nums = numbers[num_of_test];
    }
    
    if(testStruct.doIterationTest) {
        testStruct.iterations = iterations[num_of_test];
    }
    
    // Standard Test:
    // 125 Spheres
    // 6 Iterations
    // Camera distance 10
    // Has plane
    // Light moving
    // Can Refract
    // Calculate ray count
    // Resolution 800*600
    // if(testStruct.doDistanceTest || testStruct.doIterationTest || testStruct.doNumberTest || testStruct.doStandardTest) {
    //     testStruct.withPlane = true;
    //     testStruct.lightMoving = true;
    //     testStruct.canRefract = true;
    //     testStruct.turnOffRayCalculation = false;
    // }
    
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
    
    // Generate two textures for drawing and ray count data
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
    
    std::string filename = "";
    if(testStruct.doNumberTest)
        filename += "NumberTest";
    else if(testStruct.doIterationTest)
        filename += "IterationTest";
    else if(testStruct.doDistanceTest)
        filename += "DistanceTest";
    else if(testStruct.doStandardTest)
        filename += "Standard";

    if(!testStruct.doNumberTest && testStruct.nums != INIT_SPHERE_NUM)
        filename += "_" + std::to_string(testStruct.nums);
    if(!testStruct.canRefract)
        filename += "_NR";
    filename += ".txt";
    
    FILE *df;
    if(testStruct.doNumberTest || testStruct.doStandardTest || testStruct.doDistanceTest || testStruct.doIterationTest) {
        df = fopen(filename.c_str(),"w");
        if(testStruct.doStandardTest)
            fprintf(df, "Spheres\tIterations\tDistance\tPlane\tLight Moving\tRefraction\tFrame Rate\tRay Count\n");
        else
            fprintf(df, "Spheres\tIterations\tDistance\tFrame Rate\tRay Count\n");
    }
    
run:
    // Positions for each spheres
    int scale = int(cbrt(testStruct.nums));
    for(int i = 0; i < testStruct.nums; i++) {
        sp_pos[i] = glm::vec3(-3.0f + 1.5f * (i % scale), 0.5f + 1.5f * (i / (scale * scale)), 0.0f - 1.5f * ((i % (scale * scale)) / scale));
    }
    
    std::cout << testStruct.nums << " Spheres" << std::endl;
    std::cout << testStruct.iterations << " Iterations" << std::endl;
    std::cout << "Camera Distance " << camera.Position.z << std::endl;
    std::cout << "Has plane? " << (testStruct.withPlane ? "Yes" : "No") << std::endl;
    std::cout << "Light moving? " << (testStruct.lightMoving ? "Yes" : "No") << std::endl;
    std::cout << "Can refract? " << (testStruct.canRefract ? "Yes" : "No") << std::endl;
    std::cout << "Ray calculation on? " << (testStruct.turnOffRayCalculation ? "No" : "Yes") << std::endl << std::endl;
    
    while (!glfwWindowShouldClose(window)) {
        GLfloat current = glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;
        
        // Clear the colorbuffer
        glfwPollEvents();
        do_movement();
        
        // Sum of ray count
        float sum = 0;
        
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
        glfwGetCursorPos(window, &xpos, &ypos);
        glUniform2f(glGetUniformLocation(firstPassShader.Program, "cursor"), xpos, ypos);

        //Cursor rotation matrix calculate
        //1.3089 and 0.65 are mearsured number sutable for my machine
        glm::vec2 mouse = (glm::vec2(xpos, ypos) / glm::vec2(WIDTH * MUL, HEIGHT * MUL) * glm::vec2(2.233) - glm::vec2(0.74)) * glm::vec2(WIDTH * MUL / (HEIGHT * MUL), 1.0) * glm::vec2(2.0);
        glm::mat3 rot;
        if(testStruct.doStandardTest || testStruct.doIterationTest || testStruct.doDistanceTest || testStruct.doNumberTest)
            rot = glm::mat3(); // Identity Matrix
        else
            rot = glm::mat3(glm::vec3(sin(mouse.x + PI / 2.0), 0, sin(mouse.x)),glm::vec3(0, 1, 0),glm::vec3(sin(mouse.x + PI), 0, sin(mouse.x + PI / 2.0)));
        glUniformMatrix3fv(glGetUniformLocation(firstPassShader.Program, "rot"), 1, GL_FALSE, glm::value_ptr(rot));

        // Pass sphere array info to fragment shader
        for(int i = 0; i < testStruct.nums; i++) {
            std::string sphere = "spheres[" + std::to_string(i) + "]";
            glUniform4f(glGetUniformLocation(firstPassShader.Program, (sphere + ".position_r").c_str()),
                        sp_pos[i].x, sp_pos[i].y, sp_pos[i].z, 0.5f);
            glUniform3f(glGetUniformLocation(firstPassShader.Program, (sphere + ".material.color").c_str()), 1.0f, 1.0f, 0.8f);
            glUniform3f(glGetUniformLocation(firstPassShader.Program, (sphere + ".material.diff_spec_ref").c_str()), 1.0f, 0.5f, 1.1f);
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
            for(int i = 0; i < WIDTH * HEIGHT * MUL * MUL; i++) {
                sum += rayRateArray[i];
            }
            
            // Only print when 
            if(!testStruct.doStandardTest && !testStruct.doNumberTest && !testStruct.doDistanceTest && !testStruct.doIterationTest)
                std::cout << int(sum * 255) << " rays per frame"<< std::endl;
        }
        
        // Swap the screen buffers
        glfwSwapBuffers(window);

        // Calculate frame rates
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 5.0f){ // If last prinf() was more than 1 sec ago
            // printf and reset timer
            float fps = nbFrames/(currentTime - lastTime);

            nbFrames = 0;
            lastTime = currentTime;
            
            
            if(testStruct.doDistanceTest || testStruct.doIterationTest || testStruct.doNumberTest || testStruct.doStandardTest) {
                if(testStruct.doStandardTest)
                    fprintf(df, "%d\t%d\t%f\t%d\t%d\t%d\t%f\t%d\n", testStruct.nums, testStruct.iterations, camera.Position.z, testStruct.withPlane, testStruct.lightMoving, testStruct.canRefract, fps, int(sum * 255));
                else
                    fprintf(df, "%d\t%d\t%f\t%f\t%d\n", testStruct.nums, testStruct.iterations, camera.Position.z, fps, int(sum * 255));
                break;
            } else {
                std::cout << fps << " frames per second" << std::endl; // If not doing any test, print frame rate per 5 seconds
            }
        }
    }
    
    if(testStruct.doStandardTest) {
        switch(num_of_test++) {
            case 0:
                testStruct.withPlane = false; // Test without plane
                break;
            case 1:
                testStruct.withPlane = true;
                testStruct.lightMoving = false; // Test without moving light
                break;
            case 2:
                testStruct.lightMoving = true;
                testStruct.canRefract = false; // Test without refraction
                break;
            case 3:
                testStruct.canRefract = true;
                testStruct.turnOffRayCalculation = true; // Test without calculating ray counts
                break;
            case 4:
                testStruct.withPlane = false;
                testStruct.lightMoving = false;
                testStruct.canRefract = false; // Disable all features
                break;
            default:
                break;
        }
        if(num_of_test <= 5)
            goto run;
    }
    
    if(testStruct.doNumberTest && num_of_test + 1 < 6) {
        testStruct.nums = numbers[++num_of_test];
        goto run;
    }
    
    if(testStruct.doIterationTest && num_of_test + 1 < 8) {
        testStruct.iterations = iterations[++num_of_test];
        goto run;
    }
    
    if(testStruct.doDistanceTest && num_of_test + 1 < 8) {
        camera.Position.z = distances[++num_of_test];
        goto run;
    }
    
    // Close file
    if(df)
        fclose(df);
    
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
