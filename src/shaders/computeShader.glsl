#version 430 core

layout (local_size_x = 1) in;

// constants
const float GRAV_CONSTANT = 1000.0;
const float PARTICLE_MASS = 1.0;
const float INV_PARTICLE_MASS = 1.0 / PARTICLE_MASS;  // saves on calculation time

// inputs/outputs
layout (std430, binding = 0) buffer Pos {   // An SSBO (input & output of compute shader)
    vec4 Position[];
};
layout (std430, binding = 1) buffer Vel {
    vec4 Velocity[];
};

// uniforms
uniform float GravMasses[1];    // Masses of all the gravitational objects
uniform vec3 GravPositions[1];  // Positions of all the objects
uniform float dt;   // Small change in time


void main() {
    uint idX = gl_GlobalInvocationID.x;   // Current work group x position

    // grab particle pos from buffer
    vec3 position = Position[idX].xyz;

    // calculate resultant attraction from all object masses (excl. other particles) 
    vec3 resultantForce = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < length(GravMasses); i++) {
        resultantForce += calcForce(position, GravMasses[i], GravPositions[i]);
    }

    // approximate new pos and vel with constant acceleration formulas
    vec3 acceleration = resultantForce * INV_PARTICLE_MASS;
    Position[idX] = vec4(position + Velocity[idX].xyz * dt + 0.5 * acceleration * dt * dt, 1.0);
    Velocity[idX] = vec4(Velocity[idX].xyz + acceleration * dt, 0.0);
}

vec3 calcForce(vec3 particlePosition, float gravMass, float gravPosition) {
    vec3 displacement = gravMassPosition - particlePosition;
    float dist = length(displacement);
    float magnitude = GRAV_CONSTANT * gravMass * PARTICLE_MASS / (dist * dist);

    return magnitude * displacement;
}