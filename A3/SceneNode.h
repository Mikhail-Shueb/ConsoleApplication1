#pragma once

#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

class SceneNode : public std::enable_shared_from_this<SceneNode> {
public:
    std::string name;
    std::shared_ptr<Mesh> mesh;
    Shader* shader;
    glm::vec4 color;

    glm::mat4 localTransform;
    glm::mat4 worldTransform;

    SceneNode* parent;
    std::vector<std::shared_ptr<SceneNode>> children;

    SceneNode(const std::string& name = "Node",
              std::shared_ptr<Mesh> mesh = nullptr,
              Shader* shader = nullptr,
              const glm::vec4& color = glm::vec4(1.0f));

    void addChild(std::shared_ptr<SceneNode> child);

    // Atualiza a matriz de mundo de forma hierarquica (World = ParentWorld * Local)
    void update(const glm::mat4& parentWorldMatrix = glm::mat4(1.0f));

    // Desenha o no e propaga o desenho aos nos filhos
    void draw(const glm::mat4& view, const glm::mat4& projection) const;
};
