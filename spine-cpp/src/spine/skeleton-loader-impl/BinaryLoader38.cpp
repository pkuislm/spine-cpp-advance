//
// Created by Pkuism on 2025/9/20.
//
#include <spine/skeleton-loader-impl/Loader38.h>

#include <spine/SkeletonData.h>
#include <spine/Atlas.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/LinkedMesh.h>
#include <spine/Skin.h>
#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Animation.h>
#include <spine/CurveTimeline.h>

#include <spine/ContainerUtil.h>
#include <spine/BoneData.h>
#include <spine/SlotData.h>
#include <spine/IkConstraintData.h>
#include <spine/TransformConstraintData.h>
#include <spine/PathConstraintData.h>
#include <spine/AttachmentType.h>
#include <spine/RegionAttachment.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PointAttachment.h>
#include <spine/ClippingAttachment.h>
#include <spine/EventData.h>
#include <spine/AttachmentTimeline.h>
#include <spine/ColorTimeline.h>
#include <spine/RotateTimeline.h>
#include <spine/TranslateTimeline.h>
#include <spine/ScaleTimeline.h>
#include <spine/ShearTimeline.h>
#include <spine/IkConstraintTimeline.h>
#include <spine/TransformConstraintTimeline.h>
#include <spine/PathConstraintPositionTimeline.h>
#include <spine/PathConstraintSpacingTimeline.h>
#include <spine/PathConstraintMixTimeline.h>
#include <spine/DeformTimeline.h>
#include <spine/DrawOrderTimeline.h>
#include <spine/EventTimeline.h>
#include <spine/Event.h>

using namespace spine;

