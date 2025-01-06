#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

// SHADERS
// -------
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n";


// SETTINGS
// --------
int SCR_WIDTH = 800, SCR_HEIGHT = 600; 


// CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);


int main()
{
    // GLFW: INIT & CONFIG
    // ------------------
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
        return -1;
    }
    glfwMakeContextCurrent(window); // SETS CREATED WINDOW OBJ AS THE MAIN CONTEXT ON THE CURRENT THREAD
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // FUNCTION IS CALLED ON WINDOW RESIZE

    // INIT GLAD, WHICH MANAGES FUNCTION POINTERS FOR OPENGL
    // -----------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // BUILD & COMPILE SHADER PROGRAM
    // ------------------------------
    // 1. GEN & COMPILE VERTEX SHADER
    unsigned int vertexShader;  // ID of the vertex shader object that OpenGL dynamically compiles
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attach source code
    glCompileShader(vertexShader);  // Compile source code
    // Check for succesful compilation of this shader
    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);   // Queries shader for some target info, in this case if it was successful
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);   // Retrieves compilation error message
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 2. GEN & COMPILE FRAGMENT SHADER
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for succesful compilation of this shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);   // Queries shader for some target info, in this case if it was successful
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);   // Retrieves compilation error message
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // 3. CREATE SHADER PROGRAM, LINKING PREVIOUSLY COMPILED SHADERS
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for successful linking of shader program
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 4. DELETE SHADERS FROM MEMORY (as not needed after shader progam object is created)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // INIT VERTEX & INDEX DATA
    // ----------------
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // Top right
        0.5f, -0.5f, 0.0f,  // Bottom right
        -0.5f, -0.5f, 0.0f,  // Bottom left
        -0.5f,  0.5f, 0.0f   // Top left 
    };
    unsigned int indices[] = {
        0, 1, 3,   // First triangle
        1, 2, 3    // Second triangle
    };

    // INIT VERTEX BUFFER (VBO), INIT INDEX DRAWING BUFFER (EBO), AND CONFIG VERTEX ATTRIBUTES (VAO)
    // ---------------------------------------------------------------------------------------------
    // INIT & BIND VAO THAT STORES STATE CONFIGS FOR SUPPLYING INTERPRETABLE VERTEX DATA TO OPENGL
    unsigned int VAO;   // ID of the object
    glGenVertexArrays(1, &VAO); // // Generates the object and stores the resulting id in passed in integer
    glBindVertexArray(VAO); // Binds 'VAO' as current active vertex array object


    // INIT, BIND & SET VBO THAT STORES MANY VERTICES IN GPU MEM FOR SPEEDY GPU ACCESS
    unsigned int VBO;   
    glGenBuffers(1, &VBO);  

    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Binds newly created object to the correct buffer type, which when updated/configured will update 'VBO' (as seen below)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // Copies vertex data into the buffer

    // INIT, BIND & SET EBO THAT STORES INDEX DATA
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // CONFIG VAO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);   // Describes to OpenGL how to interpet vertex data
    glEnableVertexAttribArray(0);   /// Enables vertex attribute, since they are disabled by default

    // UNBIND VBO FROM CURRENT ACTIVE BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, 0); // This is allowed, the call to glVertexAttribPointer registered 'VBO' as the vertex attribute's bound VBO, so can safely unbind after

    // DRAW IN WIREFRAME
    // -----------------
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // The fronts and backs of polygons will be rasterised as lines

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // INPUT
        // -----
        processInput(window);

        // RENDER
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);    // Sets current active shader program to the one defined
        glBindVertexArray(VAO); // Binds the defined VAO (and automatically the EBO) so OpenGL correctly uses vertex data
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    // Draws index data in EBO, using VAO configs, as a two traingle primitives
        glBindVertexArray(0); // UNBINDS VAO

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();
    }

    // OPTIONAL: DE-ALLOC ALL RESOURCES ONCE PURPOSES ARE OUTLIVED
    // -----------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // GLFW: TERMINATE GLFW, CLEARING ALL PREVIOUSLY ALLOCATED GLFW RESOURCES
    glfwTerminate();
    return 0;
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