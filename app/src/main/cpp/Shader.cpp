//
// Created by vitor on 08/09/2022.
//

#include "Shader.h"
#include "LogUtils.h"

#include <vector>
#include <string>

#include <GLES3/gl31.h>
#include <glm/gtc/type_ptr.hpp>

void checkShaderCompile(GLuint shader){
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        LOG("error %s", errorLog.data());
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(shader); // Don't leak the shader.
    }
}

Shader::Shader(const char *vertexSource, const char *fragSource) : Id(0){
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, nullptr);
    glCompileShader(fragShader);
    checkShaderCompile(fragShader);

    this->Id = glCreateProgram();
    glAttachShader(this->Id, vertexShader);
    glAttachShader(this->Id, fragShader);
    glLinkProgram(this->Id);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
}

void Shader::Bind() const {
    glUseProgram(this->Id);
}

Shader::~Shader() {
    //glDeleteProgram(this->Id);
}

void Shader::SetUniform(const char *name, glm::mat4 value) const {
    auto location = glGetUniformLocation(this->Id, name);
    if(location < 0) return;

    glUniformMatrix4fv(location, 1, false, glm::value_ptr(value));
}

void Shader::SetUniform(const char *name, float value) const {
    auto location = glGetUniformLocation(this->Id, name);
    if(location < 0) return;

    glUniform1f(location, value);
}

void Shader::SetUniform(const char *name, int32_t value) const {
    auto location = glGetUniformLocation(this->Id, name);
    if(location < 0) return;

    glUniform1i(location, value);
}

Shader::Shader():Id(0){
}

Shader &Shader::operator=(Shader &&other)  noexcept {
    this->Id = other.Id;
    other.Id = 0;
    return *this;
}

Shader::Shader(Shader &&other) noexcept {
    this->Id = other.Id;
    other.Id = 0;
}
