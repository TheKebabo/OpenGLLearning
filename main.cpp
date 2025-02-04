#include <iostream>
#include <cmath>
#include <vector>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vf_shader_program.h"
#include "compute_shader_program.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;


// SETTINGS
// --------
int SCR_WIDTH = 800, SCR_HEIGHT = 600; 


// FUNCTIONS
// ---------
// Utilities
GLFWwindow* configGLFW();
void configBuffers(unsigned& VBO, unsigned& VAO, const std::vector<float>& vertices);
void loadTexture(unsigned& texture, std::string imagePath, GLenum sWrap, GLenum tWrap, GLenum minFilter, GLenum maxFilter);

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


// GLOBALS
// -------
Camera mainCam;
struct {
    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame
} TimeStruct;

int main()
{
    // GLFW: INIT & CONFIG
    // -------------------
    GLFWwindow* window = configGLFW();
    if (window == NULL) return -1;

    // INIT GLAD, WHICH MANAGES FUNCTION POINTERS FOR OPENGL
    // -----------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // BUILD & COMPILE SHADER PROGRAM
    // ------------------------------
    VFShaderProgram* mainShader = new VFShaderProgram("src//shaders//vertexShader.vs", "src//shaders//fragmentShader.fs");
    
    // INIT VERTEX DATA
    // ----------------
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };

    // CONFIG VBOs, VAOs
    // -----------------
    unsigned VBO, VAO;
    configBuffers(VBO, VAO, vertices);

    // Activate the shader program
    mainShader->use();

    // OBJECT TRANSFORMATIONS
    // ----------------------
    mat4 model = mat4(1.0f);      // (LOCAL -> WORLD): specifies the local-space transformations of coordinates on an object
    mat4 view = mat4(1.0f);       // (WORLD -> VIEW): specifies the position of the camera relative to world-space coordinates
    mat4 projection = mat4(1.0f); // (VIEW -> CLIP) - specifies how the 3D coordinates should be transformed to a 2D viewport
    
    model = translate(model, vec3(0.0f, 0.0f, -2.5f));
    mainShader->setMat4("model", GL_FALSE, value_ptr(model));

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // INPUT
        // -----
        processInput(window);

        // RENDER
        // ------
        // Clear colour & depth buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        view = mainCam.GetViewMatrix();
        projection = perspective(radians(mainCam.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mainShader->setMat4("view", GL_FALSE, value_ptr(view));
        mainShader->setMat4("projection", GL_FALSE, value_ptr(projection));

        // RENDER LORENZ
        // -------------
        // Render points(s)
        glBindVertexArray(VAO); // Binds the defined VAO (and automatically the EBO if present) so OpenGL correctly uses vertex data
        glDrawArrays(GL_LINE_STRIP, 0, vertices.size());

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        // -----------------------------------------
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();

        // Calculate delta time
        float currentFrame = glfwGetTime();
        TimeStruct.deltaTime = currentFrame - TimeStruct.lastFrame;
        TimeStruct.lastFrame = currentFrame; 
    }

    // OPTIONAL: DE-ALLOC ALL RESOURCES ONCE PURPOSES ARE OUTLIVED
    // -----------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    delete mainShader;

    // GLFW: TERMINATE GLFW, CLEARING ALL PREVIOUSLY ALLOCATED GLFW RESOURCES
    glfwTerminate();
    return 0;
}


// GLFW: INIT & SETUP WINDOW OBJECT
// --------------------------------
GLFWwindow* configGLFW()
{
    if (!glfwInit())
    {
        std::cout << "Failed to init GLFW" << std::endl;
        return NULL;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    // CREATE WINDOW OBJ
    // -----------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window); // SETS CREATED WINDOW OBJ AS THE MAIN CONTEXT ON THE CURRENT THREAD
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);  // FUNCTION IS CALLED ON WINDOW RESIZE

    // Set glfw mouse configs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    return window;
}

