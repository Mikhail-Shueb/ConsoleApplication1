#pragma once

#include "SceneNode.h"
#include "Camera.h"
#include <memory>
#include <glm/glm.hpp>

class Table {
public:
    std::shared_ptr<SceneNode> node;
    glm::vec3 position;

    Table(std::shared_ptr<Mesh> mesh, Shader* shader);

    // Move a mesa usando o arrasto do botao esquerdo do rato relativo a camara
    void moveWithMouse(float deltaX, float deltaY, const Camera& camera);

    void resetPosition();

private:
    void updateTransform();
};