SkeletonData *BinaryLoader38::readSkeleton(DataInput *input, SkeletonData *skeletonData) {
    bool nonessential;

    skeletonData->_x = readFloat(input);
    skeletonData->_y = readFloat(input);
    skeletonData->_width = readFloat(input);
    skeletonData->_height = readFloat(input);

    nonessential = readBoolean(input);

    if (nonessential) {
        /* Skip images path, audio path & fps */
        skeletonData->_fps = readFloat(input);
        skeletonData->_imagesPath.own(readString(input));
        skeletonData->_audioPath.own(readString(input));
    }

    int numStrings = readVarint(input, true);
    for (int i = 0; i < numStrings; i++)
        skeletonData->_strings.add(readString(input));

    /* Bones. */
    int numBones = readVarint(input, true);
    skeletonData->_bones.setSize(numBones, 0);
    for (int i = 0; i < numBones; ++i) {
        const char *name = readString(input);
        BoneData *parent = i == 0 ? 0 : skeletonData->_bones[readVarint(input, true)];
        BoneData *data = new(__FILE__, __LINE__) BoneData(i, String(name, true), parent);
        data->_rotation = readFloat(input);
        data->_x = readFloat(input) * _scale;
        data->_y = readFloat(input) * _scale;
        data->_scaleX = readFloat(input);
        data->_scaleY = readFloat(input);
        data->_shearX = readFloat(input);
        data->_shearY = readFloat(input);
        data->_length = readFloat(input) * _scale;
        data->_inherit = static_cast<Inherit>(readVarint(input, true));
        data->_skinRequired = readBoolean(input);
        if (nonessential) readInt(input); /* Skip bone color. */
        skeletonData->_bones[i] = data;
    }

    /* Slots. */
    int slotsCount = readVarint(input, true);
    skeletonData->_slots.setSize(slotsCount, 0);
    for (int i = 0; i < slotsCount; ++i) {
        const char *slotName = readString(input);
        BoneData *boneData = skeletonData->_bones[readVarint(input, true)];
        SlotData *slotData = new(__FILE__, __LINE__) SlotData(i, String(slotName, true), *boneData);

        readColor(input, slotData->getColor());
        unsigned char r = readByte(input);
        unsigned char g = readByte(input);
        unsigned char b = readByte(input);
        unsigned char a = readByte(input);
        if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
            slotData->getDarkColor().set(r / 255.0f, g / 255.0f, b / 255.0f, 1);
            slotData->setHasDarkColor(true);
        }
        slotData->_attachmentName = readStringRef(input, skeletonData);
        slotData->_blendMode = static_cast<BlendMode>(readVarint(input, true));
        skeletonData->_slots[i] = slotData;
    }

    /* IK constraints. */
    int ikConstraintsCount = readVarint(input, true);
    skeletonData->_ikConstraints.setSize(ikConstraintsCount, 0);
    for (int i = 0; i < ikConstraintsCount; ++i) {
        const char *name = readString(input);
        IkConstraintData *data = new(__FILE__, __LINE__) IkConstraintData(String(name, true));
        data->setOrder(readVarint(input, true));
        data->setSkinRequired(readBoolean(input));
        int bonesCount = readVarint(input, true);
        data->_bones.setSize(bonesCount, 0);
        for (int ii = 0; ii < bonesCount; ++ii)
            data->_bones[ii] = skeletonData->_bones[readVarint(input, true)];
        data->_target = skeletonData->_bones[readVarint(input, true)];
        data->_mix = readFloat(input);
        data->_softness = readFloat(input) * _scale;
        data->_bendDirection = readSByte(input);
        data->_compress = readBoolean(input);
        data->_stretch = readBoolean(input);
        data->_uniform = readBoolean(input);
        skeletonData->_ikConstraints[i] = data;
    }

    /* Transform constraints. */
    int transformConstraintsCount = readVarint(input, true);
    skeletonData->_transformConstraints.setSize(transformConstraintsCount, 0);
    for (int i = 0; i < transformConstraintsCount; ++i) {
        const char *name = readString(input);
        TransformConstraintData *data = new(__FILE__, __LINE__) TransformConstraintData(String(name, true));
        data->setOrder(readVarint(input, true));
        data->setSkinRequired(readBoolean(input));
        int bonesCount = readVarint(input, true);
        data->_bones.setSize(bonesCount, 0);
        for (int ii = 0; ii < bonesCount; ++ii)
            data->_bones[ii] = skeletonData->_bones[readVarint(input, true)];
        data->_target = skeletonData->_bones[readVarint(input, true)];
        data->_local = readBoolean(input);
        data->_relative = readBoolean(input);
        data->_offsetRotation = readFloat(input);
        data->_offsetX = readFloat(input) * _scale;
        data->_offsetY = readFloat(input) * _scale;
        data->_offsetScaleX = readFloat(input);
        data->_offsetScaleY = readFloat(input);
        data->_offsetShearY = readFloat(input);
        data->_mixRotate = readFloat(input);
        data->_mixX = data->_mixY = readFloat(input);
        data->_mixScaleX = data->_mixScaleY = readFloat(input);
        data->_mixShearY = readFloat(input);
        skeletonData->_transformConstraints[i] = data;
    }

    /* Path constraints */
    int pathConstraintsCount = readVarint(input, true);
    skeletonData->_pathConstraints.setSize(pathConstraintsCount, 0);
    for (int i = 0; i < pathConstraintsCount; ++i) {
        const char *name = readString(input);
        PathConstraintData *data = new(__FILE__, __LINE__) PathConstraintData(String(name, true));
        data->setOrder(readVarint(input, true));
        data->setSkinRequired(readBoolean(input));
        int bonesCount = readVarint(input, true);
        data->_bones.setSize(bonesCount, 0);
        for (int ii = 0; ii < bonesCount; ++ii)
            data->_bones[ii] = skeletonData->_bones[readVarint(input, true)];
        data->_target = skeletonData->_slots[readVarint(input, true)];
        data->_positionMode = static_cast<PositionMode>(readVarint(input, true));
        data->_spacingMode = static_cast<SpacingMode>(readVarint(input, true));
        data->_rotateMode = static_cast<RotateMode>(readVarint(input, true));
        data->_offsetRotation = readFloat(input);
        data->_position = readFloat(input);
        if (data->_positionMode == PositionMode_Fixed) data->_position *= _scale;
        data->_spacing = readFloat(input);
        if (data->_spacingMode == SpacingMode_Length || data->_spacingMode == SpacingMode_Fixed)
            data->_spacing *= _scale;
        data->_mixRotate = readFloat(input);
        data->_mixX = data->_mixY = readFloat(input);
        skeletonData->_pathConstraints[i] = data;
    }

    /* Default skin. */
    Skin* defaultSkin = readSkin(input, true, skeletonData, nonessential);
    if (defaultSkin) {
        skeletonData->_defaultSkin = defaultSkin;
        skeletonData->_skins.add(defaultSkin);
    }

    /* Skins. */
    for (size_t i = 0, n = (size_t)readVarint(input, true); i < n; ++i) {
        Skin* skin = readSkin(input, false, skeletonData, nonessential);
        if (skin)
            skeletonData->_skins.add(skin);
        else {
            delete input;
            delete skeletonData;
            return NULL;
        }
    }

    /* Linked meshes. */
    for (int i = 0, n = _linkedMeshes.size(); i < n; ++i) {
        LinkedMesh *linkedMesh = _linkedMeshes[i];
        Skin *skin = linkedMesh->_skin.length() == 0 ? skeletonData->getDefaultSkin() : skeletonData->findSkin(
                linkedMesh->_skin);
        if (skin == NULL) {
            delete input;
            delete skeletonData;
            setError("Skin not found: ", linkedMesh->_skin.buffer());
            return NULL;
        }
        Attachment *parent = skin->getAttachment(linkedMesh->_slotIndex, linkedMesh->_parent);
        if (parent == NULL) {
            delete input;
            delete skeletonData;
            setError("Parent mesh not found: ", linkedMesh->_parent.buffer());
            return NULL;
        }
        linkedMesh->_mesh->_timelineAttachment = linkedMesh->_inheritTimeline ? static_cast<VertexAttachment *>(parent)
                                                                              : linkedMesh->_mesh;
        linkedMesh->_mesh->setParentMesh(static_cast<MeshAttachment *>(parent));
        if (linkedMesh->_mesh->_region) linkedMesh->_mesh->updateRegion();
        _attachmentLoader->configureAttachment(linkedMesh->_mesh);
    }
    ContainerUtil::cleanUpVectorOfPointers(_linkedMeshes);
    _linkedMeshes.clear();

    /* Events. */
    int eventsCount = readVarint(input, true);
    skeletonData->_events.setSize(eventsCount, 0);
    for (int i = 0; i < eventsCount; ++i) {
        const char *name = readStringRef(input, skeletonData);
        EventData *eventData = new(__FILE__, __LINE__) EventData(String(name));
        eventData->_intValue = readVarint(input, false);
        eventData->_floatValue = readFloat(input);
        eventData->_stringValue.own(readString(input));
        eventData->_audioPath.own(readString(input)); // skip audio path
        if (!eventData->_audioPath.isEmpty()) {
            eventData->_volume = readFloat(input);
            eventData->_balance = readFloat(input);
        }
        skeletonData->_events[i] = eventData;
    }

    /* Animations. */
    int animationsCount = readVarint(input, true);
    skeletonData->_animations.setSize(animationsCount, 0);
    for (int i = 0; i < animationsCount; ++i) {
        String name(readString(input), true);
        Animation *animation = readAnimation(name, input, skeletonData);
        if (!animation) {
            delete input;
            delete skeletonData;
            return NULL;
        }
        skeletonData->_animations[i] = animation;
    }

    delete input;
    return skeletonData;
}

