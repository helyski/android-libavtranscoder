//
// Created by mapgoo1426 on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DECODER_H
#define ANDROID_LIBTRANSCODE_DECODER_H

extern "C"
{
#include "hw_decode.h"

namespace LibTranscode {
    class Decoder {
    public:
        Decoder();

        ~Decoder();

        int StartDecode();

    private:

    };

}

};

#endif //ANDROID_LIBTRANSCODE_DECODER_H
