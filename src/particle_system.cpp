#include <GLAD/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "particle_system.h"

// PUBLICS
ParticleSystem::ParticleSystem(unsigned _numParticlesX = 100, unsigned _numParticlesY = 100, unsigned _numParitclesZ = 100):
    numParticlesX(_numParticlesX), numParticlesY(_numParticlesY), numParticlesZ(_numParitclesZ)
{
    totalNumParticles = numParticlesX * numParticlesY * numParticlesZ;

    // Init shader programs
    mainShader = new VFShaderProgram("src//shaders//vertexShader.vs", "src//shaders//fragmentShader.fs");
    computeShader = new ComputeShaderProgram("src//shaders//computeShader.glsl");

    initBuffers(new glm::vec3(0, 0, 0));
}

ParticleSystem::~ParticleSystem()
{
    glDeleteBuffers(1, &posBuffer);
    glDeleteVertexArrays(1, &VAO);
    delete mainShader;
    delete computeShader;
}

void ParticleSystem::Render(const glm::mat4& viewProjection)
{
    executeComputeShader();
    renderParticles(viewProjection);
}

// PRIVATES
void ParticleSystem::initBuffers(glm::vec3* systemCentre)
{
    std::vector<glm::vec4> positions(totalNumParticles);
    initPositions(positions, *systemCentre);

    glGenBuffers(1, &posBuffer);

    GLuint bufferSize = (int)positions.size() * sizeof(positions[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuffer);   // Binds the buffer to an SSBO at binding index = 0 in the compute shader
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, positions.data(), GL_DYNAMIC_DRAW);  // Send buffer data to SSBO target

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);   // Same buffer is bound as an SSBO and as 'GL_ARRAY_BUFFER' in normal vertex shader pipeline
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);   // Unbind VAO
}

void ParticleSystem::initPositions(std::vector<glm::vec4>& positions, glm::vec3& centre)
{
    float INIT_CUBE_SIZE = 5.0f;

    // Distance between adjacent particles
    float dx = INIT_CUBE_SIZE / (float)numParticlesX;
    float dy = INIT_CUBE_SIZE / (float)numParticlesY;
    float dz = INIT_CUBE_SIZE / (float)numParticlesZ;

    // Centre at 'centre'
    glm::mat4 translation = glm::mat4(1.0f);
    translation = glm::translate(translation, -centre);

    // Iterate through all init positions of particles
    int particleI = 0;
    for (unsigned x = 0; x < numParticlesX; ++x) {
        for (unsigned y = 0; y < numParticlesX; ++y) {
            for (unsigned z = 0; z < numParticlesZ; ++z) {
                glm::vec4 pos(dx * x, dy * y, dz * z, 1.0f);
                pos = translation * pos;
                positions[particleI] = pos;
                particleI++;
            }
        }
    }
}

void ParticleSystem::executeComputeShader()
{
    computeShader->use();
    glDispatchCompute(totalNumParticles, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Wait for execution to complete so data isn't overwritten
}

void ParticleSystem::renderParticles(const glm::mat4& viewProjection)
{
    mainShader->use();
    mainShader->setMat4_w_Loc(0, false, glm::value_ptr(viewProjection));
    mainShader->set4Floats_w_Loc(0, 0.0f, 0.0f, 0.0f, 0.2f);    // Set colour of particles in fragment shader

    glEnable(GL_BLEND); // don't rlly understand blending

    // Render particles themselves
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, totalNumParticles);
    glBindVertexArray(0);
}