Skin *BinaryLoader38::readSkin(DataInput *input, bool defaultSkin, SkeletonData *skeletonData, bool nonessential) {
    Skin *skin;
    int slotCount = 0;
    if (defaultSkin) {
        slotCount = readVarint(input, true);
        if (slotCount == 0) return NULL;
        skin = new(__FILE__, __LINE__) Skin("default");
    } else {
        skin = new(__FILE__, __LINE__) Skin(readStringRef(input, skeletonData));
        for (int i = 0, n = readVarint(input, true); i < n; i++)
            skin->getBones().add(skeletonData->_bones[readVarint(input, true)]);

        for (int i = 0, n = readVarint(input, true); i < n; i++)
            skin->getConstraints().add(skeletonData->_ikConstraints[readVarint(input, true)]);

        for (int i = 0, n = readVarint(input, true); i < n; i++)
            skin->getConstraints().add(skeletonData->_transformConstraints[readVarint(input, true)]);

        for (int i = 0, n = readVarint(input, true); i < n; i++)
            skin->getConstraints().add(skeletonData->_pathConstraints[readVarint(input, true)]);
        slotCount = readVarint(input, true);
    }

    for (int i = 0; i < slotCount; ++i) {
        int slotIndex = readVarint(input, true);
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
            String name(readStringRef(input, skeletonData));
            Attachment *attachment = readAttachment(input, skin, slotIndex, name, skeletonData, nonessential);
            if (attachment)
                skin->setAttachment(slotIndex, String(name), attachment);
            else {
                delete skin;
                return nullptr;
            }
        }
    }
    return skin;
}

