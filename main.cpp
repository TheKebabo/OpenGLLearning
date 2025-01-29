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
GLFWwindow* configGLFW();
void configBuffers(unsigned& VBO, unsigned& EBO, unsigned& VAO, const std::vector<float>& vertices, const std::vector<unsigned>& indices);
void loadTexture(unsigned& texture, std::string imagePath, GLenum sWrap, GLenum tWrap, GLenum minFilter, GLenum maxFilter);

// CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


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

    // BUILD & COMPILE SHADER PROGRAM
    // ------------------------------
    Shader* mainShader = new Shader("src//vertexShader.vs", "src//fragmentShader.fs");
    
    // INIT VERTEX & INDEX DATA
    // ------------------------
    std::vector<float> vertices = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,       // Top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,       // Bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,       // Bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f        // Top left 
    };

    std::vector<unsigned> indices = {
        0, 1, 3,   // First triangle
        1, 2, 3,   // Second triangle
    };

    // CONFIG VBO, EBO, VAO
    // --------------------
    unsigned VBO, EBO, VAO;
    configBuffers(VBO, EBO, VAO, vertices, indices);

    // LOAD TEXTURES
    // -------------
    unsigned texture1, texture2;
    loadTexture(texture1, "textures//wall.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    loadTexture(texture2, "textures//face2.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Activate the shader program
    mainShader->use();
    // Set each uniform sampler to the correct texture unit
    mainShader->setInt("texture1", 0);
    mainShader->setInt("texture2", 1);

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // INPUT
        // -----
        processInput(window);

        // RENDER
        // ------
        // Clear colour buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind textures to corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // Render triangle(s)
        glBindVertexArray(VAO); // Binds the defined VAO (and automatically the EBO) so OpenGL correctly uses vertex data
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    // Draws index data in EBO, using VAO configs, as a triangle primitive(s)

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
GLFWwindow* configGLFW()
{
    if (!glfwInit())
    {
        std::cout << "Failed to init GLFW" << std::endl;
        return NULL;
    }
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // FUNCTION IS CALLED ON WINDOW RESIZE
    return window;
}

// INIT VERTEX BUFFER (VBO), INIT INDEX DRAWING BUFFER (EBO), AND CONFIG VERTEX ATTRIBUTES (VAO)
// ---------------------------------------------------------------------------------------------
void configBuffers(unsigned& VBO, unsigned& EBO, unsigned& VAO, const std::vector<float>& vertices, const std::vector<unsigned>& indices)
{
    // INIT & BIND VAO THAT STORES STATE CONFIGS FOR SUPPLYING INTERPRETABLE VERTEX DATA TO OPENGL
    glGenVertexArrays(1, &VAO); // // Generates the object and stores the resulting id in passed in integer
    glBindVertexArray(VAO); // Binds 'VAO' as current active vertex array object

    // INIT, BIND & SET VBO THAT STORES MANY VERTICES IN GPU MEM FOR SPEEDY GPU ACCESS
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Binds newly created object to the correct buffer type, which when updated/configured will update 'VBO' (as seen below)
    glBufferData(GL_ARRAY_BUFFER, size(vertices) * sizeof(float), vertices.data(), GL_STATIC_DRAW);  // Copies vertex data into the buffer

    // INIT, BIND & SET EBO THAT STORES INDEX DATA
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size(indices) * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    // CONFIG VAO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                     // Describes to OpenGL how to interpet vertex POSITION data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));   // Describes to OpenGL how to interpet vertex COLOUR data
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));   // Describes to OpenGL how to interpet vertex TEXTURE data
    // Enable vertex attributes at location = n, since they are disabled by default
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

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
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)   // GLFW_RELEASE RETURNED FROM GetKey IF NOT PRESSED 
        glfwSetWindowShouldClose(window, true);
}


// CALLBACK FUNC THAT SETS SIZE OF RENDERING SPACE WITH RESPECT TO WINDOW OBJ
// --------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);    
}