#pragma once
#include "Math/Math.hpp"
#include "Prefab.hpp"
#include "Geometry/GeometryGeneration.hpp"
#include "RendererDefinitions.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Renderable.hpp"

#include "ECS/Resource.hpp"
#include "ECS/Commands.hpp"

#include <cstdint>

namespace worse
{

    class Renderer
    {
    public:
        static void initialize(ecs::Commands commands);
        static void shutdown(ecs::Commands commands);
        static void tick(ecs::Resource<DrawcallStorage> drawcalls,
                         ecs::Resource<Camera> camera,
                         ecs::Resource<GlobalContext> globalContext,
                         ecs::ResourceArray<TextureWrite> textureWrites);

        // swapchain
        static RHISwapchain* getSwapchain();
        static void blitToBackBuffer(RHICommandList* cmdList);
        // submit command list and present image to swapchain
        static void submitAndPresent();

        static void
        createMaterialBuffers(std::span<StandardMaterialGPU> materials);

        static void
        writeBindlessTextures(ecs::ResourceArray<TextureWrite> textureWrites);

        static void setViewport(float const width, float const height);
        static RHIViewport const& getViewport();

        static RHIFormat getSwapchainFormat();

        static RHIRasterizerState*
        getRasterizerState(RendererRasterizerState const state);
        static RHIDepthStencilState*
        getDepthStencilState(RendererDepthStencilState const state);
        static RHIBlendState* getBlendState(RendererBlendState const state);
        static RHITexture* getRenderTarget(RendererTarget const target);
        static RHIShader* getShader(RendererShader const shader);
        static RHITexture* getTexture(RendererTexture const texture);
        static RHISampler* getSampler(RHISamplerType const sampler);
        static Mesh* getStandardMesh(geometry::GeometryType const type);
        static RHIPipelineState const& getPipelineState(RendererPSO const pso);
        static RHIBuffer* getMaterialBuffer();

        static math::Vector2 getResolutionRender();
        static math::Vector2 getResolutionOutput();

        static void setPushParameters(float a, float b);
        static void setCameraPosition(math::Vector3 const& position);
        static void setCameraForward(math::Vector3 const& forward);

    private:
        static void updateBuffers(RHICommandList* cmdList,
                                  ecs::Resource<Camera> camera,
                                  ecs::Resource<GlobalContext> globalContext);

        // =====================================================================
        // Resources
        // =====================================================================

        static void createRasterizerStates();
        static void createDepthStencilStates();
        static void createBlendStates();
        static void createRendererTarget();
        static void createShaders();
        static void createTextures();
        static void createSamplers();
        static void createStandardMeshes();
        static void createPipelineStates();

        static void destroyResources();

        // =====================================================================
        // Passes
        // =====================================================================

        static void passDpethPrepass(RHICommandList* cmdList,
                                     ecs::Resource<DrawcallStorage> drawcalls);
        static void passColor(RHICommandList* cmdList,
                              ecs::Resource<DrawcallStorage> drawcalls);
        static void passWireFrame(RHICommandList* cmdList,
                                  ecs::Resource<DrawcallStorage> drawcalls);
        static void passPostProcessing(RHICommandList* cmdList);

        static void passImGui(RHICommandList* cmdList);

        static void
        produceFrame(RHICommandList* cmdList,
                     ecs::Resource<GlobalContext> globalContext,
                     ecs::Resource<DrawcallStorage> drawcalls,
                     ecs::ResourceArray<TextureWrite> textureWrites);

    private:
        static inline std::uint64_t s_frameCount = 0;
    };

} // namespace worse