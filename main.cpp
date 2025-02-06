#include <iostream>
#include <cmath>
#include <vector>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "particle_system.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;


// SETTINGS
// --------
int SCR_WIDTH = 800, SCR_HEIGHT = 600; 
const unsigned TEXTURE_WIDTH = 1000, TEXTURE_HEIGHT = 1000;


// FUNCTIONS
// ---------
// Utilities
GLFWwindow* configGLFW();
void outputGLLimits();

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


// GLOBALS
// -------
Camera MainCam;
bool ShowFPS = false;
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
    unsigned frameCounterMax = 10000;
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

    glEnable(GL_DEPTH_TEST);

    ParticleSystem particles(50, 50, 50);

    // RENDER LOOP
    // -----------
    while(!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        Time.Update(ShowFPS);

        // INPUT
        // -----
        processInput(window);

        // RENDER
        // ------        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
        
        mat4 model = mat4(1.0f);
        // model = translate(model, vec3())
        mat4 view = MainCam.GetViewMatrix();
        mat4 projection = mat4(1.0f);
        projection = perspective(radians(MainCam.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        particles.Render(projection * view * model);

        // GLFW: POLL & CALL IOEVENTS + SWAP BUFFERS
        // -----------------------------------------
        glfwSwapBuffers(window);    // For reader - search 'double buffer'
        glfwPollEvents();
    }

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

// PROCESSES INPUT BY QUERING GLFW ABOUT CURRENT FRAME
// ---------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)   // GLFW_RELEASE RETURNED FROM GetKey IF NOT PRESSED 
        glfwSetWindowShouldClose(window, true);
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        ShowFPS = !ShowFPS;

    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        MainCam.ProcessKeyboard(FORWARD, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        MainCam.ProcessKeyboard(BACKWARD, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        MainCam.ProcessKeyboard(RIGHT, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        MainCam.ProcessKeyboard(LEFT, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        MainCam.ProcessKeyboard(UP, Time.deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        MainCam.ProcessKeyboard(DOWN, Time.deltaTime);
    
}

void outputGLLimits()
{
    // query limitations
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}	
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;
}


//  SETS SIZE OF RENDERING SPACE WITH RESPECT TO WINDOW OBJ
// --------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    MainCam.Mouse.PrevPos = vec2(width / 2.0f, height / 2.0f);
}

void mouseMoveCallback(GLFWwindow* window, double mouseX, double mouseY)
{
    MainCam.ProcessMouseMovement(mouseX, mouseY, true);
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    MainCam.ProcessMouseScroll(yoffset);
}