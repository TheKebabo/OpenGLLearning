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
const unsigned TEXTURE_WIDTH = 512, TEXTURE_HEIGHT = 512;


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
class Time {
public:
    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame
    unsigned fpsCounter = 0;    // Ensures fps isn't written to frequently (cout would slow down program)

    void Update(bool writeFPS)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (writeFPS)
        {
            if(frameCounterMax > 500)
            {
                std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
                fpsCounter = 0;
            } else
                fpsCounter++;
        }
    }
private:
    unsigned frameCounterMax = 750;
} Time;

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

    // BUILD & COMPILE SHADER PROGRAMS
    // -------------------------------
    VFShaderProgram* mainShader = new VFShaderProgram("src//shaders//vertexShader.vs", "src//shaders//fragmentShader.fs");
    ComputeShaderProgram* computeShader = new ComputeShaderProgram("src//shaders//computeShader.glsl");

    mainShader->use();
    mainShader->setInt_w_Name("tex", 0);

    // QUAD VERTEX DATA
    // ----------------
    std::vector<float> vertices = {
        // pos data             // texture data
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f,     // top-left
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,     // bottom-left
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f,     // top-right
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,     // bottom-right
    };

    unsigned VBO, VAO;
    configBuffers(VBO, VAO, vertices);
    
    unsigned refQuadVBO, refQuadVAO;
    configBuffers(refQuadVBO, refQuadVAO, vertices);

    // CREATE CUSTOM TEXTURE
    // ---------------------
    unsigned texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    computeShader->use();
    computeShader->setFloat_w_Name("textureWidth", TEXTURE_WIDTH);

    // REFERENCE QUAD TEXTURE
    // ----------------------
    unsigned refQuadTexture;
    loadTexture(refQuadTexture, "textures//face1.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINE, GL_LINEAR);

    mat4 model = mat4(1.0f);
    mat4 refQuadmodel = mat4(1.0f);
    mat4 view = mat4(1.0f);
    mat4 projection = mat4(1.0f);

    model = translate(model, vec3(-1.5f, 0.0f, -4.5f));
    refQuadmodel = translate(refQuadmodel, vec3(1.5f, 0.0f, -4.5f));

    glEnable(GL_DEPTH_TEST);

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        Time.Update(false);

        // INPUT
        // -----
        processInput(window);

        // RENDER
        // ------        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render main quad
        // ----------------
        // Activate and dispatch the compute shader program
        computeShader->use();
        computeShader->setFloat_w_Loc(0, glfwGetTime());
        glDispatchCompute((unsigned)TEXTURE_WIDTH, (unsigned)TEXTURE_HEIGHT, 1);

        // Make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        mainShader->use();

        view = mainCam.GetViewMatrix();
        projection = glm::perspective(glm::radians(mainCam.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mainShader->setMat4_w_Loc(0, GL_FALSE, value_ptr(model));
        mainShader->setMat4_w_Loc(1, GL_FALSE, value_ptr(view));
        mainShader->setMat4_w_Loc(2, GL_FALSE, value_ptr(projection));

        // Render image to quad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // Render reference quad
        // ---------------------
        mainShader->setMat4_w_Loc(0, GL_FALSE, value_ptr(refQuadmodel));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, refQuadTexture);

        glBindVertexArray(refQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        // -----------------------------------------
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();
    }

    // OPTIONAL: DE-ALLOC ALL RESOURCES ONCE PURPOSES ARE OUTLIVED
    // -----------------------------------------------------------
    unsigned VBOs[2] = { VBO, refQuadVBO };
    unsigned VAOs[2] = { VAO, refQuadVAO };
    glDeleteBuffers(2, VBOs);
    glDeleteVertexArrays(2, VAOs);
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
    glfwSwapInterval(0);    // Gets bigger refresh rate

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);    // Describes to OpenGL how to interpet vertex POSITION data
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));                       // Describes to OpenGL how to interpet vertex TEXTURE data
    // Enable vertex attributes at location = n, since they are disabled by default
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

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
        mainCam.ProcessKeyboard(FORWARD, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mainCam.ProcessKeyboard(BACKWARD, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mainCam.ProcessKeyboard(RIGHT, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mainCam.ProcessKeyboard(LEFT, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        mainCam.ProcessKeyboard(UP, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        mainCam.ProcessKeyboard(DOWN, Time.deltaTime);
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