Attachment *BinaryLoader38::readAttachment(DataInput *input, Skin *skin, int slotIndex, const String &attachmentName,
                                           SkeletonData *skeletonData, bool nonessential
) {
    String name(readStringRef(input, skeletonData));
    if (name.isEmpty()) name = attachmentName;

    AttachmentType type = static_cast<AttachmentType>(readByte(input));
    switch (type) {
        case AttachmentType_Region: {
            String path(readStringRef(input, skeletonData));
            if (path.isEmpty()) path = name;
            RegionAttachment *region = _attachmentLoader->newRegionAttachment(*skin, String(name), String(path), NULL);
            if (!region) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            region->_path = path;
            region->_rotation = readFloat(input);
            region->_x = readFloat(input) * _scale;
            region->_y = readFloat(input) * _scale;
            region->_scaleX = readFloat(input);
            region->_scaleY = readFloat(input);
            region->_width = readFloat(input) * _scale;
            region->_height = readFloat(input) * _scale;
            readColor(input, region->getColor());
            region->_sequence = NULL;
            region->updateRegion();
            _attachmentLoader->configureAttachment(region);
            return region;
        }
        case AttachmentType_Boundingbox: {
            int vertexCount = readVarint(input, true);
            BoundingBoxAttachment *box = _attachmentLoader->newBoundingBoxAttachment(*skin, String(name));
            if (!box) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            readVertices(input, static_cast<VertexAttachment *>(box), vertexCount);
            if (nonessential) {
                /* Skip color. */
                readInt(input);
            }
            _attachmentLoader->configureAttachment(box);
            return box;
        }
        case AttachmentType_Mesh: {
            int vertexCount;
            MeshAttachment *mesh;
            String path(readStringRef(input, skeletonData));
            if (path.isEmpty()) path = name;

            mesh = _attachmentLoader->newMeshAttachment(*skin, String(name), String(path), NULL);
            if (!mesh) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            mesh->_path = path;
            readColor(input, mesh->getColor());
            vertexCount = readVarint(input, true);
            readFloatArray(input, vertexCount << 1, 1, mesh->getRegionUVs());
            readShortArray(input, mesh->getTriangles());
            readVertices(input, static_cast<VertexAttachment *>(mesh), vertexCount);
            mesh->_sequence = NULL;
            mesh->updateRegion();
            mesh->_hullLength = readVarint(input, true) << 1;
            if (nonessential) {
                readShortArray(input, mesh->getEdges());
                mesh->_width = readFloat(input) * _scale;
                mesh->_height = readFloat(input) * _scale;
            } else {
                mesh->_width = 0;
                mesh->_height = 0;
            }
            _attachmentLoader->configureAttachment(mesh);
            return mesh;
        }
        case AttachmentType_Linkedmesh: {
            String path(readStringRef(input, skeletonData));
            if (path.isEmpty()) path = name;

            MeshAttachment *mesh = _attachmentLoader->newMeshAttachment(*skin, String(name), String(path), NULL);
            if (!mesh) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            mesh->_path = path;
            readColor(input, mesh->getColor());
            String skinName(readStringRef(input, skeletonData));
            String parent(readStringRef(input, skeletonData));
            bool inheritDeform = readBoolean(input);
            if (nonessential) {
                mesh->_width = readFloat(input) * _scale;
                mesh->_height = readFloat(input) * _scale;
            }
            mesh->_sequence = NULL;
            LinkedMesh *linkedMesh = new(__FILE__, __LINE__) LinkedMesh(mesh, String(skinName), slotIndex,
                                                                        String(parent), inheritDeform);
            _linkedMeshes.add(linkedMesh);
            return mesh;
        }
        case AttachmentType_Path: {
            PathAttachment* path = _attachmentLoader->newPathAttachment(*skin, String(name));
            if (!path) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            path->_closed = readBoolean(input);
            path->_constantSpeed = readBoolean(input);
            int vertexCount = readVarint(input, true);
            readVertices(input, static_cast<VertexAttachment *>(path), vertexCount);
            int lengthsLength = vertexCount / 3;
            path->_lengths.setSize(lengthsLength, 0);
            for (int i = 0; i < lengthsLength; ++i) {
                path->_lengths[i] = readFloat(input) * _scale;
            }
            if (nonessential) {
                /* Skip color. */
                readInt(input);
            }
            _attachmentLoader->configureAttachment(path);
            return path;
        }
        case AttachmentType_Point: {
            PointAttachment* point = _attachmentLoader->newPointAttachment(*skin, String(name));
            if (!point) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            point->_rotation = readFloat(input);
            point->_x = readFloat(input) * _scale;
            point->_y = readFloat(input) * _scale;

            if (nonessential) {
                /* Skip color. */
                readInt(input);
            }
            _attachmentLoader->configureAttachment(point);
            return point;
        }
        case AttachmentType_Clipping: {
            int endSlotIndex = readVarint(input, true);
            int vertexCount = readVarint(input, true);
            ClippingAttachment* clip = _attachmentLoader->newClippingAttachment(*skin, name);
            if (!clip) {
                setError("Error reading attachment: ", name.buffer());
                return nullptr;
            }
            readVertices(input, static_cast<VertexAttachment *>(clip), vertexCount);
            clip->_endSlot = skeletonData->_slots[endSlotIndex];
            if (nonessential) {
                /* Skip color. */
                readInt(input);
            }
            _attachmentLoader->configureAttachment(clip);
            return clip;
        }
    }
    return nullptr;
}

