#include "RHICommandList.hpp"
#include "Renderer.hpp"
#include "Pipeline/RHIPipelineState.hpp"

namespace worse
{

    void produceFrame(RHICommandList* cmdList)
    {
        // get render target

        // declare pso
        RHIPipelineState pso;
        pso.name              = "RenderPassPso";
        pso.type              = RHIPipelineType::Graphics;
        pso.primitiveTopology = RHIPrimitiveTopology::Trianglelist;
        /// ...

        // cmdList->setPipelineState(pso);

        // for (drawCall : drawCalls)
        // cmdList->pushConstants
        // cmdList->setVertexBuffer
        // cmdList->setIndexBuffer
        // cmdList->drawIndexed
    }
} // namespace worse