#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

// Estrutura de vertice com posicao e vetor normal para iluminacao por face
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh {
public:
    std::string name;
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;
    bool hasIndices;

    Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {});
    ~Mesh();

    void draw() const;

private:
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};
