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

#include <spine/SkeletonJson.h>

#include <spine/Atlas.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/CurveTimeline.h>
#include <spine/Json.h>
#include <spine/LinkedMesh.h>
#include <spine/SkeletonData.h>
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
#include <spine/Skin.h>
#include <spine/SlotData.h>
#include <spine/TransformConstraintData.h>
#include <spine/TransformConstraintTimeline.h>
#include <spine/TranslateTimeline.h>
#include <spine/Vertices.h>
#include <spine/SequenceTimeline.h>
#include <spine/Version.h>

#include <spine/SkeletonLoader.h>

using namespace spine;

SkeletonJson::SkeletonJson(Atlas *atlas) : _attachmentLoader(new (__FILE__, __LINE__) AtlasAttachmentLoader(atlas)),
										   _scale(1), _ownsLoader(true) {}

SkeletonJson::SkeletonJson(AttachmentLoader *attachmentLoader, bool ownsLoader) : _attachmentLoader(attachmentLoader),
																				  _scale(1),
																				  _ownsLoader(ownsLoader) {
	assert(_attachmentLoader != NULL);
}

SkeletonJson::~SkeletonJson() {
	ContainerUtil::cleanUpVectorOfPointers(_linkedMeshes);

	if (_ownsLoader) delete _attachmentLoader;
}

SkeletonData *SkeletonJson::readSkeletonDataFile(const String &path) {
	int length;
	SkeletonData *skeletonData;
	const char *json = SpineExtension::readFile(path, &length);
	if (length == 0 || !json) {
		setError(NULL, "Unable to read skeleton file: ", path);
		return NULL;
	}

	skeletonData = readSkeletonData(json);

	SpineExtension::free(json, __FILE__, __LINE__);

	return skeletonData;
}

SkeletonData *SkeletonJson::readSkeletonData(const char *json) {
    Json * root, *skeleton;
    SkeletonData* skeletonData;
    _error = "";
    _linkedMeshes.clear();

    root = new (__FILE__, __LINE__) Json(json);

    if (!root) {
        setError(NULL, "Invalid skeleton JSON: ", Json::getError());
        return NULL;
    }

    skeletonData = new (__FILE__, __LINE__) SkeletonData();

    skeleton = Json::getItem(root, "skeleton");
    if (skeleton) {
        skeletonData->_hash = Json::getString(skeleton, "hash", 0);
        skeletonData->_version = Json::getString(skeleton, "spine", 0);
        //if (!skeletonData->_version.startsWith(SPINE_VERSION_STRING)) {
        //	char errorMsg[255];
        //	snprintf(errorMsg, 255, "Skeleton version %s does not match runtime version %s", skeletonData->_version.buffer(), SPINE_VERSION_STRING);
        //	delete skeletonData;
        //	setError(NULL, errorMsg, "");
        //	return NULL;
        //}
        skeletonData->_x = Json::getFloat(skeleton, "x", 0);
        skeletonData->_y = Json::getFloat(skeleton, "y", 0);
        skeletonData->_width = Json::getFloat(skeleton, "width", 0);
        skeletonData->_height = Json::getFloat(skeleton, "height", 0);
        skeletonData->_referenceScale = Json::getFloat(skeleton, "referenceScale", 100) * _scale;
        skeletonData->_fps = Json::getFloat(skeleton, "fps", 30);
        skeletonData->_audioPath = Json::getString(skeleton, "audio", 0);
        skeletonData->_imagesPath = Json::getString(skeleton, "images", 0);

        auto loader = SkeletonLoaderFactory::CreateJsonLoader(_attachmentLoader, _linkedMeshes, skeletonData->_version, _scale, _error);
        if(loader == NULL) {
            char errorMsg[255];
            snprintf(errorMsg, 255, "Skeleton version %s does not match any supported version.", skeletonData->_version.buffer());
            delete skeletonData;
            setError(NULL, errorMsg, "");
            return NULL;
        }
        skeletonData = loader->readSkeleton(root, skeletonData);
        delete loader;
        return skeletonData;
    }else {
        delete skeletonData;
        setError(root, "Invalid skeleton json data: missing header.", "");
        return NULL;
    }
}

void SkeletonJson::setError(Json *root, const String &value1, const String &value2) {
	_error = String(value1).append(value2);
	delete root;
}
