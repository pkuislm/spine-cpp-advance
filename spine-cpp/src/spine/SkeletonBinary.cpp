/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated July 28, 2023. Replaces all prior versions.
 *
 * Copyright (c) 2013-2023, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software or
 * otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THE
 * SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <spine/SkeletonBinary.h>

#include <spine/Animation.h>
#include <spine/Atlas.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/Attachment.h>
#include <spine/CurveTimeline.h>
#include <spine/LinkedMesh.h>
#include <spine/SkeletonData.h>
#include <spine/Skin.h>
#include <spine/VertexAttachment.h>

#include <spine/AttachmentTimeline.h>
#include <spine/AttachmentType.h>
#include <spine/BoneData.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/ClippingAttachment.h>
#include <spine/ColorTimeline.h>
#include <spine/ContainerUtil.h>
#include <spine/DeformTimeline.h>
#include <spine/DrawOrderTimeline.h>
#include <spine/Event.h>
#include <spine/EventData.h>
#include <spine/EventTimeline.h>
#include <spine/IkConstraintData.h>
#include <spine/IkConstraintTimeline.h>
#include <spine/InheritTimeline.h>
#include <spine/MeshAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PathConstraintData.h>
#include <spine/PathConstraintMixTimeline.h>
#include <spine/PathConstraintPositionTimeline.h>
#include <spine/PathConstraintSpacingTimeline.h>
#include <spine/PhysicsConstraintData.h>
#include <spine/PhysicsConstraintTimeline.h>
#include <spine/PointAttachment.h>
#include <spine/RegionAttachment.h>
#include <spine/RotateTimeline.h>
#include <spine/ScaleTimeline.h>
#include <spine/ShearTimeline.h>
#include <spine/SlotData.h>
#include <spine/TransformConstraintData.h>
#include <spine/TransformConstraintTimeline.h>
#include <spine/TranslateTimeline.h>
#include <spine/SequenceTimeline.h>
#include <spine/Version.h>

#include <spine/DataInput.h>
#include <spine/SkeletonLoader.h>

using namespace spine;

SkeletonBinary::SkeletonBinary(Atlas *atlasArray) : _attachmentLoader(
															new (__FILE__, __LINE__) AtlasAttachmentLoader(atlasArray)),
													_error(), _scale(1), _ownsLoader(true) {
}

SkeletonBinary::SkeletonBinary(AttachmentLoader *attachmentLoader, bool ownsLoader) : _attachmentLoader(
																							  attachmentLoader),
																					  _error(),
																					  _scale(1),
																					  _ownsLoader(ownsLoader) {
	assert(_attachmentLoader != NULL);
}

SkeletonBinary::~SkeletonBinary() {
	ContainerUtil::cleanUpVectorOfPointers(_linkedMeshes);
	_linkedMeshes.clear();

	if (_ownsLoader) delete _attachmentLoader;
}

SkeletonData *SkeletonBinary::readSkeletonData(const unsigned char *binary, const int length) {
    SkeletonData *skeletonData;

    auto *input = new (__FILE__, __LINE__) DataInput();
    input->cursor = binary;
    input->end = binary + length;

    _linkedMeshes.clear();

    skeletonData = new (__FILE__, __LINE__) SkeletonData();

    if(*binary <= 0x40) {//might be version 3.8 or below
        char *hashString = readString(input);
        char *version = readString(input);
        if(version[0] == '3' && version[1] == '.' && (version[2] >= '1' && version[2] <= '9')){
            skeletonData->_hash.own(hashString);
            skeletonData->_version.own(version);

            auto loader = SkeletonLoaderFactory::CreateBinaryLoader(_attachmentLoader, _linkedMeshes, skeletonData->_version, _scale, _error);
            if(loader == NULL) {
                char errorMsg[255];
                snprintf(errorMsg, 255, "Skeleton version %s does not match any supported version.", skeletonData->_version.buffer());
                delete skeletonData;
                setError(errorMsg, "");
                return NULL;
            }
            skeletonData = loader->readSkeleton(input, skeletonData);
            delete loader;
            return skeletonData;
        }
        SpineExtension::free(hashString, __FILE__, __LINE__);
        SpineExtension::free(version, __FILE__, __LINE__);
    }

    char buffer[16] = {0};
    int lowHash = readInt(input);
    int hightHash = readInt(input);
    String hashString;
    snprintf(buffer, 16, "%x", hightHash);
    hashString.append(buffer);
    snprintf(buffer, 16, "%x", lowHash);
    hashString.append(buffer);
    skeletonData->_hash = hashString;

    char *skeletonDataVersion = readString(input);
    skeletonData->_version.own(skeletonDataVersion);

    //if (!skeletonData->_version.startsWith(SPINE_VERSION_STRING)) {
    //	char errorMsg[255];
    //	snprintf(errorMsg, 255, "Skeleton version %s does not match runtime version %s", skeletonData->_version.buffer(), SPINE_VERSION_STRING);
    //	setError(errorMsg, "");
    //	delete input;
    //	delete skeletonData;
    //	return NULL;
    //}

    auto loader = SkeletonLoaderFactory::CreateBinaryLoader(_attachmentLoader, _linkedMeshes, skeletonData->_version, _scale, _error);
    if(loader == NULL) {
        char errorMsg[255];
        snprintf(errorMsg, 255, "Skeleton version %s does not match any supported version.", skeletonData->_version.buffer());
        delete skeletonData;
        setError(errorMsg, "");
        return NULL;
    }
    skeletonData = loader->readSkeleton(input, skeletonData);
    delete loader;
    return skeletonData;
}

SkeletonData *SkeletonBinary::readSkeletonDataFile(const String &path) {
    int length;
    SkeletonData *skeletonData;
    const char *binary = SpineExtension::readFile(path.buffer(), &length);
    if (length == 0 || !binary) {
        setError("Unable to read skeleton file: ", path.buffer());
        return NULL;
    }
    skeletonData = readSkeletonData((unsigned char *) binary, length);
    SpineExtension::free(binary, __FILE__, __LINE__);
    return skeletonData;
}

void SkeletonBinary::setError(const char *value1, const char *value2) {
	char message[256];
	int length;
	strcpy(message, value1);
	length = (int) strlen(value1);
	if (value2) strncat(message + length, value2, 255 - length);
	_error = String(message);
}

unsigned char SkeletonBinary::readByte(DataInput *input) {
    return *input->cursor++;
}

char *SkeletonBinary::readString(DataInput *input) {
    int length = readVarint(input, true);
    char *string;
    if (length == 0) return NULL;
    string = SpineExtension::alloc<char>(length, __FILE__, __LINE__);
    memcpy(string, input->cursor, length - 1);
    input->cursor += length - 1;
    string[length - 1] = '\0';
    return string;
}


int SkeletonBinary::readInt(DataInput *input) {
    int result = readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    return result;
}

int SkeletonBinary::readVarint(DataInput *input, bool optimizePositive) {
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