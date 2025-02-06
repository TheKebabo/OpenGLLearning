#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include <vf_shader_program.h>
#include <compute_shader_program.h>

class ParticleSystem
{
public:
    ParticleSystem(unsigned _numParticlesX, unsigned _numParticlesY, unsigned _numParitclesZ);
    ~ParticleSystem();
    void Render(const glm::mat4& viewProjection);
    
private:
    unsigned numParticlesX;
    unsigned numParticlesY;
    unsigned numParticlesZ;
    unsigned totalNumParticles;

    GLuint posBuffer;
    GLuint VAO;

    VFShaderProgram* mainShader;
    ComputeShaderProgram* computeShader;

    // Init position buffer and VAO, and bind to SSBO and GL_VERTEX array for compute shader and vertex/fragment shader use
    void initBuffers(glm::vec3* systemCentre);
    // Particle positions calculated in cube centered at the origin
    void initPositions(std::vector<glm::vec4>& positions, glm::vec3& centre);
    void executeComputeShader();
    void renderParticles(const glm::mat4& viewProjection); // No model matrix needed particle positions being determined by compute shader
};

#endif
