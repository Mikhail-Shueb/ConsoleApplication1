#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec4(const std::string& name, const glm::vec4& val) const;
    void setVec3(const std::string& name, const glm::vec3& val) const;
    void setFloat(const std::string& name, float val) const;
    void setInt(const std::string& name, int val) const;

private:
    void checkCompileErrors(GLuint shader, const std::string& type);
};
