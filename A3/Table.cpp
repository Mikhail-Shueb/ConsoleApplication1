#include "Table.h"
#include <glm/gtc/matrix_transform.hpp>

Table::Table(std::shared_ptr<Mesh> mesh, Shader* shader)
    : position(0.0f, -0.075f, 0.0f) {
    // Cor elegante de madeira/ardosia escura para o tampo da mesa
    glm::vec4 tableColor(0.25f, 0.25f, 0.28f, 1.0f);
    node = std::make_shared<SceneNode>("Table", mesh, shader, tableColor);
    updateTransform();
}

void Table::moveWithMouse(float deltaX, float deltaY, const Camera& camera) {
    const float speed = 0.015f;

    // Vetor lateral da camara no plano horizontal (X-Z)
    glm::vec3 right = camera.getRight();
    glm::vec3 flatRight = glm::vec3(right.x, 0.0f, right.z);
    if (glm::length(flatRight) > 0.001f) {
        flatRight = glm::normalize(flatRight);
    } else {
        flatRight = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Vetor frontal da camara no plano horizontal (X-Z)
    glm::vec3 forward = camera.getForward();
    glm::vec3 flatForward = glm::vec3(forward.x, 0.0f, forward.z);
    if (glm::length(flatForward) > 0.001f) {
        flatForward = glm::normalize(flatForward);
    } else {
        flatForward = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    // deltaX positivo = direita; deltaY positivo = cima (afasta da camara)
    position += flatRight * (deltaX * speed);
    position += flatForward * (deltaY * speed);

    updateTransform();
}

void Table::resetPosition() {
    position = glm::vec3(0.0f, -0.075f, 0.0f);
    updateTransform();
}

void Table::updateTransform() {
    node->localTransform = glm::translate(glm::mat4(1.0f), position);
}
