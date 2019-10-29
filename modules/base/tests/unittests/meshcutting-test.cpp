/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>

#include <modules/base/algorithm/mesh/meshclipping.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/base/algorithm/meshutils.h>

namespace inviwo {


TEST(MeshCutting, SutherlandHodgman) {

    const glm::u32vec3 triangle{0, 1, 2};
    const Plane plane{vec3{0, 0, 0}, vec3{0, 1, 0}};
    std::vector<vec3> positions{vec3{-1, -1, 0}, vec3{1, -1, 0}, vec3{0, 1, 0}};
    std::vector<std::uint32_t> indicesVec{};
    const auto addInterpolatedVertex = [&](const std::vector<uint32_t>& indices,
                                           const std::vector<float>& weights) -> uint32_t {
        const auto val = std::inner_product(
            indices.begin(), indices.end(), weights.begin(), vec3{0}, std::plus<>{},
            [&](uint32_t index, float weight) { return positions[index] * weight; });

        positions.push_back(val);
        return static_cast<uint32_t>(positions.size() - 1);
    };

    auto newEdge = meshutil::detail::sutherlandHodgman(triangle, plane, positions, indicesVec,
                                                       addInterpolatedVertex);

    ASSERT_TRUE(newEdge);
    EXPECT_EQ((*newEdge)[0], 3);
    EXPECT_EQ((*newEdge)[1], 4);

    ASSERT_EQ(indicesVec.size(), 3);
    EXPECT_EQ(indicesVec[0], 3);
    EXPECT_EQ(indicesVec[1], 2);
    EXPECT_EQ(indicesVec[2], 4);

    ASSERT_EQ(positions.size(), 5);

    EXPECT_FLOAT_EQ(positions[3][0], 0.5f);
    EXPECT_FLOAT_EQ(positions[3][1], 0.0f);

    EXPECT_FLOAT_EQ(positions[4][0], -0.5f);
    EXPECT_FLOAT_EQ(positions[4][1], 0.0f);
}

TEST(MeshCutting, GatherLoops) {
    const std::vector<vec3> positions{vec3{-1, -1, 0}, vec3{1, -1, 0}, vec3{0, 1, 0}};
    std::vector<glm::u32vec2> edges{{0,1},{1,2},{2,0}};

    const auto loops = meshutil::detail::gatherLoops(edges, positions, 0.0000001f);

    ASSERT_EQ(loops.size(), 1);
    ASSERT_EQ(loops[0].size(), 3);
}


TEST(MeshCutting, ClipMeshAgainstPlaneNew) {

    const auto mesh = meshutil::cube(mat4{1}, vec4{1, 1, 0, 1});
    const Plane plane{vec3{0.5, 0.5, 0.5}, vec3{0, 1, 0}};

    const auto clipped = meshutil::clipMeshAgainstPlaneNew(*mesh, plane, true);

    EXPECT_EQ(clipped->getIndexBuffers().front().second->getSize(), 66);

    EXPECT_EQ(clipped->getBuffers()[0].second->getSize(), 41);
    EXPECT_EQ(clipped->getBuffers()[1].second->getSize(), 41);
    EXPECT_EQ(clipped->getBuffers()[2].second->getSize(), 41);
}

}  // namespace inviwo
