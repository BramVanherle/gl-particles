#pragma once
#include <cstdint>
#include <glm/glm.hpp>


struct alignas(16) Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    alignas(16) glm::vec4 color;
    float mass;
    std::uint32_t respawn;
};
static_assert(sizeof(Particle) % 16 == 0, "Particle struct size must be a multiple of 16 bytes for std430 alignment");
