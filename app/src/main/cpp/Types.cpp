//
// Created by vitor on 08/09/2022.
//
#include <utility>
#include "Types.h"

Img::Img(RefImg src) : m_Data(src){
}

Img::~Img() {
    delete[] m_Data.Data;
    m_Data.Data = nullptr;
}
