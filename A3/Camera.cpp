#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 target, float distance, glm::quat initialOrientation, bool isPerspective)
    : target(target), distance(distance), orientation(initialOrientation),
      isPerspective(isPerspective), fov(45.0f), aspect(1.0f), nearPlane(0.1f), farPlane(100.0f) {
}

void Camera::orbit(float deltaX, float deltaY) {
    const float sensitivity = 0.005f;

    // Criamos as rotacoes elementares estritamente atraves de quaternioes:
    // - qPitch: roda em torno do eixo X local da camara (eixo horizontal do ecra)
    // - qYaw:   roda em torno do eixo Y local da camara (eixo vertical do ecra)
    glm::quat qPitch = glm::angleAxis(deltaY * sensitivity, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qYaw   = glm::angleAxis(-deltaX * sensitivity, glm::vec3(0.0f, 1.0f, 0.0f));

    // Multiplicamos no espaco local da camara e normalizamos o quaterniao resultante:
    // orientation = normalize(orientation * qPitch * qYaw)
    // 
    // VANTAGEM MATEMATICA CRUCIAL:
    // Esta formulacao opera inteiramente no espaco de rotacoes 3D SO(3) através
    // da esfera 4D unitaria S^3. Como nao depende de angulos de Euler (yaw/pitch/roll),
    // NUNCA ocorre perda de grau de liberdade (Gimbal Lock), permitindo a camara
    // orbitar suavemente e continuamente sobre qualquer ponto da esfera, incluindo os polos.
    orientation = glm::normalize(orientation * qPitch * qYaw);
}

glm::vec3 Camera::screenToSphere(glm::vec2 mousePos, float width, float height) const {
    if (width <= 0.0f) width = 800.0f;
    if (height <= 0.0f) height = 600.0f;

    // Normalizar para o intervalo [-1, 1] com origem no centro da janela
    float x = (2.0f * mousePos.x - width) / width;
    float y = (height - 2.0f * mousePos.y) / height;

    float rSq = x * x + y * y;
    if (rSq <= 1.0f) {
        return glm::vec3(x, y, std::sqrt(1.0f - rSq));
    } else {
        float invLen = 1.0f / std::sqrt(rSq);
        return glm::vec3(x * invLen, y * invLen, 0.0f);
    }
}

void Camera::orbitArcball(glm::vec2 prevMouse, glm::vec2 currMouse, float screenWidth, float screenHeight) {
    glm::vec3 p0 = screenToSphere(prevMouse, screenWidth, screenHeight);
    glm::vec3 p1 = screenToSphere(currMouse, screenWidth, screenHeight);

    glm::vec3 axis = glm::cross(p0, p1);
    float dotVal = glm::clamp(glm::dot(p0, p1), -1.0f, 1.0f);
    float angle = glm::acos(dotVal);

    if (glm::length(axis) > 0.0001f && angle > 0.0001f) {
        axis = glm::normalize(axis);
        // Quaterniao incremental derivado dos dois vetores na esfera
        glm::quat qDelta = glm::angleAxis(angle * 1.5f, axis);
        orientation = glm::normalize(orientation * qDelta);
    }
}

void Camera::zoom(float delta) {
    const float zoomSpeed = 1.0f;
    distance -= delta * zoomSpeed;
    if (distance < 2.0f) distance = 2.0f;
    if (distance > 50.0f) distance = 50.0f;
}

void Camera::toggleProjection() {
    isPerspective = !isPerspective;
}

void Camera::setAspect(float newAspect) {
    aspect = (newAspect > 0.0f) ? newAspect : 1.0f;
}

glm::vec3 Camera::getPosition() const {
    // Posicao no mundo obtida rodando o vetor de recuo (0, 0, distance) pelo quaterniao
    return target + (orientation * glm::vec3(0.0f, 0.0f, distance));
}

glm::vec3 Camera::getForward() const {
    // Vetor de visao (aponta para o alvo, -Z local)
    return glm::normalize(orientation * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 Camera::getRight() const {
    // Vetor lateral (+X local)
    return glm::normalize(orientation * glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 Camera::getUp() const {
    // Vetor para cima (+Y local)
    return glm::normalize(orientation * glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getViewMatrix() const {
    // CONSTRUCAO PURA DA MATRIZ DE VISTA POR QUATERNIOES:
    // View = T(0, 0, -distance) * R(conjugado(orientation)) * T(-target)
    // 
    // Em vez de invocar glm::lookAt (que calcula cross(f, up) e sofre de singularidades
    // e NaN quando a camara olha verticalmente para baixo ou para cima),
    // utilizamos diretamente a matriz ortogonal gerada a partir do quaterniao conjugado.
    // Desta forma, a transformacao e estritamente continua e suave em qualquer orientacao orbital.
    glm::mat4 tTarget = glm::translate(glm::mat4(1.0f), -target);
    glm::mat4 rCam = glm::mat4_cast(glm::conjugate(orientation));
    glm::mat4 tDist = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -distance));
    return tDist * rCam * tTarget;
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (isPerspective) {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    } else {
        // Projecao ortografica proporcional a distancia para manter a mesma escala visual aparente
        float orthoHeight = distance * 0.45f;
        float orthoWidth = orthoHeight * aspect;
        return glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -farPlane, farPlane);
    }
}
