
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO inviwo/sgct
    REF 2a7c9319dc8aba97a433cb64f4cded689f36348d
    SHA512 fd12b6b1ed846d60b2914276c94b4355a4c3f21ce1125daeff2b362d993b6252b49a749b1c5b0683fc5fafac0273707c27269716b6254bfe07b0ff10f5211824
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        freetype       SGCT_FREETYPE_SUPPORT
        openvr         SGCT_OPENVR_SUPPORT
        spout2         SGCT_SPOUT_SUPPORT
        tracy          SGCT_TRACY_SUPPORT
)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DSGCT_INSTALL=ON
        -DSGCT_BUILD_TESTS=OFF
        -DSGCT_EXAMPLES=OFF
        -DSGCT_VRPN_SUPPORT=OFF
        -DSGCT_ENABLE_EDIT_CONTINUE=OFF
        -DSGCT_MEMORY_PROFILING=OFF

        -DSGCT_DEP_INCLUDE_CATCH2=OFF
        -DSGCT_DEP_INCLUDE_FMT=OFF
        -DSGCT_DEP_INCLUDE_FREETYPE=OFF
        -DSGCT_DEP_INCLUDE_GLAD=OFF
        -DSGCT_DEP_INCLUDE_GLFW=OFF
        -DSGCT_DEP_INCLUDE_GLM=OFF
        -DSGCT_DEP_INCLUDE_JSON=OFF
        -DSGCT_DEP_INCLUDE_JSON_VALIDATOR=OFF
        -DSGCT_DEP_INCLUDE_LIBPNG=OFF
        -DSGCT_DEP_INCLUDE_OPENVR=OFF
        -DSGCT_DEP_INCLUDE_SCN=OFF
        -DSGCT_DEP_INCLUDE_STB=OFF
        -DSGCT_DEP_INCLUDE_TINYXML=OFF
        -DSGCT_DEP_INCLUDE_TRACY=OFF
        -DSGCT_DEP_INCLUDE_VRPN=OFF
        -DSGCT_DEP_INCLUDE_ZLIB=OFF

        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH share/sgct) 

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(
    INSTALL ${SOURCE_PATH}/LICENSE.md 
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/sgct 
    RENAME copyright
)
