//
// Created by mapgoo1426 on 2022/10/9.
//




extern "C"
{
#include "Decoder.h"


namespace LibTranscode {


    Decoder::Decoder() {

    }

    Decoder::~Decoder() {

    }

    int Decoder::StartDecode() {
        start_hwdecode();
        return 1;
    }
}

};