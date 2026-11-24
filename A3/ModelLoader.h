#pragma once

#include "Mesh.h"
#include <memory>
#include <string>

class ModelLoader {
public:
    // Carrega um ficheiro .obj (usando Assimp se disponivel, ou leitor OBJ nativo)
    static std::shared_ptr<Mesh> loadModel(const std::string& filepath, const std::string& meshName = "");

private:
    static std::shared_ptr<Mesh> loadOBJNative(const std::string& filepath, const std::string& meshName);
#if __has_include(<assimp/Importer.hpp>)
    static std::shared_ptr<Mesh> loadAssimp(const std::string& filepath, const std::string& meshName);
#endif
};
