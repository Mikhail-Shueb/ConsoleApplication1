#include "Animation.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

AnimationManager::AnimationManager()
    : progress(1.0f), speed(0.8f) { // Comeca no Lobo (1.0)
    setupConfigurations();
}

void AnimationManager::registerPieces(const std::vector<std::shared_ptr<SceneNode>>& pieces) {
    pieceNodes = pieces;
    applyInterpolation();
}

void AnimationManager::setupConfigurations() {
    boxConfig.resize(7);
    wolfConfig.resize(7);

    // =========================================================================
    // CONFIGURACAO 2: LOBO (#100 WOLF) - Em pe no plano X-Y
    // =========================================================================
    // Baseado na geometria exata do Assignment 2, adaptada para o espaco 3D
    // As patas assentam no tampo da mesa (Y = 0.0), orientadas ao longo de X-Y
    const float wolfScale = 0.85f;
    const float cx = 95.9545f;
    const float cy = 156.2440f; // Linha de chao do lobo no PDF (patas em Y=0)
    const float u = 22.8064f;
    const float invU = 1.0f / u;

    auto toWolfPos = [&](float px, float py) -> glm::vec3 {
        return glm::vec3((px - cx) * invU * wolfScale, (py - cy) * invU * wolfScale, 0.0f);
    };

    auto makeWolfRot = [](float angleDeg) -> glm::quat {
        // Rotacao no plano X-Y (em torno de Z)
        return glm::angleAxis(glm::radians(angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
    };

    // Peca 0: Triangulo Grande 1 (Peito)
    wolfConfig[0] = { toWolfPos(58.6339f, 195.602f), makeWolfRot(-14.6388f), glm::vec3(wolfScale) };

    // Peca 1: Triangulo Grande 2 (Corpo)
    wolfConfig[1] = { toWolfPos(130.598f, 200.377f), makeWolfRot(165.361f), glm::vec3(wolfScale) };

    // Peca 2: Paralelogramo 3a (Cauda)
    wolfConfig[2] = { toWolfPos(114.994f, 204.453f), makeWolfRot(-14.6388f), glm::vec3(wolfScale) };

    // Peca 3: Triangulo Pequeno 1 (Garganta)
    wolfConfig[3] = { toWolfPos(64.3977f, 217.6685f), makeWolfRot(165.361f), glm::vec3(wolfScale) };

    // Peca 4: Quadrado (Cabeca)
    wolfConfig[4] = { toWolfPos(64.3977f, 217.6685f), makeWolfRot(75.3605f), glm::vec3(wolfScale) };

    // Peca 5: Triangulo Pequeno 2 (Focinho)
    wolfConfig[5] = { toWolfPos(48.0956f, 245.4989f), makeWolfRot(165.361f), glm::vec3(wolfScale) };

    // Peca 6: Triangulo Medio (Pata Dianteira)
    wolfConfig[6] = { toWolfPos(89.8409f, 187.451f), makeWolfRot(165.362f), glm::vec3(wolfScale) };


    // =========================================================================
    // CONFIGURACAO 1: CAIXA (BOX) - Quadrado fechado deitado no plano X-Z
    // =========================================================================
    // No Tangram classico, as 7 pecas formam um quadrado perfeito de lado 2*sqrt(2).
    // As pecas estao deitadas horizontalmente na mesa (plano X-Z com Y = 0.2, espessura ao longo de Y).
    // Para deitar a peca (que foi modelada em X-Y com espessura em Z), aplicamos uma rotacao de -90 graus em X.
    glm::quat layFlat = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    auto makeBoxRot = [&](float angleZ_deg) -> glm::quat {
        glm::quat rotZ = glm::angleAxis(glm::radians(angleZ_deg), glm::vec3(0.0f, 0.0f, 1.0f));
        return layFlat * rotZ;
    };

    const float boxScale = 0.85f;
    const float yLevel = 0.2f * boxScale; // Altura para assentar na mesa

    // Peca 0: Triangulo Grande 1 (Ocupa o canto inferior esquerdo do quadrado)
    boxConfig[0] = { glm::vec3(-1.0f * boxScale, yLevel, 1.0f * boxScale), makeBoxRot(0.0f), glm::vec3(boxScale) };

    // Peca 1: Triangulo Grande 2 (Ocupa o canto superior esquerdo do quadrado)
    boxConfig[1] = { glm::vec3(1.0f * boxScale, yLevel, -1.0f * boxScale), makeBoxRot(180.0f), glm::vec3(boxScale) };

    // Peca 2: Paralelogramo 3a (Ocupa faixa lateral)
    boxConfig[2] = { glm::vec3(0.0f * boxScale, yLevel, 0.0f * boxScale), makeBoxRot(45.0f), glm::vec3(boxScale) };

    // Peca 3: Triangulo Pequeno 1 (Canto)
    boxConfig[3] = { glm::vec3(-0.5f * boxScale, yLevel, -0.5f * boxScale), makeBoxRot(90.0f), glm::vec3(boxScale) };

    // Peca 4: Quadrado (Centro)
    boxConfig[4] = { glm::vec3(0.0f * boxScale, yLevel, -0.5f * boxScale), makeBoxRot(45.0f), glm::vec3(boxScale) };

    // Peca 5: Triangulo Pequeno 2 (Borda)
    boxConfig[5] = { glm::vec3(0.5f * boxScale, yLevel, 0.5f * boxScale), makeBoxRot(270.0f), glm::vec3(boxScale) };

    // Peca 6: Triangulo Medio (Canto inferior direito)
    boxConfig[6] = { glm::vec3(1.0f * boxScale, yLevel, 1.0f * boxScale), makeBoxRot(135.0f), glm::vec3(boxScale) };
}

void AnimationManager::update(float deltaTime, bool keyLeftPressed, bool keyRightPressed) {
    if (keyLeftPressed) {
        // Anima em direcao a Caixa (progress = 0.0)
        progress -= speed * deltaTime;
        if (progress < 0.0f) progress = 0.0f;
        applyInterpolation();
    } else if (keyRightPressed) {
        // Anima em direcao ao Lobo (progress = 1.0)
        progress += speed * deltaTime;
        if (progress > 1.0f) progress = 1.0f;
        applyInterpolation();
    }
    // Se nenhuma tecla estiver pressionada, progress mantem o valor atual (pausa no ponto exato)
}

void AnimationManager::applyInterpolation() {
    if (pieceNodes.size() < 7) return;

    for (size_t i = 0; i < 7; ++i) {
        // Interpolacao Linear (LERP) de Posicao
        glm::vec3 pos = glm::mix(boxConfig[i].position, wolfConfig[i].position, progress);

        // Interpolacao Esferica de Quaternioes (SLERP) de Rotacao
        glm::quat rot = glm::slerp(boxConfig[i].rotation, wolfConfig[i].rotation, progress);

        // Interpolacao Linear de Escala
        glm::vec3 scl = glm::mix(boxConfig[i].scale, wolfConfig[i].scale, progress);

        // Matriz de transformacao local do no da peca
        glm::mat4 m(1.0f);
        m = glm::translate(m, pos);
        m = m * glm::mat4_cast(rot);
        m = glm::scale(m, scl);

        pieceNodes[i]->localTransform = m;
    }
}
