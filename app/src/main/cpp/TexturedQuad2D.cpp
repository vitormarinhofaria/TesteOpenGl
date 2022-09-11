//
// Created by vitor on 08/09/2022.
//

#include <cstring>
#include "TexturedQuad2D.h"

TexturedQuad2D::TexturedQuad2D(bool fullscreen) : VAO(0), VBO(0), EBO(0) {
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};
    float screenQuad[20];
    if (fullscreen)
    {
        float quad[] = {
                // positions         // texture coords
                1.0f, 1.0f, 1.0f, 1.0f, 1.0f,   // top right
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f,  // bottom right
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom left
                -1.0f, 1.0f, 1.0f, 0.0f, 1.0f   // top left
        };
        std::memcpy(screenQuad, quad, sizeof(quad));
    }
    else
    {
        float quad[] = {
                // positions         // texture coords
                0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
                0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
                -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
                -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left
        };
        std::memcpy(screenQuad, quad, sizeof(quad));
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), screenQuad, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 *sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TexturedQuad2D::Draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

TexturedQuad2D::~TexturedQuad2D(){
//    glDeleteBuffers(1, &EBO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteVertexArrays(1, &VAO);
}
