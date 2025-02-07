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
    void Render(float dt, const glm::mat4& viewProjection);
    
private:
    unsigned numParticlesX;
    unsigned numParticlesY;
    unsigned numParticlesZ;
    unsigned totalNumParticles;

    GLuint posBuffer;
    GLuint velBuffer;
    GLuint VAO;

    // shader uniforms so that the id of uniforms doesn't have to be fetched constantly
    // vertex/fragment
    GLuint modelViewProjectionUniform = INVALID_UNIFORM_LOC;
    GLuint colorUniform = INVALID_UNIFORM_LOC;
    // compute
    GLuint gravMassesUniform = INVALID_UNIFORM_LOC;
    GLuint gravPositionsUniform = INVALID_UNIFORM_LOC;
    GLuint dtUniform = INVALID_UNIFORM_LOC;

    VFShaderProgram* mainShader;
    ComputeShaderProgram* computeShader;

    // Init position buffer and VAO, and bind to SSBO and GL_VERTEX array for compute shader and vertex/fragment shader use
    void initBuffers(glm::vec3* systemCentre);
    // Particle positions calculated in cube centered at the origin
    void initPositions(std::vector<glm::vec4>& positions, glm::vec3& centre);
    void executeComputeShader(float dt);
    void renderParticles(const glm::mat4& viewProjection); // No model matrix needed particle positions being determined by compute shader
};

#endif
