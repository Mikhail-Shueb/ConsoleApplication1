#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Classe Camera Orbital com Quaternioes
 * 
 * Requisitos cumpridos:
 * - Orientacao representada e manipulada exclusivamente por quaternioes (glm::quat).
 * - Ausencia total de Gimbal Lock: nao utiliza angulos de Euler (yaw/pitch/roll)
 *   e a Matriz de Vista e gerada diretamente do quaterniao sem recorrer a lookAt
 *   (evitando singularidades quando o vetor de visao coincide com o vetor up).
 * - Movimento orbital sobre a esfera centrado no alvo (target).
 * - Zoom continuo variando a distancia orbital.
 * - Suporte a projecao Perspetiva e Ortografica com preservacao do aspect ratio.
 */
class Camera {
public:
    glm::vec3 target;
    float distance;
    glm::quat orientation; // Quaterniao unitario de orientacao

    bool isPerspective;
    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    Camera(glm::vec3 target = glm::vec3(0.0f), float distance = 12.0f,
           glm::quat initialOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
           bool isPerspective = true);

    // Orbita esferica usando quaternioes puros a partir do arrasto do rato (sem Gimbal Lock)
    void orbit(float deltaX, float deltaY);

    // Orbita virtual Arcball (Shoemake) mapeando pontos na esfera unitaria 3D
    void orbitArcball(glm::vec2 prevMouse, glm::vec2 currMouse, float screenWidth, float screenHeight);

    // Zoom orbital (altera a distancia ao alvo)
    void zoom(float delta);

    // Alterna entre projecao Perspetiva e Ortografica
    void toggleProjection();

    // Atualizacao da razao de aspeto (aspect ratio) no redimensionamento da janela
    void setAspect(float newAspect);

    // Vetores e posicoes no espaco do mundo derivados do quaterniao
    glm::vec3 getPosition() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    // Matriz de Vista (World -> View) calculada diretamente a partir do quaterniao conjugado
    glm::mat4 getViewMatrix() const;

    // Matriz de Projecao (Perspetiva / Ortografica)
    glm::mat4 getProjectionMatrix() const;

private:
    // Mapeamento de coordenadas de ecra para ponto na esfera unitaria (para Arcball)
    glm::vec3 screenToSphere(glm::vec2 mousePos, float width, float height) const;
};
