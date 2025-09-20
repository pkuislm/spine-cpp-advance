//
// Created by Pkuism on 2025/9/20.
//

#ifndef PINE_FLUTTER_LIBRARY_DATAINPUT_H
#define PINE_FLUTTER_LIBRARY_DATAINPUT_H
#include <spine/SpineObject.h>

namespace spine {
    struct DataInput : public SpineObject {
        const unsigned char *cursor;
        const unsigned char *end;
    };
}

#endif //PINE_FLUTTER_LIBRARY_DATAINPUT_H
