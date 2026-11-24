#include "SceneNode.h"

SceneNode::SceneNode(const std::string& name, std::shared_ptr<Mesh> mesh, Shader* shader, const glm::vec4& color)
    : name(name), mesh(mesh), shader(shader), color(color),
      localTransform(glm::mat4(1.0f)), worldTransform(glm::mat4(1.0f)), parent(nullptr) {
}

void SceneNode::addChild(std::shared_ptr<SceneNode> child) {
    if (child) {
        child->parent = this;
        children.push_back(child);
    }
}

void SceneNode::update(const glm::mat4& parentWorldMatrix) {
    // Hierarquia do grafo de cena: compoe a transformacao local com a do no pai
    worldTransform = parentWorldMatrix * localTransform;

    for (auto& child : children) {
        child->update(worldTransform);
    }
}

void SceneNode::draw(const glm::mat4& view, const glm::mat4& projection) const {
    if (mesh && shader) {
        shader->use();
        shader->setMat4("u_Model", worldTransform);
        shader->setMat4("u_View", view);
        shader->setMat4("u_Projection", projection);
        shader->setVec4("u_BaseColor", color);

        mesh->draw();
    }

    for (const auto& child : children) {
        child->draw(view, projection);
    }
}
