/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#pragma once

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for CoordinateSpace, Coordinat...
#include <inviwo/core/datastructures/volume/volume.h>          // for Volume
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport
#include <inviwo/core/ports/volumeport.h>                      // for VolumeSequenceInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/ordinalproperty.h>            // for DoubleProperty
#include <inviwo/core/util/glmutils.h>                         // for Vector
#include <inviwo/core/util/glmvec.h>                           // for dvec3
#include <inviwo/core/util/spatialsampler.h>                   // for SpatialSampler
#include <inviwo/core/util/volumesampler.h>                    // for VolumeDoubleSampler

#include <memory>  // for shared_ptr
#include <string>  // for operator+, string

#include <fmt/core.h>      // for format
#include <glm/common.hpp>  // for mix
#include <glm/fwd.hpp>     // for uvec3
#include <glm/mat4x4.hpp>  // for operator*
#include <glm/vec3.hpp>    // for operator/, operator*, oper...
#include <glm/vec4.hpp>    // for operator*, operator+

namespace inviwo {

class VolumeSequenceSingleTimestepSampler : public SpatialSampler<3, double> {
public:
    VolumeSequenceSingleTimestepSampler(double t, std::shared_ptr<const Volume> v0,
                                        std::shared_ptr<const Volume> v1,
                                        CoordinateSpace space = CoordinateSpace::Data)
        : SpatialSampler(*v0, space), t_(t), v0_(v0, space), v1_(v1, space) {}

protected:
    virtual dvec3 sampleDataSpace(const dvec3& pos) const override {
        auto a = v0_.sampleDataSpace(pos);
        auto b = v1_.sampleDataSpace(pos);
        return a + t_ * (b - a);
    };

    virtual bool withinBoundsDataSpace(const dvec3& pos) const override {
        return v0_.withinBoundsDataSpace(pos) || v1_.withinBoundsDataSpace(pos);
    };

private:
    double t_;
    VolumeDoubleSampler<3> v0_;
    VolumeDoubleSampler<3> v1_;
};

/** \docpage{org.inviwo.VolumeSequenceSingleTimestepSampler, Volume Sequence Single Timestep
 * Sampler}
 * ![](org.inviwo.VolumeSequenceSingleTimestepSampler.png?classIdentifier=org.inviwo.VolumeSequenceSingleTimestepSampler)
 *
 * Creates a spatial sampler for a given timestamp from a VolumeSequence. Will use linear
 * interpolation to sample between two volume in the sequence.
 * Useful for streamline visualization of a specific timestep
 *
 *
 * ### Inports
 *   * __volumeSequence__ The input sequence of volumes
 *
 * ### Outports
 *   * __sampler__ The created sampler
 *
 * ### Properties
 *   * __Timestamp__ the timestamp to sample at
 *
 */

class IVW_MODULE_BASE_API VolumeSequenceSingleTimestepSamplerProcessor : public Processor {
public:
    VolumeSequenceSingleTimestepSamplerProcessor();
    virtual ~VolumeSequenceSingleTimestepSamplerProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeSequenceInport volumeSequence_;
    DataOutport<SpatialSampler<3, double>> sampler_;

    DoubleProperty timestamp_;
};

}  // namespace inviwo
