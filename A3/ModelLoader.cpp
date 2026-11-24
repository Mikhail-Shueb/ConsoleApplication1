#include "ModelLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#if __has_include(<assimp/Importer.hpp>)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::shared_ptr<Mesh> ModelLoader::loadAssimp(const std::string& filepath, const std::string& meshName) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "[Assimp Erro] " << importer.GetErrorString() << " ao carregar " << filepath << std::endl;
        return loadOBJNative(filepath, meshName);
    }

    if (scene->mNumMeshes == 0) {
        return nullptr;
    }

    aiMesh* ai_mesh = scene->mMeshes[0];
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
        Vertex v;
        v.Position = glm::vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);
        if (ai_mesh->HasNormals()) {
            v.Normal = glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
        } else {
            v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
        aiFace face = ai_mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    std::string name = meshName.empty() ? filepath : meshName;
    return std::make_shared<Mesh>(name, vertices, indices);
}
#endif

std::shared_ptr<Mesh> ModelLoader::loadOBJNative(const std::string& filepath, const std::string& meshName) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[ModelLoader Erro] Nao foi possivel abrir ficheiro OBJ: " << filepath << std::endl;
        return nullptr;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<Vertex> out_vertices;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        } else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        } else if (prefix == "f") {
            std::vector<std::string> faceTokens;
            std::string token;
            while (ss >> token) {
                faceTokens.push_back(token);
            }

            // Triangulacao em leque caso a face tenha mais de 3 vertices
            for (size_t i = 1; i + 1 < faceTokens.size(); i++) {
                std::string triTokens[3] = { faceTokens[0], faceTokens[i], faceTokens[i+1] };
                for (int k = 0; k < 3; k++) {
                    int posIdx = 0, normIdx = 0;
                    std::string str = triTokens[k];

                    size_t firstSlash = str.find('/');
                    if (firstSlash == std::string::npos) {
                        posIdx = std::stoi(str);
                    } else {
                        posIdx = std::stoi(str.substr(0, firstSlash));
                        size_t secondSlash = str.find('/', firstSlash + 1);
                        if (secondSlash != std::string::npos && secondSlash + 1 < str.size()) {
                            normIdx = std::stoi(str.substr(secondSlash + 1));
                        }
                    }

                    Vertex v;
                    if (posIdx > 0 && posIdx <= (int)temp_positions.size()) {
                        v.Position = temp_positions[posIdx - 1];
                    } else {
                        v.Position = glm::vec3(0.0f);
                    }

                    if (normIdx > 0 && normIdx <= (int)temp_normals.size()) {
                        v.Normal = temp_normals[normIdx - 1];
                    } else {
                        v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                    out_vertices.push_back(v);
                }
            }
        }
    }

    std::string name = meshName.empty() ? filepath : meshName;
    return std::make_shared<Mesh>(name, out_vertices);
}

std::shared_ptr<Mesh> ModelLoader::loadModel(const std::string& filepath, const std::string& meshName) {
#if __has_include(<assimp/Importer.hpp>)
    return loadAssimp(filepath, meshName);
#else
    return loadOBJNative(filepath, meshName);
#endif
}