// INIT VERTEX BUFFER (VBO), INIT INDEX DRAWING BUFFER (EBO), AND CONFIG VERTEX ATTRIBUTES (VAO)
// ---------------------------------------------------------------------------------------------
void configBuffers(unsigned& VBO, unsigned& VAO, const std::vector<float>& vertices)
{
    // INIT & BIND VAO THAT STORES STATE CONFIGS FOR SUPPLYING INTERPRETABLE VERTEX DATA TO OPENGL
    glGenVertexArrays(1, &VAO); // // Generates the object and stores the resulting id in passed in integer
    glBindVertexArray(VAO); // Binds 'VAO' as current active vertex array object

    // INIT, BIND & SET VBO THAT STORES MANY VERTICES IN GPU MEM FOR SPEEDY GPU ACCESS
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Binds newly created object to the correct buffer type, which when updated/configured will update 'VBO' (as seen below)
    glBufferData(GL_ARRAY_BUFFER, size(vertices) * sizeof(float), vertices.data(), GL_STATIC_DRAW);  // Copies vertex data into the buffer

    // CONFIG VAO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);                     // Describes to OpenGL how to interpet vertex POSITION data (stride = 0, so tightly packed)
    // Enable vertex attributes at location = n, since they are disabled by default
    glEnableVertexAttribArray(0);

    // UNBIND VBO FROM CURRENT ACTIVE BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); // This is allowed, the call to glVertexAttribPointer registered 'VBO' as the vertex attribute's bound VBO, so can safely unbind after
}

// CONFIG + LOAD TEXTURE (IMAGE HAS TO BE MORE > 24 BIT PER PIXEL)
// ---------------------------------------------------------------
void loadTexture(unsigned& texture, std::string imagePath, GLenum sWrap, GLenum tWrap, GLenum minFilter, GLenum maxFilter)
{
    // GEN TEXTURE GLOBJECT
    // --------------------
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // CONFIG WRAPPING & FILTERING
    // ---------------------------
    // Config texture wrapping for s & t axes (x & y)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);

    // Config upscaling and downscaling texture filtering methods
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);   // Downscaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxFilter);   // Upscaling

    // LOAD TEXTURE FROM IMAGE (IMAGE HAS TO BE MORE > 24 BIT PER PIXEL)
    // -----------------------------------------------------------------
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Loads upside-down for some reason
    unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 4);  // Set number of channels to 4 manually (don't know why)
    nrChannels = 4; 
    if (data)   // Use previous image data to load texture
    {
        // Use stbi data to determine image format
        GLenum internalFormat = nrChannels == 4 ? GL_RGBA8 :
            nrChannels == 3 ? GL_RGB8 : 0;
        GLenum dataFormat = nrChannels == 4 ? GL_RGBA :
            nrChannels == 3 ? GL_RGB : 0;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);    // For reader, search 'OpenGL mipmaps'
    }
    else
        std::cout << "Failed to load texture" << std::endl;  

    stbi_image_free(data);  // Free image from memory
    glBindTexture(GL_TEXTURE_2D, 0);
}


// PROCESSES INPUT BY QUERING GLFW ABOUT CURRENT FRAME
// ---------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)   // GLFW_RELEASE RETURNED FROM GetKey IF NOT PRESSED 
        glfwSetWindowShouldClose(window, true);
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mainCam.ProcessKeyboard(FORWARD, TimeStruct.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mainCam.ProcessKeyboard(BACKWARD, TimeStruct.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mainCam.ProcessKeyboard(RIGHT, TimeStruct.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mainCam.ProcessKeyboard(LEFT, TimeStruct.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        mainCam.ProcessKeyboard(UP, TimeStruct.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        mainCam.ProcessKeyboard(DOWN, TimeStruct.deltaTime);
}


//  SETS SIZE OF RENDERING SPACE WITH RESPECT TO WINDOW OBJ
// --------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    mainCam.Mouse.PrevPos = vec2(width / 2.0f, height / 2.0f);
}

void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY)
{
    mainCam.ProcessMouseMovement(mouseX, mouseY, true);
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    mainCam.ProcessMouseScroll(yoffset);
}