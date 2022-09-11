//
// Created by vitor on 08/09/2022.
//

#ifndef TESTEOPENGL_SHADER_H
#define TESTEOPENGL_SHADER_H

#include <GLES3/gl31.h>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const char* vertexSource, const char* fragSource);
    Shader();
    Shader(const Shader &old_obj) = delete;
    Shader(Shader &&old_obj) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    void Bind() const;
    void SetUniform(const char* name, glm::mat4) const;
    void SetUniform(const char* name, float value) const;
    void SetUniform(const char* name, int32_t value) const;
    ~Shader();
private:
    GLuint Id;
};


#endif //TESTEOPENGL_SHADER_H
