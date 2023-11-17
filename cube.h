#pragma once
#include "object.h"

class Cube : public Object {
public:
    Cube(const glm::vec3& position, float size, const Material& mat, const std::string& blockType);
    Intersect rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const override;

private:
    glm::vec3 position;
    float size;
    std::string blockType;
};
