//
// Created by Pkuism on 2025/9/20.
//
#include <spine/SkeletonLoader.h>
#include <spine/Inherit.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>

#ifndef PINE_FLUTTER_LIBRARY_LOADER40_H
#define PINE_FLUTTER_LIBRARY_LOADER40_H
namespace spine {
    class BinaryLoader40 : public ISkeletonBinaryLoader {
    public:
        BinaryLoader40(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, float& scaleRef, String& errStrRef) : ISkeletonBinaryLoader(loader, meshes, scaleRef, errStrRef){}
        virtual SkeletonData *readSkeleton(DataInput *input, SkeletonData *skeletonData) override;
    private:
        Skin *readSkin(DataInput *input, bool defaultSkin, SkeletonData *skeletonData, bool nonessential);

        Attachment *readAttachment(DataInput *input, Skin *skin, int slotIndex, const String &attachmentName,
                                   SkeletonData *skeletonData, bool nonessential);

        void readVertices(DataInput *input, VertexAttachment *attachment, int vertexCount);

        Animation *readAnimation(const String &name, DataInput *input, SkeletonData *skeletonData);

        void
        setBezier(DataInput *input, CurveTimeline *timeline, int bezier, int frame, int value, float time1, float time2,
                  float value1, float value2, float scale);

        Timeline *readTimeline(DataInput *input, CurveTimeline1 *timeline, float scale);

        Timeline *readTimeline2(DataInput *input, CurveTimeline2 *timeline, float scale);
    };

    class JsonLoader40 : public ISkeletonJsonLoader {
    public:
        JsonLoader40(AttachmentLoader* loader, Vector<LinkedMesh *>& meshes, float& scaleRef, String& errStrRef) : ISkeletonJsonLoader(loader, meshes, scaleRef, errStrRef){}
        virtual SkeletonData *readSkeleton(Json *json, SkeletonData *skeletonData) override;
    private:
        static void
        setBezier(CurveTimeline *timeline, int frame, int value, int bezier, float time1, float value1, float cx1,
                  float cy1,
                  float cx2, float cy2, float time2, float value2);

        static int
        readCurve(Json *curve, CurveTimeline *timeline, int bezier, int frame, int value, float time1, float time2,
                  float value1, float value2, float scale);

        static Timeline *readTimeline(Json *keyMap, CurveTimeline1 *timeline, float defaultValue, float scale);

        static Timeline *
        readTimeline(Json *keyMap, CurveTimeline2 *timeline, const char *name1, const char *name2, float defaultValue,
                     float scale);

        Animation *readAnimation(Json *root, SkeletonData *skeletonData);

        void readVertices(Json *attachmentMap, VertexAttachment *attachment, size_t verticesLength);

        int findSlotIndex(SkeletonData *skeletonData, const String &slotName, Vector<Timeline *> timelines);
    };
}
#endif //PINE_FLUTTER_LIBRARY_LOADER40_H
