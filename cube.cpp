#include "cube.h"

Cube::Cube(const glm::vec3& position, float size, const Material& mat, const std::string& blockType)
        : position(position), size(size), blockType(blockType), Object(mat) {}

Intersect Cube::rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
    glm::vec3 relativeRayOrigin = rayOrigin - position;

    glm::vec3 tMin = (glm::vec3(-0.5f * size) - relativeRayOrigin) / rayDirection;
    glm::vec3 tMax = (glm::vec3(0.5f * size) - relativeRayOrigin) / rayDirection;

    float maxTMin = glm::max(glm::min(tMin.x, tMax.x), glm::max(glm::min(tMin.y, tMax.y), glm::min(tMin.z, tMax.z)));
    float minTMax = glm::min(glm::max(tMin.x, tMax.x), glm::min(glm::max(tMin.y, tMax.y), glm::max(tMin.z, tMax.z)));

    if (maxTMin > minTMax) {
        // No hay intersección
        return Intersect{false};
    }

    float tHit = (maxTMin < 0.0f) ? minTMax : maxTMin;
    glm::vec3 hitPoint = rayOrigin + tHit * rayDirection;

    // Calcular la normal del cubo en el punto de intersección
    glm::vec3 normal(0.0f);
    const float BIAS = 0.0001f;
    for (int i = 0; i < 3; ++i) {
        if (hitPoint[i] < position[i] - 0.5f * size + BIAS) {
            normal[i] = -1.0f;
        } else if (hitPoint[i] > position[i] + 0.5f * size - BIAS) {
            normal[i] = 1.0f;
        }
    }

    return Intersect{true, tHit, hitPoint, glm::normalize(normal)};
}
