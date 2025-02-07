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

    // Get uniform ids
    modelViewProjectionUniform = mainShader->getUniformLocation("ModelViewProjection");
    colorUniform = mainShader->getUniformLocation("Color");
    gravMassesUniform = computeShader->getUniformLocation("GravMasses");
    gravPositionsUniform = computeShader->getUniformLocation("GravPositions");
    dtUniform = computeShader->getUniformLocation("dt");

    numGravObjects = 2;

    initBuffers(new glm::vec3(0, 0.0f, -15.0f));
}

ParticleSystem::~ParticleSystem()
{
    glDeleteBuffers(1, &posBuffer);
    glDeleteVertexArrays(1, &VAO);
    delete mainShader;
    delete computeShader;
}

void ParticleSystem::Render(float dt, const glm::mat4& viewProjection)
{
    executeComputeShader(dt);
    renderParticles(viewProjection);
}

// PRIVATES
void ParticleSystem::initBuffers(glm::vec3* systemInitCentre)
{
    // Init data
    // ---------
    std::vector<glm::vec4> positions(totalNumParticles);
    initPositions(positions, *systemInitCentre);
    std::vector<glm::vec4> velocities(totalNumParticles, glm::vec4(0.0f));

    // Create buffers and bind to GL objects
    // -------------------------------------
    glGenBuffers(1, &posBuffer);
    glGenBuffers(1, &velBuffer);

    // pos buffer
    GLuint bufferSize = (int)positions.size() * sizeof(positions[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuffer);   // Binds the buffer to an SSBO at binding index = 0 in the compute shader
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, positions.data(), GL_DYNAMIC_DRAW);  // Send buffer data to SSBO target

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, velocities.data(), GL_DYNAMIC_COPY); // 'copy' used for some reason?

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
    translation = glm::translate(translation, -(glm::vec3(INIT_CUBE_SIZE) / 2.0f - centre));

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

void ParticleSystem::initGravObjects()
{
    for (int i = 0; i < numGravObjects; ++i)
    {
        gravMasses.push_back(5.0f);
        gravPositions.push_back(glm::vec3((float)i * 3.0f, 0.0f, -5.0f));
    }
}

void ParticleSystem::executeComputeShader(float dt)
{
    computeShader->use();
    // Set all grav object uniforms
    glUniform1fv(gravMassesUniform, numGravObjects, gravMasses.data());
    for (int i = 0; i < numGravObjects; ++i)    // loop needed for array of vec3s
    {
        GLint gravPosLocI = glGetUniformLocation(computeShader->ID, "GravPositions[i]");
        glUniform3f(gravPosLocI, gravPositions[i].x, gravPositions[i].y, gravPositions[i].z);
    }
    
    computeShader->setFloat_w_Loc(dtUniform, dt);
    glDispatchCompute(totalNumParticles, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Wait for execution to complete so data isn't overwritten
}

void ParticleSystem::renderParticles(const glm::mat4& viewProjection)
{
    mainShader->use();
    mainShader->setMat4_w_Loc(0, false, glm::value_ptr(viewProjection));
    mainShader->set4Floats_w_Loc(1, 0.15f, 0.15f, 0.15f, 0.7f);    // Set colour (location = 1) of particles in fragment shader

    glEnable(GL_BLEND); // don't rlly understand blending

    // Render particles themselves
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, totalNumParticles);
    glBindVertexArray(0);
}