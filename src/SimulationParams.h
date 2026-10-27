#pragma once


struct SimulationParams {
    float gravity = 9.81f;
    float windSpeed = 0.0f;
    float windDirection = 0.0f;

    float spawnPosMinX = 0.0f;
    float spawnPosMaxX = 1.0f;
    float spawnPosMinY = 0.0f;
    float spawnPosMaxY = 1.0f;

    float spawnVelMinX = -0.1f;
    float spawnVelMaxX = 0.1f;
    float spawnVelMinY = -0.1f;
    float spawnVelMaxY = 0.1f;

    float spawnMassMin = 0.1f;
    float spawnMassMax = 1.0f;
    float spawnBlueMin = 0.1f;
    float spawnBlueMax = 1.0f;
};