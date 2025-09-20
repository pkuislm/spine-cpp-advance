//
// Created by Pkuism on 2025/9/20.
//
#include <spine/SkeletonLoader.h>
#include <spine/Inherit.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>

#ifndef PINE_FLUTTER_LIBRARY_LOADER38_H
#define PINE_FLUTTER_LIBRARY_LOADER38_H

namespace spine {
    class BinaryLoader38 : public ISkeletonBinaryLoader {
    public:
        BinaryLoader38(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, float& scaleRef, String& errStrRef) : ISkeletonBinaryLoader(loader, meshes, scaleRef, errStrRef){}
        virtual SkeletonData *readSkeleton(DataInput *input, SkeletonData *skeletonData) override;
    private:
        Skin* readSkin(DataInput* input, bool defaultSkin, SkeletonData* skeletonData, bool nonessential);

        Attachment* readAttachment(DataInput* input, Skin* skin, int slotIndex, const String& attachmentName, SkeletonData* skeletonData, bool nonessential);

        void readVertices(DataInput* input, VertexAttachment* attachment, int vertexCount);

        Animation* readAnimation(const String& name, DataInput* input, SkeletonData *skeletonData);

        void setBezier(DataInput *input, CurveTimeline *timeline, int bezier, int frame, int value, float time1,
                       float time2,
                       float value1, float value2, float scale);

        void readCurve(DataInput* input, int frameIndex, CurveTimeline* timeline);
    };

    class JsonLoader38 : public ISkeletonJsonLoader {
    public:
        JsonLoader38(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, float& scaleRef, String& errStrRef) : ISkeletonJsonLoader(loader, meshes, scaleRef, errStrRef){}
        virtual SkeletonData *readSkeleton(Json *json, SkeletonData *skeletonData) override;
    private:
        static void readCurve(Json *frame, CurveTimeline *timeline, size_t frameIndex);

        Animation *readAnimation(Json *root, SkeletonData *skeletonData);

        void readVertices(Json *attachmentMap, VertexAttachment *attachment, size_t verticesLength);

        int findSlotIndex(SkeletonData *skeletonData, const String &slotName, Vector<Timeline *> timelines);
    };
}
#endif //PINE_FLUTTER_LIBRARY_LOADER38_H
