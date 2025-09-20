//
// Created by Pkuism on 2025/9/20.
//

#ifndef Spine_SkeletonLoader_h
#define Spine_SkeletonLoader_h

#include <spine/SpineString.h>
#include <spine/Json.h>
#include <spine/Vector.h>
#include <spine/DataInput.h>
#include <spine/Color.h>

namespace spine {
    class SkeletonData;

    class Atlas;

    class AttachmentLoader;

    class LinkedMesh;

    class Skin;

    class Attachment;

    class VertexAttachment;

    class Animation;

    class Timeline;

    class CurveTimeline;

    class CurveTimeline1;

    class CurveTimeline2;

    class Sequence;
    class ISkeletonBinaryLoader {
    public:
        static const int BONE_ROTATE = 0;
        static const int BONE_TRANSLATE = 1;
        static const int BONE_TRANSLATEX = 2;
        static const int BONE_TRANSLATEY = 3;
        static const int BONE_SCALE = 4;
        static const int BONE_SCALEX = 5;
        static const int BONE_SCALEY = 6;
        static const int BONE_SHEAR = 7;
        static const int BONE_SHEARX = 8;
        static const int BONE_SHEARY = 9;
        static const int BONE_INHERIT = 10;

        static const int SLOT_ATTACHMENT = 0;
        static const int SLOT_RGBA = 1;
        static const int SLOT_RGB = 2;
        static const int SLOT_RGBA2 = 3;
        static const int SLOT_RGB2 = 4;
        static const int SLOT_ALPHA = 5;

        static const int ATTACHMENT_DEFORM = 0;
        static const int ATTACHMENT_SEQUENCE = 1;

        static const int PATH_POSITION = 0;
        static const int PATH_SPACING = 1;
        static const int PATH_MIX = 2;

        static const int PHYSICS_INERTIA = 0;
        static const int PHYSICS_STRENGTH = 1;
        static const int PHYSICS_DAMPING = 2;
        static const int PHYSICS_MASS = 4;
        static const int PHYSICS_WIND = 5;
        static const int PHYSICS_GRAVITY = 6;
        static const int PHYSICS_MIX = 7;
        static const int PHYSICS_RESET = 8;

        static const int CURVE_LINEAR = 0;
        static const int CURVE_STEPPED = 1;
        static const int CURVE_BEZIER = 2;

        ISkeletonBinaryLoader(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, float& scaleRef, String& errStrRef)
                : _attachmentLoader(loader), _linkedMeshes(meshes), _error(errStrRef), _scale(scaleRef){}
        virtual ~ISkeletonBinaryLoader() = default;
        virtual SkeletonData *readSkeleton(DataInput *input, SkeletonData *skeletonData) = 0;
        String &getError() { return _error; }
    protected:
        AttachmentLoader *_attachmentLoader;
        Vector<LinkedMesh *>& _linkedMeshes;
        String& _error;
        float& _scale;
        void setError(const char *value1, const char *value2);

        char *readString(DataInput *input);
        char *readStringRef(DataInput *input, SkeletonData *skeletonData);
        float readFloat(DataInput *input);
        unsigned char readByte(DataInput *input);
        signed char readSByte(DataInput *input);
        bool readBoolean(DataInput *input);
        int readInt(DataInput *input);
        void readColor(DataInput *input, Color &color);
        int readVarint(DataInput *input, bool optimizePositive);
        void readFloatArray(DataInput *input, int n, float scale, Vector<float>& array);
        void readShortArray(DataInput *input, Vector<unsigned short>& array);
    };

    class ISkeletonJsonLoader {
    public:
        ISkeletonJsonLoader(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, float& scaleRef, String& errStrRef)
                : _attachmentLoader(loader), _linkedMeshes(meshes), _error(errStrRef), _scale(scaleRef){}
        virtual ~ISkeletonJsonLoader() = default;
        virtual SkeletonData *readSkeleton(Json *json, SkeletonData *skeletonData) = 0;
        String &getError() { return _error; }
    protected:
        AttachmentLoader *_attachmentLoader;
        Vector<LinkedMesh *>& _linkedMeshes;
        String& _error;
        float& _scale;
        void setError(Json *root, const String &value1, const String &value2);

        static float toColor(const char *value, size_t index);
        static void toColor(Color &color, const char *value, bool hasAlpha);
    };

    class SkeletonLoaderFactory {
    public:
        static ISkeletonBinaryLoader* CreateBinaryLoader(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, const String& version, float& scaleRef, String& errStrRef);
        static ISkeletonJsonLoader* CreateJsonLoader(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, const String& version, float& scaleRef, String& errStrRef);
    };
}

#endif //Spine_SkeletonLoader_h
