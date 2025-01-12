#include <iostream>
#include <cmath>
#include <vector>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// SETTINGS
// --------
int SCR_WIDTH = 800, SCR_HEIGHT = 600; 


// FUNCTIONS
// ---------
// Utilities
GLFWwindow* ConfigGLFW();
void ConfigBuffers(unsigned& VBO, unsigned& EBO, unsigned& VAO, const std::vector<float>& vertices, const std::vector<unsigned>& indices);

// CALLBACKS
void Framebuffer_size_callback(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);


int main()
{
    // GLFW: INIT & CONFIG
    // -------------------
    GLFWwindow* window = ConfigGLFW();
    if (window == NULL) return -1;

    // INIT GLAD, WHICH MANAGES FUNCTION POINTERS FOR OPENGL
    // -----------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // BUILD & COMPILE SHADER PROGRAM
    // ------------------------------
    Shader* mainShader = new Shader("vertexShader.vs", "fragmentShader.fs");
    
    // INIT VERTEX & INDEX DATA
    // ------------------------
    std::vector<float> vertices = {
         0.5f,  0.5f, 0.0f,   // Top right
         0.5f, -0.5f, 0.0f,   // Bottom right
        -0.5f, -0.5f, 0.0f,   // Bottom left
    };

    std::vector<unsigned> indices = {
        0, 1, 2,   // First triangle
    };

    // INIT TEXTURE DATA
    // -----------------
    float texCoords[] = {
        vertices[0] + 0.5f, vertices[1] + 0.5f,
        vertices[3] + 0.5f, vertices[4] + 0.5f,
        vertices[6] + 0.5f, vertices[7] + 0.5f
    };

    // Config texture wrapping for s & t axes (x & y)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Config upscaling and downscaling texture filtering methods
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Downscaling - nearest neighbour
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // Upscaling - bilinear filtering

    unsigned VBO, EBO, VAO;
    ConfigBuffers(VBO, EBO, VAO, vertices, indices);

    // DRAW IN WIREFRAME
    // -----------------
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // The fronts and backs of polygons will be rasterised as lines

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // INPUT
        // -----
        ProcessInput(window);

        // RENDER
        // ------
        // Clear colour buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Activate the shader program
        mainShader->use();

        // Render triangle
        glBindVertexArray(VAO); // Binds the defined VAO (and automatically the EBO) so OpenGL correctly uses vertex data
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);    // Draws index data in EBO, using VAO configs, as a triangle primitive
        glBindVertexArray(0); // Unbinds VAO

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        // -----------------------------------------
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();
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
// -------------------
GLFWwindow* ConfigGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
    glfwSetFramebufferSizeCallback(window, Framebuffer_size_callback);  // FUNCTION IS CALLED ON WINDOW RESIZE
    return window;
}

// INIT VERTEX BUFFER (VBO), INIT INDEX DRAWING BUFFER (EBO), AND CONFIG VERTEX ATTRIBUTES (VAO)
// ---------------------------------------------------------------------------------------------
void ConfigBuffers(unsigned& VBO, unsigned& EBO, unsigned& VAO, const std::vector<float>& vertices, const std::vector<unsigned>& indices)
{
    // INIT & BIND VAO THAT STORES STATE CONFIGS FOR SUPPLYING INTERPRETABLE VERTEX DATA TO OPENGL
    glGenVertexArrays(1, &VAO); // // Generates the object and stores the resulting id in passed in integer
    glBindVertexArray(VAO); // Binds 'VAO' as current active vertex array object

    // INIT, BIND & SET VBO THAT STORES MANY VERTICES IN GPU MEM FOR SPEEDY GPU ACCESS
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Binds newly created object to the correct buffer type, which when updated/configured will update 'VBO' (as seen below)
    glBufferData(GL_ARRAY_BUFFER, size(vertices), &vertices, GL_STATIC_DRAW);  // Copies vertex data into the buffer

    // INIT, BIND & SET EBO THAT STORES INDEX DATA
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size(indices), &indices, GL_STATIC_DRAW);

    // CONFIG VAO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);   // Describes to OpenGL how to interpet vertex POSITION data
    glEnableVertexAttribArray(0);   /// Enables vertex attribute at location = 0, since they are disabled by default

    // UNBIND VBO FROM CURRENT ACTIVE BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); // This is allowed, the call to glVertexAttribPointer registered 'VBO' as the vertex attribute's bound VBO, so can safely unbind after
}


// PROCESSES INPUT BY QUERING GLFW ABOUT CURRENT FRAME
void ProcessInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)   // GLFW_RELEASE RETURNED FROM GetKey IF NOT PRESSED 
        glfwSetWindowShouldClose(window, true);
}


// CALLBACK FUNC THAT SETS SIZE OF RENDERING SPACE WITH RESPECT TO WINDOW OBJ
// --------------------------------------------------------------------------
void Framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);    
}