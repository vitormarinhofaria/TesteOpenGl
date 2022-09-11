//
// Created by vitor on 08/09/2022.
//

#include "Framebuffer.h"
#include <GLES3/gl31.h>
#include "glm/glm/glm.hpp"
#include <cstdint>

#include "LogUtils.h"

Framebuffer::Framebuffer(int32_t width, int32_t height) : ColorBuffer(0), RBO(0), FBO(0),
                                                          ScreenSize{width, height}{
    glGenFramebuffers(1, &this->FBO);

    glGenTextures(1, &this->ColorBuffer);
    glBindTexture(GL_TEXTURE_2D, this->ColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->ScreenSize.x, this->ScreenSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &this->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->ScreenSize.x, this->ScreenSize.y);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG("Failed to initialize Framebuffer");
}

void Framebuffer::GetPixels(uint8_t *pixelsDest) const {
    this->Bind();
    glReadPixels(0, 0, this->ScreenSize.x, this->ScreenSize.y, GL_RGB, GL_UNSIGNED_BYTE, pixelsDest);
}

size_t Framebuffer::GetPixelsBufferSize() const {
    return sizeof(uint8_t) * (this->ScreenSize.x * this->ScreenSize.y) * 3;
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
}

Framebuffer::Framebuffer():FBO(0), RBO(0), ColorBuffer(0), ScreenSize{0,0} {
}

GLuint Framebuffer::GetColorAttachment() const {
    return this->ColorBuffer;
}
