//
// Created by Pkuism on 2025/9/20.
//
#include <spine/SkeletonLoader.h>
#include <spine/SkeletonData.h>

#include <spine/skeleton-loader-impl/Loader42.h>
#include <spine/skeleton-loader-impl/Loader40.h>
#include <spine/skeleton-loader-impl/Loader38.h>

namespace spine {

    void ISkeletonBinaryLoader::setError(const char *value1, const char *value2) {
        char message[256];
        int length;
        strcpy(message, value1);
        length = (int) strlen(value1);
        if (value2) strncat(message + length, value2, 255 - length);
        _error = String(message);
    }

    void ISkeletonJsonLoader::setError(Json *root, const String &value1, const String &value2) {
        _error = String(value1).append(value2);
        delete root;
    }

    char *ISkeletonBinaryLoader::readString(DataInput *input) {
        int length = readVarint(input, true);
        char *string;
        if (length == 0) return NULL;
        string = SpineExtension::alloc<char>(length, __FILE__, __LINE__);
        memcpy(string, input->cursor, length - 1);
        input->cursor += length - 1;
        string[length - 1] = '\0';
        return string;
    }

    char *ISkeletonBinaryLoader::readStringRef(DataInput *input, SkeletonData *skeletonData) {
        int index = readVarint(input, true);
        return index == 0 ? NULL : skeletonData->_strings[index - 1];
    }

    float ISkeletonBinaryLoader::readFloat(DataInput *input) {
        union {
            int intValue;
            float floatValue;
        } intToFloat;
        intToFloat.intValue = readInt(input);
        return intToFloat.floatValue;
    }

    unsigned char ISkeletonBinaryLoader::readByte(DataInput *input) {
        return *input->cursor++;
    }

    signed char ISkeletonBinaryLoader::readSByte(DataInput *input) {
        return (signed char) readByte(input);
    }

    bool ISkeletonBinaryLoader::readBoolean(DataInput *input) {
        return readByte(input) != 0;
    }

    int ISkeletonBinaryLoader::readInt(DataInput *input) {
        int result = readByte(input);
        result <<= 8;
        result |= readByte(input);
        result <<= 8;
        result |= readByte(input);
        result <<= 8;
        result |= readByte(input);
        return result;
    }

    void ISkeletonBinaryLoader::readColor(DataInput *input, Color &color) {
        color.r = readByte(input) / 255.0f;
        color.g = readByte(input) / 255.0f;
        color.b = readByte(input) / 255.0f;
        color.a = readByte(input) / 255.0f;
    }

    int ISkeletonBinaryLoader::readVarint(DataInput *input, bool optimizePositive) {
        unsigned char b = readByte(input);
        int value = b & 0x7F;
        if (b & 0x80) {
            b = readByte(input);
            value |= (b & 0x7F) << 7;
            if (b & 0x80) {
                b = readByte(input);
                value |= (b & 0x7F) << 14;
                if (b & 0x80) {
                    b = readByte(input);
                    value |= (b & 0x7F) << 21;
                    if (b & 0x80) value |= (readByte(input) & 0x7F) << 28;
                }
            }
        }
        if (!optimizePositive) value = (((unsigned int) value >> 1) ^ -(value & 1));
        return value;
    }

    void ISkeletonBinaryLoader::readFloatArray(DataInput *input, int n, float scale, Vector<float> &array) {
        array.setSize(n, 0);

        int i;
        if (scale == 1) {
            for (i = 0; i < n; ++i) {
                array[i] = readFloat(input);
            }
        } else {
            for (i = 0; i < n; ++i) {
                array[i] = readFloat(input) * scale;
            }
        }
    }

    void ISkeletonBinaryLoader::readShortArray(DataInput *input, Vector<unsigned short> &array) {
        int n = readVarint(input, true);
        array.setSize(n, 0);

        int i;
        for (i = 0; i < n; ++i) {
            array[i] = readByte(input) << 8;
            array[i] |= readByte(input);
        }
    }

    float ISkeletonJsonLoader::toColor(const char *value, size_t index) {
        char digits[3];
        char *error;
        int color;

        if (index >= strlen(value) / 2) return -1;

        value += index * 2;

        digits[0] = *value;
        digits[1] = *(value + 1);
        digits[2] = '\0';
        color = (int) strtoul(digits, &error, 16);
        if (*error != 0) return -1;

        return color / (float) 255;
    }

    void ISkeletonJsonLoader::toColor(Color &color, const char *value, bool hasAlpha) {
        color.r = toColor(value, 0);
        color.g = toColor(value, 1);
        color.b = toColor(value, 2);
        if (hasAlpha) color.a = toColor(value, 3);
    }

#define _STR(s)     #s
#define STR(s)      _STR(s)
#define _CONS(a,b)  a##.##b
#define CONS(a,b)   _CONS(a,b)
#define REG_SPINE_VERSION(x, y, c) do{if(version.startsWith(STR(CONS(x,y)))) return new c(loader, meshes, scaleRef, errStrRef);}while(0)

    ISkeletonBinaryLoader* SkeletonLoaderFactory::CreateBinaryLoader(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, const String& version, float& scaleRef, String& errStrRef) {
        REG_SPINE_VERSION(4, 2, BinaryLoader42);
        REG_SPINE_VERSION(4, 1, BinaryLoader42);
        REG_SPINE_VERSION(4, 0, BinaryLoader40);
        REG_SPINE_VERSION(3, 8, BinaryLoader38);
        return NULL;
    }

    ISkeletonJsonLoader* SkeletonLoaderFactory::CreateJsonLoader(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, const String& version, float& scaleRef, String& errStrRef) {
        REG_SPINE_VERSION(4, 2, JsonLoader42);
        REG_SPINE_VERSION(4, 1, JsonLoader42);
        REG_SPINE_VERSION(4, 0, JsonLoader40);
        REG_SPINE_VERSION(3, 8, JsonLoader38);
        return NULL;
    }

#undef _STR
#undef STR
#undef _CONS
#undef CONS
#undef REG_SPINE_VERSION
}