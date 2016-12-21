/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_ANIMATIONEDITORQT_H
#define IVW_ANIMATIONEDITORQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/datastructures/animationobserver.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <warn/pop>

namespace inviwo {

namespace animation {

constexpr int TrackHeight = 25;
constexpr int TimelineHeight = TrackHeight;
constexpr int KeyframeWidth = 15;
constexpr int KeyframeHeight = TrackHeight;
constexpr int WidthPerTimeUnit = 96; 
constexpr int WidthPerFrame = WidthPerTimeUnit/24;

class AnimationController;

/**
 * \class AnimationEditorQt
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */

class IVW_MODULE_ANIMATIONQT_API AnimationEditorQt : public QGraphicsScene, public AnimationObserver {
public:
    AnimationEditorQt(AnimationController& controller);
    virtual ~AnimationEditorQt() = default;

	virtual void onTrackAdded(Track* track) override;
	virtual void onTrackRemoved(Track* track) override;

protected:
    AnimationController& controller_;
};

}  // namespace
}  // namespace

#endif  // IVW_ANIMATIONEDITORQT_H
