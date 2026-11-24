#pragma once

#include "SceneNode.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct KeyframeTransform {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

class AnimationManager {
public:
    float progress; // t in [0.0, 1.0] (0.0 = Caixa, 1.0 = Lobo)
    float speed;
    std::vector<std::shared_ptr<SceneNode>> pieceNodes;

    AnimationManager();

    void registerPieces(const std::vector<std::shared_ptr<SceneNode>>& pieces);

    // Atualiza a interpolacao se as teclas das setas estiverem pressionadas
    void update(float deltaTime, bool keyLeftPressed, bool keyRightPressed);

    void applyInterpolation();

private:
    std::vector<KeyframeTransform> boxConfig;
    std::vector<KeyframeTransform> wolfConfig;

    void setupConfigurations();
};
