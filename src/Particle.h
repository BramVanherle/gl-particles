#pragma once
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>


struct alignas(16) Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    alignas(16) glm::vec4 color;
    float mass;
    std::uint32_t respawn;
    float spawnDelay;
};
static_assert(offsetof(Particle, spawnDelay) == 40, "Particle layout must match the std430 shader struct");
static_assert(sizeof(Particle) == 48, "Particle size must match the std430 shader struct");
