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

#ifndef Spine_SkeletonBinary_h
#define Spine_SkeletonBinary_h

#include <spine/Inherit.h>
#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>
#include <spine/DataInput.h>

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

	class SP_API SkeletonBinary : public SpineObject {
	public:
		explicit SkeletonBinary(Atlas *atlasArray);

		explicit SkeletonBinary(AttachmentLoader *attachmentLoader, bool ownsLoader = false);

		~SkeletonBinary();

		SkeletonData *readSkeletonData(const unsigned char *binary, int length);

		SkeletonData *readSkeletonDataFile(const String &path);

		void setScale(float scale) { _scale = scale; }

		String &getError() { return _error; }

	private:
		AttachmentLoader *_attachmentLoader;
        Vector<LinkedMesh *> _linkedMeshes;
		String _error;
		float _scale;
		const bool _ownsLoader;

		void setError(const char *value1, const char *value2);

        char *readString(DataInput *input);
        int readInt(DataInput *input);
        int readVarint(DataInput *input, bool optimizePositive);
        unsigned char readByte(DataInput *input);
	};
}

#endif /* Spine_SkeletonBinary_h */