void BinaryLoader38::readVertices(DataInput *input, VertexAttachment *attachment, int vertexCount) {
    float scale = _scale;
    int verticesLength = vertexCount << 1;
    attachment->setWorldVerticesLength(vertexCount << 1);

    if (!readBoolean(input)) {
        readFloatArray(input, verticesLength, scale, attachment->getVertices());
        return;
    }

    Vector<float> &vertices = attachment->getVertices();
    Vector<int> &bones = attachment->getBones();
    vertices.ensureCapacity(verticesLength * 3 * 3);
    bones.ensureCapacity(verticesLength * 3);

    for (int i = 0; i < vertexCount; ++i) {
        int boneCount = readVarint(input, true);
        bones.add(boneCount);
        for (int ii = 0; ii < boneCount; ++ii) {
            bones.add(readVarint(input, true));
            vertices.add(readFloat(input) * scale);
            vertices.add(readFloat(input) * scale);
            vertices.add(readFloat(input));
        }
    }
}

Animation *BinaryLoader38::readAnimation(const String &name, DataInput *input, SkeletonData *skeletonData) {
    Vector<Timeline *> timelines;
    float scale = _scale;

    // Slot timelines.
    for (int i = 0, n = readVarint(input, true); i < n; ++i) {
        int slotIndex = readVarint(input, true);
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
            unsigned char timelineType = readByte(input);
            int frameCount = readVarint(input, true);
            int frameLast = frameCount - 1;
            switch (timelineType) {
                case SLOT_ATTACHMENT: {
                    AttachmentTimeline *timeline = new(__FILE__, __LINE__) AttachmentTimeline(frameCount, slotIndex);
                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        String attachmentName(readStringRef(input, skeletonData));
                        timeline->setFrame(frameIndex, time, attachmentName);
                    }
                    timelines.add(timeline);
                    break;
                }
                case SLOT_RGBA: {//SLOT_COLOR
                    RGBATimeline *timeline = new(__FILE__, __LINE__) RGBATimeline(frameCount, slotIndex);
                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        int color = readInt(input);
                        float r = ((color & 0xff000000) >> 24) / 255.0f;
                        float g = ((color & 0x00ff0000) >> 16) / 255.0f;
                        float b = ((color & 0x0000ff00) >> 8) / 255.0f;
                        float a = ((color & 0x000000ff)) / 255.0f;
                        timeline->setFrame(frameIndex, time, r, g, b, a);
                        if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
                    }
                    timelines.add(timeline);
                    break;
                }
                case SLOT_RGB: {//SLOT_TWO_COLOR
                    RGBA2Timeline *timeline = new(__FILE__, __LINE__) RGBA2Timeline(frameCount, slotIndex);
                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        int color = readInt(input);
                        float r = ((color & 0xff000000) >> 24) / 255.0f;
                        float g = ((color & 0x00ff0000) >> 16) / 255.0f;
                        float b = ((color & 0x0000ff00) >> 8) / 255.0f;
                        float a = ((color & 0x000000ff)) / 255.0f;
                        int color2 = readInt(input); // 0x00rrggbb
                        float r2 = ((color2 & 0x00ff0000) >> 16) / 255.0f;
                        float g2 = ((color2 & 0x0000ff00) >> 8) / 255.0f;
                        float b2 = ((color2 & 0x000000ff)) / 255.0f;

                        timeline->setFrame(frameIndex, time, r, g, b, a, r2, g2, b2);
                        if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
                    }
                    timelines.add(timeline);
                    break;
                }
                default: {
                    ContainerUtil::cleanUpVectorOfPointers(timelines);
                    setError("Invalid timeline type for a slot: ", skeletonData->_slots[slotIndex]->_name.buffer());
                    return NULL;
                }
            }
        }
    }

    // Bone timelines.
    for (int i = 0, n = readVarint(input, true); i < n; ++i) {
        int boneIndex = readVarint(input, true);
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
            unsigned char timelineType = readByte(input);
            int frameCount = readVarint(input, true);
            switch (timelineType) {
                case BONE_ROTATE: {
                    RotateTimeline *timeline = new(__FILE__, __LINE__) RotateTimeline(frameCount, boneIndex);
                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        float degrees = readFloat(input);
                        timeline->setFrame(frameIndex, time, degrees);
                        if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
                    }
                    timelines.add(timeline);
                    break;
                }
                case BONE_TRANSLATE:
                case 2/*BONE_SCALE*/:
                case 3/*BONE_SHEAR*/: {
                    CurveTimeline2 *timeline;
                    float timelineScale = 1;
                    if (timelineType == 2/*BONE_SCALE*/) {
                        timeline = new(__FILE__, __LINE__) ScaleTimeline(frameCount, boneIndex);
                    } else if (timelineType == 3/*BONE_SHEAR*/) {
                        timeline = new(__FILE__, __LINE__) ShearTimeline(frameCount, boneIndex);
                    } else {
                        timeline = new(__FILE__, __LINE__) TranslateTimeline(frameCount, boneIndex);
                        timelineScale = scale;
                    }
                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        float x = readFloat(input) * timelineScale;
                        float y = readFloat(input) * timelineScale;
                        timeline->setFrame(frameIndex, time, x, y);
                        if (frameIndex < frameCount - 1) {
                            readCurve(input, frameIndex, timeline);
                        }
                    }
                    timelines.add(timeline);
                    break;
                }
                default: {
                    ContainerUtil::cleanUpVectorOfPointers(timelines);
                    setError("Invalid timeline type for a bone: ", skeletonData->_bones[boneIndex]->_name.buffer());
                    return NULL;
                }
            }
        }
    }

    // IK timelines.
    for (int i = 0, n = readVarint(input, true); i < n; ++i) {
        int index = readVarint(input, true);
        int frameCount = readVarint(input, true);
        IkConstraintTimeline *timeline = new(__FILE__, __LINE__) IkConstraintTimeline(frameCount,index);
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            float time = readFloat(input);
            float mix = readFloat(input);
            float softness = readFloat(input) * _scale;
            signed char bendDirection = readSByte(input);
            bool compress = readBoolean(input);
            bool stretch = readBoolean(input);
            timeline->setFrame(frameIndex, time, mix, softness, bendDirection, compress, stretch);
            if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
        }
        timelines.add(timeline);
    }

    // Transform constraint timelines.
    for (int i = 0, n = readVarint(input, true); i < n; ++i) {
        int index = readVarint(input, true);
        int frameCount = readVarint(input, true);
        TransformConstraintTimeline *timeline = new(__FILE__, __LINE__) TransformConstraintTimeline(frameCount, index);
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            float time = readFloat(input);
            float rotateMix = readFloat(input);
            float translateMix = readFloat(input);
            float scaleMix = readFloat(input);
            float shearMix = readFloat(input);
            timeline->setFrame(frameIndex, time, rotateMix, translateMix, translateMix, scaleMix, scaleMix, shearMix);
            if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
        }
        timelines.add(timeline);
    }

    // Path constraint timelines.
    for (int i = 0, n = readVarint(input, true); i < n; ++i) {
        int index = readVarint(input, true);
        PathConstraintData *data = skeletonData->_pathConstraints[index];
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
            int timelineType = readSByte(input);
            int frameCount = readVarint(input, true);
            switch (timelineType) {
                case PATH_POSITION:
                case PATH_SPACING: {
                    CurveTimeline1 *timeline;
                    float timelineScale = 1;
                    if (timelineType == PATH_SPACING) {
                        timeline = new(__FILE__, __LINE__) PathConstraintSpacingTimeline(frameCount, index);

                        if (data->_spacingMode == SpacingMode_Length || data->_spacingMode == SpacingMode_Fixed) timelineScale = scale;
                    } else {
                        timeline = new(__FILE__, __LINE__) PathConstraintPositionTimeline(frameCount,  index);

                        if (data->_positionMode == PositionMode_Fixed) timelineScale = scale;
                    }
                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        float value = readFloat(input) * timelineScale;
                        timeline->setFrame(frameIndex, time, value);
                        if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
                    }
                    timelines.add(timeline);
                    break;
                }
                case PATH_MIX: {
                    PathConstraintMixTimeline *timeline = new(__FILE__, __LINE__) PathConstraintMixTimeline(frameCount, index);

                    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                        float time = readFloat(input);
                        float rotateMix = readFloat(input);
                        float translateMix = readFloat(input);
                        timeline->setFrame(frameIndex, time, rotateMix, translateMix, translateMix);
                        if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
                    }
                    timelines.add(timeline);
                    break;
                }
            }
        }
    }

    // Deform timelines.
    for (int i = 0, n = readVarint(input, true); i < n; ++i) {
        Skin *skin = skeletonData->_skins[readVarint(input, true)];
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
            int slotIndex = readVarint(input, true);
            for (int iii = 0, nnn = readVarint(input, true); iii < nnn; iii++) {
                const char *attachmentName = readStringRef(input, skeletonData);
                Attachment *baseAttachment = skin->getAttachment(slotIndex, String(attachmentName));

                if (!baseAttachment) {
                    ContainerUtil::cleanUpVectorOfPointers(timelines);
                    setError("Attachment not found: ", attachmentName);
                    return NULL;
                }

                VertexAttachment *attachment = static_cast<VertexAttachment *>(baseAttachment);

                bool weighted = attachment->_bones.size() > 0;
                Vector<float> &vertices = attachment->_vertices;
                size_t deformLength = weighted ? vertices.size() / 3 * 2 : vertices.size();

                size_t frameCount = (size_t)readVarint(input, true);

                DeformTimeline *timeline = new(__FILE__, __LINE__) DeformTimeline(frameCount, frameCount+1, slotIndex, attachment);

                for (size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                    float time = readFloat(input);
                    Vector<float> deform;
                    size_t end = (size_t)readVarint(input, true);
                    if (end == 0) {
                        if (weighted) {
                            deform.setSize(deformLength, 0);
                            for (size_t iiii = 0; iiii < deformLength; ++iiii)
                                deform[iiii] = 0;
                        } else {
                            deform.clearAndAddAll(vertices);
                        }
                    } else {
                        deform.setSize(deformLength, 0);
                        size_t start = (size_t)readVarint(input, true);
                        end += start;
                        if (scale == 1) {
                            for (size_t v = start; v < end; ++v)
                                deform[v] = readFloat(input);
                        } else {
                            for (size_t v = start; v < end; ++v)
                                deform[v] = readFloat(input) * scale;
                        }

                        if (!weighted) {
                            for (size_t v = 0, vn = deform.size(); v < vn; ++v)
                                deform[v] += vertices[v];
                        }
                    }

                    timeline->setFrame(frameIndex, time, deform);
                    if (frameIndex < frameCount - 1) readCurve(input, frameIndex, timeline);
                }

                timelines.add(timeline);
            }
        }
    }

    // Draw order timeline.
    size_t drawOrderCount = (size_t)readVarint(input, true);
    if (drawOrderCount > 0) {
        DrawOrderTimeline *timeline = new(__FILE__, __LINE__) DrawOrderTimeline(drawOrderCount);

        size_t slotCount = skeletonData->_slots.size();
        for (size_t i = 0; i < drawOrderCount; ++i) {
            float time = readFloat(input);
            size_t offsetCount = (size_t)readVarint(input, true);

            Vector<int> drawOrder;
            drawOrder.setSize(slotCount, 0);
            for (int ii = (int)slotCount - 1; ii >= 0; --ii)
                drawOrder[ii] = -1;

            Vector<int> unchanged;
            unchanged.setSize(slotCount - offsetCount, 0);
            size_t originalIndex = 0, unchangedIndex = 0;
            for (size_t ii = 0; ii < offsetCount; ++ii) {
                size_t slotIndex = (size_t)readVarint(input, true);
                // Collect unchanged items.
                while (originalIndex != slotIndex)
                    unchanged[unchangedIndex++] = originalIndex++;
                // Set changed items.
                size_t index = originalIndex;
                drawOrder[index + (size_t)readVarint(input, true)] = originalIndex++;
            }

            // Collect remaining unchanged items.
            while (originalIndex < slotCount) {
                unchanged[unchangedIndex++] = originalIndex++;
            }

            // Fill in unchanged items.
            for (int ii = (int)slotCount - 1; ii >= 0; --ii)
                if (drawOrder[ii] == -1) drawOrder[ii] = unchanged[--unchangedIndex];
            timeline->setFrame(i, time, drawOrder);
        }
        timelines.add(timeline);
    }

    // Event timeline.
    int eventCount = readVarint(input, true);
    if (eventCount > 0) {
        EventTimeline *timeline = new(__FILE__, __LINE__) EventTimeline(eventCount);

        for (int i = 0; i < eventCount; ++i) {
            float time = readFloat(input);
            EventData *eventData = skeletonData->_events[readVarint(input, true)];
            Event *event = new(__FILE__, __LINE__) Event(time, *eventData);

            event->_intValue = readVarint(input, false);
            event->_floatValue = readFloat(input);
            bool freeString = readBoolean(input);
            const char *event_stringValue = freeString ? readString(input) : eventData->_stringValue.buffer();
            event->_stringValue = String(event_stringValue);
            if (freeString) SpineExtension::free(event_stringValue, __FILE__, __LINE__);

            if (!eventData->_audioPath.isEmpty()) {
                event->_volume = readFloat(input);
                event->_balance = readFloat(input);
            }
            timeline->setFrame(i, event);
        }

        timelines.add(timeline);
    }

    float duration = 0;
    for (int i = 0, n = timelines.size(); i < n; i++) {
        duration = MathUtil::max(duration, (timelines[i])->getDuration());
    }
    return new(__FILE__, __LINE__) Animation(String(name), timelines, duration);
}

void BinaryLoader38::readCurve(DataInput *input, int frameIndex, CurveTimeline *timeline) {
    switch (readByte(input)) {
        case CURVE_STEPPED: {
            timeline->setStepped(frameIndex);
            break;
        }
        case CURVE_BEZIER: {
            float cx1 = readFloat(input);
            float cy1 = readFloat(input);
            float cx2 = readFloat(input);
            float cy2 = readFloat(input);
            timeline->setCurve(frameIndex, cx1, cy1, cx2, cy2);
            break;
        }
    }
}
