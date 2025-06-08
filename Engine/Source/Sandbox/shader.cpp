#include "Pipeline/RHIRasterizerState.hpp"

#include <iostream>

int main()
{
    using namespace worse;

    RHIRasterizerState rasterizerState(RHIPolygonMode::Solid,
                                       RHICullMode::Back,
                                       RHIFrontFace::CW,
                                       1.0f,
                                       false,
                                       0.0f,
                                       0.0f,
                                       0.0f);

    std::cout << std::hex << rasterizerState.getHash() << std::endl;

    return 0;
}
