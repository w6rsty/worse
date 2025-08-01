#include "Log.hpp"
#include "glTF/glTF.hpp"

#include "fastgltf/core.hpp"

namespace worse
{

    glTFManager::glTFManager()
    {
    }
    glTFManager::~glTFManager()
    {
    }

    glTFModel* glTFManager::load(std::string const& filepath)
    {

        fastgltf::Parser parser{};

        auto gltfFile = fastgltf::GltfDataBuffer::FromPath(filepath);
        if (gltfFile.error() != fastgltf::Error::None)
        {
            WS_LOG_ERROR("gltf", "Failed to load file {}", filepath);
            return nullptr;
        }

        std::filesystem::path parentDir = std::filesystem::path(filepath).parent_path();
        auto asset                      = parser.loadGltf(gltfFile.get(), parentDir, fastgltf::Options::None);

        if (auto error = asset.error(); error != fastgltf::Error::None)
        {
            WS_LOG_ERROR("gltf", "Failed to parse glTF {}", filepath);
            return nullptr;
        }

        // glTFModel model;

        // std::vector<glTFMesh*> meshes;
        // std::vector<ImageSlot> images;
        // std::vector<glTFMaterial*> materials;
        // std::vector<std::shared_ptr<Node>> nodes;

        // // Load samplers
        // for (auto const& sampler : asset->samplers)
        // {
        //     auto& slot = *library.GetSamplerSlot();
        //     m_device.CreateSampler(slot, {.min_filter = ExtractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest)), .mag_filter = ExtractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest)), .mipmap_mode = ExtractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest))});
        //     model.samplers.push_back(slot);
        // }

        // // Load textures
        // for (auto const& image : asset->images)
        // {
        //     ImageSlot slot = LoadImage(asset.get(), image, library);

        //     if (slot.content)
        //     {
        //         images.push_back(slot);
        //         model.images[image.name.c_str()] = slot;
        //     }
        //     else
        //     {
        //         images.push_back(library.GetDefaultErrorImage());
        //         LOG_WARN("Failed to load image {}", image.name);
        //     }
        // }

        // // Load material
        // ASSERT(!asset->materials.empty(), "glTF has no material");
        // model.material_buffer = *library.GetBufferSlot();
        // m_device.CreateBuffer(model.material_buffer, {.size = sizeof(glTFPass::MaterialConstants) * asset->materials.size(), .buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, .memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU, .name = std::format("{}_buffer", filepath.Filename().ToString())});
        // auto scene_constants = s_cast<glTFPass::MaterialConstants*>(model.material_buffer.content->info.pMappedData);

        // for (usize i = 0; i < asset->materials.size(); ++i)
        // {
        //     auto& mat = asset->materials[i];

        //     glTFMaterial* new_mat = glTFMaterialAllocator::Instance().ReuseOrAllocate();
        //     materials.push_back(new_mat);
        //     model.materials[mat.name.c_str()] = new_mat;

        //     scene_constants[i].base_color_factor.x        = mat.pbrData.baseColorFactor.x();
        //     scene_constants[i].base_color_factor.y        = mat.pbrData.baseColorFactor.y();
        //     scene_constants[i].base_color_factor.z        = mat.pbrData.baseColorFactor.z();
        //     scene_constants[i].base_color_factor.w        = mat.pbrData.baseColorFactor.w();
        //     scene_constants[i].metallicRoughnessFactors.x = mat.pbrData.metallicFactor;
        //     scene_constants[i].metallicRoughnessFactors.y = mat.pbrData.roughnessFactor;
        //     scene_constants[i].emissive_factor.x          = mat.emissiveFactor.x();
        //     scene_constants[i].emissive_factor.y          = mat.emissiveFactor.y();
        //     scene_constants[i].emissive_factor.z          = mat.emissiveFactor.z();

        //     MaterialPass pass_type = mat.alphaMode == fastgltf::AlphaMode::Blend ? MaterialPass::Transparent : MaterialPass::Opaque;

        //     glTFPass::MaterialResources resources{
        //         .base_color_texture                 = library.GetDefaultWhiteImage(),
        //         .base_color_texture_sampler         = library.GetDefaultLinearSampler(),
        //         .metallic_roughness_texture         = library.GetDefaultBlueImage(),
        //         .metallic_roughness_texture_sampler = library.GetDefaultLinearSampler(),
        //         .normal_texture                     = library.GetDefaultNormalImage(),
        //         .normal_texture_sampler             = library.GetDefaultLinearSampler(),
        //         .occlusion_texture                  = library.GetDefaultWhiteImage(),
        //         .occlusion_texture_sampler          = library.GetDefaultLinearSampler(),
        //         .emissive_texture                   = library.GetDefaultBlackImage(),
        //         .emissive_texture_sampler           = library.GetDefaultLinearSampler(),
        //         .data_buffer                        = model.material_buffer.content->buffer,
        //         .data_buffer_offset                 = i * sizeof(glTFPass::MaterialConstants),
        //     };

        //     if (mat.pbrData.baseColorTexture.has_value())
        //     {
        //         resources.base_color_texture         = images[asset->textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value()];
        //         resources.base_color_texture_sampler = model.samplers[asset->textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value()];
        //     }
        //     if (mat.pbrData.metallicRoughnessTexture.has_value())
        //     {
        //         resources.metallic_roughness_texture         = images[asset->textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value()];
        //         resources.metallic_roughness_texture_sampler = model.samplers[asset->textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].samplerIndex.value()];
        //     }
        //     if (mat.normalTexture.has_value())
        //     {
        //         resources.normal_texture         = images[asset->textures[mat.normalTexture.value().textureIndex].imageIndex.value()];
        //         resources.normal_texture_sampler = model.samplers[asset->textures[mat.normalTexture.value().textureIndex].samplerIndex.value()];
        //     }
        //     if (mat.occlusionTexture.has_value())
        //     {
        //         resources.occlusion_texture         = images[asset->textures[mat.occlusionTexture.value().textureIndex].imageIndex.value()];
        //         resources.occlusion_texture_sampler = model.samplers[asset->textures[mat.occlusionTexture.value().textureIndex].samplerIndex.value()];
        //     }
        //     if (mat.emissiveTexture.has_value())
        //     {
        //         resources.emissive_texture         = images[asset->textures[mat.emissiveTexture.value().textureIndex].imageIndex.value()];
        //         resources.emissive_texture_sampler = model.samplers[asset->textures[mat.emissiveTexture.value().textureIndex].samplerIndex.value()];
        //     }

        //     new_mat->data = m_pass.WriteMaterial(
        //         m_device.m_device,
        //         pass_type,
        //         resources,
        //         model.descriptor_allocator);
        // }

        // std::vector<Vertex> vertices;
        // std::vector<u16> indices_u16;
        // std::vector<u32> indices_u32;

        // for (auto const& mesh : asset->meshes)
        // {
        //     glTFMesh* new_mesh = glTFMeshAllocator::Instance().ReuseOrAllocate();
        //     meshes.push_back(new_mesh);
        //     model.meshes[mesh.name.c_str()] = new_mesh;
        //     new_mesh->name                  = mesh.name;

        //     vertices.clear();
        //     indices_u16.clear();
        //     indices_u32.clear();

        //     usize vertices_offset = 0;
        //     IndexType index_type  = IndexType::Uint32;

        //     for (auto const& p : mesh.primitives)
        //     {
        //         glTFSurface& new_surface = new_mesh->surfaces.emplace_back();

        //         vertices_offset = vertices.size();

        //         // load indexes
        //         fastgltf::Accessor& index_accessor = asset->accessors[p.indicesAccessor.value()];
        //         new_surface.count                  = s_cast<u32>(index_accessor.count);

        //         if (index_accessor.componentType == fastgltf::ComponentType::UnsignedShort)
        //         {
        //             new_surface.start_index = indices_u16.size();
        //             new_surface.index_type  = IndexType::Uint16;
        //             index_type              = IndexType::Uint16;

        //             indices_u16.reserve(indices_u16.size() + index_accessor.count);
        //             fastgltf::iterateAccessor<u16>(asset.get(), index_accessor, [&](u16 index)
        //                                            {
        //                                                indices_u16.push_back(index + vertices_offset);
        //                                            });
        //         }
        //         else
        //         {
        //             new_surface.start_index = indices_u32.size();
        //             new_surface.index_type  = IndexType::Uint32;
        //             index_type              = IndexType::Uint32;

        //             indices_u32.reserve(indices_u32.size() + index_accessor.count);
        //             fastgltf::iterateAccessor<u32>(asset.get(), index_accessor, [&](u32 index)
        //                                            {
        //                                                indices_u32.push_back(index + vertices_offset);
        //                                            });
        //         }

        //         // load vertex positions

        //         fastgltf::Accessor& position_accessor = asset->accessors[p.findAttribute("POSITION")->accessorIndex];
        //         // Fit current size
        //         vertices.resize(vertices.size() + position_accessor.count);

        //         fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), position_accessor, [&](glm::vec3 position, usize index)
        //                                                       {
        //                                                           Vertex new_vtx{
        //                                                               .position = position};
        //                                                           // Default values
        //                                                           new_vtx.normal                    = {1, 0, 0};
        //                                                           new_vtx.color                     = glm::vec4{1.f};
        //                                                           new_vtx.uv_x                      = 0;
        //                                                           new_vtx.uv_y                      = 0;
        //                                                           vertices[vertices_offset + index] = new_vtx;
        //                                                       });

        //         // load vertex normals
        //         auto normals = p.findAttribute("NORMAL");
        //         if (normals != p.attributes.end())
        //         {
        //             fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), asset->accessors[(*normals).accessorIndex], [&](glm::vec3 normal, usize index)
        //                                                           {
        //                                                               vertices[vertices_offset + index].normal = normal;
        //                                                           });
        //         }

        //         // load UVs
        //         auto uvs = p.findAttribute("TEXCOORD_0");
        //         if (uvs != p.attributes.end())
        //         {

        //             fastgltf::iterateAccessorWithIndex<glm::vec2>(asset.get(), asset->accessors[(*uvs).accessorIndex], [&](glm::vec2 uv, usize index)
        //                                                           {
        //                                                               vertices[vertices_offset + index].uv_x = uv.x;
        //                                                               vertices[vertices_offset + index].uv_y = uv.y;
        //                                                           });
        //         }

        //         // load vertex colors
        //         auto colors = p.findAttribute("COLOR_0");
        //         if (colors != p.attributes.end())
        //         {
        //             fastgltf::iterateAccessorWithIndex<glm::vec4>(asset.get(), asset->accessors[(*colors).accessorIndex], [&](glm::vec4 color, usize index)
        //                                                           {
        //                                                               vertices[vertices_offset + index].color = color;
        //                                                           });
        //         }

        //         new_surface.material = materials[p.materialIndex.has_value() ? p.materialIndex.value() : 0];
        //     }

        //     if (index_type == IndexType::Uint16)
        //     {
        //         new_mesh->mesh_buffer = library.UploadMesh(
        //             indices_u16.data(),
        //             indices_u16.size() * sizeof(u16),
        //             vertices);
        //     }
        //     else
        //     {
        //         new_mesh->mesh_buffer = library.UploadMesh(
        //             indices_u32.data(),
        //             indices_u32.size() * sizeof(u32),
        //             vertices);
        //     }
        // }

        // for (auto const& node : asset->nodes)
        // {
        //     std::shared_ptr<Node> new_node;

        //     if (node.meshIndex.has_value())
        //     {
        //         new_node                                    = std::make_shared<glTFMeshNode>();
        //         s_cast<glTFMeshNode*>(new_node.get())->mesh = meshes[node.meshIndex.value()];
        //     }
        //     else
        //     {
        //         new_node = std::make_shared<Node>();
        //     }

        //     nodes.push_back(new_node);
        //     model.nodes[node.name.c_str()];

        //     std::visit(fastgltf::visitor{[&](fastgltf::math::fmat4x4 matrix)
        //                                  {
        //                                      std::memcpy(&new_node->local_transform, matrix.data(), sizeof(matrix));
        //                                  },
        //                                  [&](fastgltf::TRS tranform)
        //                                  {
        //                                      glm::vec3 translation(
        //                                          tranform.translation[0],
        //                                          tranform.translation[1],
        //                                          tranform.translation[2]);
        //                                      glm::quat rotation(
        //                                          tranform.rotation[3],
        //                                          tranform.rotation[0],
        //                                          tranform.rotation[1],
        //                                          tranform.rotation[2]);
        //                                      glm::vec3 scale(
        //                                          tranform.scale[0],
        //                                          tranform.scale[1],
        //                                          tranform.scale[2]);

        //                                      glm::mat4 tm = glm::translate(glm::mat4(1.0f), translation);
        //                                      glm::mat4 rm = glm::toMat4(rotation);
        //                                      glm::mat4 sm = glm::scale(glm::mat4(1.0f), scale);

        //                                      new_node->local_transform = tm * rm * sm;
        //                                  }},
        //                node.transform);
        // }

        // // Load nodes hierarchy
        // for (usize i = 0; i < asset->nodes.size(); ++i)
        // {
        //     fastgltf::Node& node              = asset->nodes[i];
        //     std::shared_ptr<Node>& scene_node = nodes[i];

        //     for (auto& child : node.children)
        //     {
        //         scene_node->children.push_back(nodes[child]);
        //         nodes[child]->parent = scene_node;
        //     }
        // }

        // for (auto& node : nodes)
        // {
        //     if (node->parent.lock() == nullptr)
        //     {
        //         model.top_nodes.push_back(node);
        //         node->RefreshTransform(glm::mat4(1.0f));
        //     }
        // }

        // return m_model_storage.emplace_back(&model);
    }
} // namespace worse