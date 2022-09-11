//
// Created by vitor on 08/09/2022.
//

#include "Texture2D.h"

Texture2D::Texture2D(RefImg imgData): Id(0) {
    glGenTextures(1, &this->Id);
    glBindTexture(GL_TEXTURE_2D, this->Id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, imgData.GL_Format, imgData.Dimensions.x, imgData.Dimensions.y,0,imgData.GL_Format, GL_UNSIGNED_BYTE, imgData.Data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, this->Id);
}

Texture2D::~Texture2D() {
    //glDeleteTextures(1, &this->Id);
}

Texture2D::Texture2D():Id(0) {
}

Texture2D::Texture2D(Texture2D &&old) noexcept {
    this->Id = old.Id;
    old.Id = 0;
}

Texture2D& Texture2D::operator=(Texture2D &&old) noexcept {
    this->Id = old.Id;
    old.Id = 0;
    return *this;
}
