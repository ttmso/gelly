# We automatically compile any auto-generated shader glue code

include(CreateShaderGlueTarget)

if (GELLY_USE_NVAPI)
    include(GetNVAPI)
    get_nvapi()
    add_compile_definitions(GELLY_USE_NVAPI)
    # copy the extension instructions over to the shader directory
    set(EXTENSION_INSTRUCTION_FILES
            nvHLSLExtns.h
            nvHLSLExtnsInternal.h
            nvShaderExtnEnums.h
    )

    foreach (EXTENSION_INSTRUCTION_FILE ${EXTENSION_INSTRUCTION_FILES})
        configure_file(
                ${NVAPI_PATH}/${EXTENSION_INSTRUCTION_FILE}
                ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/${EXTENSION_INSTRUCTION_FILE}
                COPYONLY
        )
    endforeach ()
endif ()

create_shader_glue_targets(GellyShaderCompile "Compiling the fluid renderer's shaders" ${GELLY_PRODUCTION_BUILD} ${GELLY_USE_NVAPI})
add_library(
        gelly_fluid_renderer
        STATIC
        ${SHADER_OUTPUT_FILES}
        src/v2/device.cpp
        src/v2/device.h
        src/v2/helpers/comptr.h
        src/v2/helpers/throw-informative-exception.h
        src/v2/resources/texture.cpp
        src/v2/resources/texture.h
        src/v2/resources/buffer.cpp
        src/v2/resources/buffer.h
        src/v2/helpers/parse-bind-flags.h
        src/v2/resources/shader.cpp
        src/v2/resources/shader.h
        src/v2/resources/input-layout.cpp
        src/v2/resources/input-layout.h
        src/v2/pipeline/render-pass.cpp
        src/v2/pipeline/render-pass.h
        src/v2/pipeline/binding/Input.h
        src/v2/pipeline/binding/output.h
        src/v2/pipeline/pipeline.cpp
        src/v2/pipeline/pipeline.h
        src/v2/pipeline/validate-pipeline.cpp
        src/v2/pipeline/validate-pipeline.h
        src/v2/resources/depth-buffer.cpp
        src/v2/resources/depth-buffer.h
        src/v2/resources/buffer-view.cpp
        src/v2/resources/buffer-view.h
        src/v2/renderers/splatting/pipelines/ellipsoid-splatting.h
        src/v2/renderers/splatting/pipelines/pipeline-info.h
        src/v2/renderers/splatting/texture-registry.h
        src/v2/resources/native-image.cpp
        src/v2/resources/native-image.h
        src/v2/resources/shared-image.cpp
        src/v2/resources/shared-image.h
        src/v2/resources/image.h
        src/v2/helpers/create-gsc-shader.h
        src/v2/renderers/splatting/buffers.h
        src/v2/helpers/rendering/constant-buffer.h
        src/v2/renderers/splatting/cbuffers.h
        src/v2/helpers/rendering/screen-quad.cpp
        src/v2/helpers/rendering/screen-quad.h
        src/v2/renderers/splatting/pipelines/surface-filtering.h
        src/v2/renderers/splatting/splatting-renderer.cpp
        src/v2/renderers/splatting/splatting-renderer.h
        src/v2/nvapi-extension-slot.h
)

target_include_directories(
        gelly_fluid_renderer
        PUBLIC
        src/v2
        src/shaders/out
        src/shaders/isosurface/out
        src
        ../gelly-interfaces/include
        ../gelly-fluid-sim/include
        # For the in-application API
        vendor/renderdoc/renderdoc/api/app
)

if (GELLY_USE_NVAPI)
    target_include_directories(
            gelly_fluid_renderer
            PUBLIC
            ${NVAPI_PATH}
    )
endif ()

target_link_libraries(
        gelly_fluid_renderer
        PUBLIC
        gelly_fluid_sim
        d3d11.lib
        dxguid.lib
)

if (GELLY_USE_NVAPI)
    target_link_libraries(
            gelly_fluid_renderer
            PUBLIC
            ${NVAPI_PATH}/amd64/nvapi64.lib
    )
endif ()

# If tracy is being used we need to link to it
if (TRACY_ENABLE)
    message(STATUS "Tracy is enabled, using in gelly-fluid-renderer")
    target_link_libraries(
            gelly_fluid_renderer
            PUBLIC
            TracyClient
    )
    # also include the tracy headers
    target_include_directories(
            gelly_fluid_renderer
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tracy
    )
endif ()

# Propagate GELLY_ENABLE_RENDERDOC_CAPTURES down to compile definitions
if (GELLY_ENABLE_RENDERDOC_CAPTURES)
    add_compile_definitions("GELLY_ENABLE_RENDERDOC_CAPTURES")
endif ()

add_dependencies(
        gelly_fluid_renderer
        gsc
        GellyShaderCompile
)