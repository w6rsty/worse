#include "base_type.hpp"
#include "math/transform.hpp"
#include "glTF/glTF.hpp"
#include "AssetServer.hpp"
#include "Log.hpp"
#include "RHITypes.hpp"

#include "MathElementTraits.hpp" // IWYU pragma: keep

#include <span>
#include <optional>
#include <filesystem>

namespace Worse
{
    namespace
    {
        /**
         * @brief Load glTF using AssetServer
         */
        std::optional<AssetHandle> loadTexture(
            fastgltf::Image const& image,
            fastgltf::Asset const& asset,
            AssetServer& assetServer,
            std::filesystem::path const& parentDir,
            std::string const& imageName)
        {
            AssetHandle handle = {};

            // clang-format off
            std::visit(fastgltf::visitor{
                [&](fastgltf::sources::BufferView view)
                {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer      = asset.buffers[bufferView.bufferIndex];

                    std::visit(fastgltf::visitor{
                        [&](fastgltf::sources::Vector vector)
                        {
                            std::span<Byte> data{r_cast<Byte*>(vector.bytes.data() + bufferView.byteOffset), bufferView.byteLength};
                            handle = assetServer.addTexture(data, imageName);
                        },
                        [&](fastgltf::sources::Array array)
                        {
                            std::span<Byte> data{r_cast<Byte*>(array.bytes.data() + bufferView.byteOffset), bufferView.byteLength};
                            handle = assetServer.addTexture(data, imageName);
                        },
                        [&](auto arg)
                        {
                            WS_LOG_ERROR("glTF", "Unsupported buffer view type");
                        }}, buffer.data);
                },
                [&](fastgltf::sources::URI path)
                {
                    WS_ASSERT(path.fileByteOffset == 0);
                    WS_ASSERT(path.uri.isLocalPath());

                    handle = assetServer.addTexture(parentDir / path.uri.fspath(), AssetServer::LoadStrategy::Immediate);
                },
                [&](fastgltf::sources::Vector vector)
                {
                    std::span<Byte> data{r_cast<Byte*>(vector.bytes.data()), vector.bytes.size()};
                    handle = assetServer.addTexture(data, imageName);
                },
                [&](auto arg)
                {
                    WS_LOG_ERROR("glTF", "Unsupported image source type");
                },
            }, image.data);
            // clang-format on

            return handle;
        }

        /**
         * @brief Compute tangent and bitanget
         * @details Mikkelsen's algorithm
         */
        void calculateTangent(std::span<RHIVertexPosUvNrmTan> vertices, std::span<UInt> indices)
        {
            // Store per-face data before averaging
            std::vector<Vector3> tan1(vertices.size(), Vector3::ZERO);
            std::vector<Vector3> tan2(vertices.size(), Vector3::ZERO);

            // Iterate over all triangles in the primitive
            for (Size i = 0; i < indices.size(); i += 3)
            {
                UInt i0 = indices[i];
                UInt i1 = indices[i + 1];
                UInt i2 = indices[i + 2];

                RHIVertexPosUvNrmTan const& v0 = vertices[i0];
                RHIVertexPosUvNrmTan const& v1 = vertices[i1];
                RHIVertexPosUvNrmTan const& v2 = vertices[i2];

                // Calculate edge vectors
                Vector3 edge1 = v1.position - v0.position;
                Vector3 edge2 = v2.position - v0.position;

                // Calculate UV deltas
                Vector2 uv1Delta = v1.uv - v0.uv;
                Vector2 uv2Delta = v2.uv - v0.uv;

                Float f = 1.0f / (uv1Delta.x * uv2Delta.y - uv2Delta.x * uv1Delta.y);

                Vector3 tangent;
                Vector3 bitangent;

                // Check for division by zero (e.g., degenerate UVs)
                if (std::isinf(f) || std::isnan(f))
                {
                    // Default to something safe or just skip
                    tangent   = Vector3(1.0f, 0.0f, 0.0f);
                    bitangent = Vector3(0.0f, 1.0f, 0.0f);
                }
                else
                {
                    tangent   = (edge1 * uv2Delta.y - edge2 * uv1Delta.y) * f;
                    bitangent = (edge1 * (-uv2Delta.x) + edge2 * uv1Delta.x) * f;
                }

                // Accumulate tangent and bitangent for each vertex
                // Use verticesOffset to correctly index into the current primitives vertices
                tan1[i0] += tangent;
                tan1[i1] += tangent;
                tan1[i2] += tangent;
            }

            // Now, average and orthogonalized the tangents
            for (Size i = 0; i < vertices.size(); ++i)
            {
                Vector3 const& n = vertices[i].normal;
                Vector3 const& t = tan1[i];

                // Gram-Schmidt orthogonalization: T' = T - (N · T)N
                // Ensure the tangent is orthogonal to the normal
                Vector3 tangent = Normalize(t - n * DotProduct(n, t));

                // Calculate bitangent to find handedness
                Vector3 bitangent = Normalize(CrossProduct(n, t));

                // Determine the handedness (the w component of the tangent)
                // This is necessary for some normal mapping implementations
                Float handedness = (DotProduct(CrossProduct(n, t), bitangent) < 0.0f) ? -1.0f : 1.0f;

                vertices[i].tangent = Vector4(tangent, handedness);
            }
        }

    } // namespace

    void glTFMeshNode::draw(Matrix4 const& topMat, DrawContext& ctx)
    {
        Matrix4 const nodeTransform = topMat * localTransform;

        for (glTFSurface const& surface : mesh->surfaces)
        {
            ctx.opaqueObjects.push_back(RenderObject{
                .indexCount = surface.indexCount,
                .startIndex = surface.startIndex,
                .mesh       = mesh->mesh.get(),
                .material   = surface.material,
                .transform  = nodeTransform,
            });
        }

        Node::draw(topMat, ctx);
    }

    glTFManager::glTFManager(AssetServer& assetServer)
        : m_assetServer(assetServer)
    {
    }

    glTFManager::~glTFManager()
    {
    }

    glTFModel* glTFManager::load(std::string const& filepath, std::string const& modelName)
    {
        std::lock_guard<std::mutex> lock(m_mtxParser);

        auto gltfFile = fastgltf::GltfDataBuffer::FromPath(filepath);
        if (gltfFile.error() != fastgltf::Error::None)
        {
            WS_LOG_ERROR("glTF", "Failed to load file {}", filepath);
            return nullptr;
        }

        std::filesystem::path parentDir = std::filesystem::path(filepath).parent_path();
        fastgltf::Options options       = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages;
        auto asset                      = m_parser.loadGltf(gltfFile.get(), parentDir, options);

        if (auto error = asset.error(); error != fastgltf::Error::None)
        {
            WS_LOG_ERROR("glTF", "Failed to parse glTF {}", filepath);
            return nullptr;
        }

        std::unique_ptr<glTFModel> model = std::make_unique<glTFModel>();

        // TODO: Read Samplers

        // =====================================================================
        // Read textures
        // =====================================================================

        // Stage loaded texture handles, assign default for failure
        std::vector<AssetHandle> textures;
        textures.reserve(asset->images.size());

        UInt anonymousTextureIndex = 0;
        for (fastgltf::Image const& image : asset->images)
        {
            // Generate unique name, because AssetServer needs one
            std::string textureName = modelName;
            if (image.name.empty())
            {
                textureName += "_anonymous_image_" + std::to_string(anonymousTextureIndex++);
            }
            else
            {
                textureName += "_" + image.name;
            }

            if (std::optional<AssetHandle> handle = loadTexture(image, asset.get(), m_assetServer, parentDir, textureName))
            {
                textures.push_back(handle.value());
            }
            else
            {
                textures.push_back(m_assetServer.getErrorTexture());
                WS_LOG_WARN("gltf", "Faile to load {}", textureName);
            }
        }

        // =====================================================================
        // Read textures
        // =====================================================================

        std::vector<AssetHandle> materials;
        materials.reserve(asset->materials.size());

        for (fastgltf::Material const& material : asset->materials)
        {
            AssetHandle materialHandle = m_assetServer.addMaterial(StandardMaterial{
                .baseColor                = Vector4(material.pbrData.baseColorFactor.x(), material.pbrData.baseColorFactor.y(), material.pbrData.baseColorFactor.z(), material.pbrData.baseColorFactor.w()),
                .baseColorTexture         = material.pbrData.baseColorTexture.has_value() ? std::make_optional(textures[material.pbrData.baseColorTexture->textureIndex]) : std::nullopt,
                .normalTexture            = material.normalTexture.has_value() ? std::make_optional(textures[material.normalTexture->textureIndex]) : std::nullopt,
                .metallic                 = material.pbrData.metallicFactor,
                .metallicRoughnessTexture = material.pbrData.metallicRoughnessTexture.has_value() ? std::make_optional(textures[material.pbrData.metallicRoughnessTexture->textureIndex]) : std::nullopt,
                .roughness                = material.pbrData.roughnessFactor,
                .ambientOcclusionTexture  = material.occlusionTexture.has_value() ? std::make_optional(textures[material.occlusionTexture->textureIndex]) : std::nullopt,
                .emissive                 = Vector4(material.emissiveFactor.x(), material.emissiveFactor.y(), material.emissiveFactor.z(), 1.0f),
                .emissiveTexture          = material.emissiveTexture.has_value() ? std::make_optional(textures[material.emissiveTexture->textureIndex]) : std::nullopt,
            });

            model->textures[std::string{material.name}] = materialHandle;
            materials.push_back(materialHandle);
        }

        // =====================================================================
        // Read vertices and indices
        // =====================================================================

        std::vector<std::shared_ptr<glTFMesh>> meshes;
        std::vector<RHIVertexPosUvNrmTan> vertices;
        std::vector<UInt> indices;

        for (fastgltf::Mesh const& mesh : asset->meshes)
        {
            std::shared_ptr<glTFMesh> newMesh = std::make_shared<glTFMesh>();
            newMesh->name                     = mesh.name;

            vertices.clear();
            indices.clear();

            UInt verticesOffset = 0;

            for (fastgltf::Primitive const& primitive : mesh.primitives)
            {
                glTFSurface& newSurface = newMesh->surfaces.emplace_back();

                verticesOffset = vertices.size();

                // load indexes
                fastgltf::Accessor& indexAccessor = asset->accessors[primitive.indicesAccessor.value()];
                newSurface.indexCount             = static_cast<UInt>(indexAccessor.count);

                if (indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort)
                {
                    newSurface.startIndex = indices.size();

                    indices.reserve(indices.size() + indexAccessor.count);
                    fastgltf::iterateAccessor<UShort>(
                        asset.get(),
                        indexAccessor,
                        [&](UShort index)
                        {
                            indices.push_back(static_cast<UInt>(index) + verticesOffset);
                        });
                }
                else
                {
                    newSurface.startIndex = indices.size();

                    indices.reserve(indices.size() + indexAccessor.count);
                    fastgltf::iterateAccessor<UInt>(
                        asset.get(),
                        indexAccessor,
                        [&](UInt index)
                        {
                            indices.push_back(index + verticesOffset);
                        });
                }

                // load vertex positions

                fastgltf::Accessor& positionAccessor = asset->accessors[primitive.findAttribute("POSITION")->accessorIndex];
                // Fit current size
                vertices.resize(vertices.size() + positionAccessor.count);

                fastgltf::iterateAccessorWithIndex<Vector3>(
                    asset.get(),
                    positionAccessor,
                    [&](Vector3 const& position, Size index)
                    {
                        RHIVertexPosUvNrmTan& vtx = vertices[verticesOffset + index];
                        vtx.position              = position;
                        // default
                        vtx.normal  = Vector3::ZERO;
                        vtx.uv      = Vector2::ZERO;
                        vtx.tangent = Vector4::ZERO;
                    });

                // load vertex normals
                fastgltf::Attribute const* normals = primitive.findAttribute("NORMAL");
                if (normals != primitive.attributes.end())
                {
                    fastgltf::iterateAccessorWithIndex<Vector3>(
                        asset.get(),
                        asset->accessors[normals->accessorIndex],
                        [&](Vector3 const& normal, Size index)
                        {
                            RHIVertexPosUvNrmTan& vtx = vertices[verticesOffset + index];
                            // glTF -> Vulkan
                            vtx.normal = -normal;
                        });
                }
                else
                {
                    WS_LOG_WARN("gltf", "{} does not have attribute NORMAL", modelName);
                }

                // load UVs
                fastgltf::Attribute const* uvs = primitive.findAttribute("TEXCOORD_0");
                if (uvs != primitive.attributes.end())
                {

                    fastgltf::iterateAccessorWithIndex<Vector2>(
                        asset.get(),
                        asset->accessors[uvs->accessorIndex],
                        [&](Vector2 const& uv, Size index)
                        {
                            RHIVertexPosUvNrmTan& vtx = vertices[verticesOffset + index];
                            vtx.uv                    = uv;
                        });
                }
                else
                {
                    WS_LOG_WARN("gltf", "{} does not have attribute TEXCOORD_0", modelName);
                }

                // load tangents
                fastgltf::Attribute const* tangents = primitive.findAttribute("TANGENT");
                if (tangents != primitive.attributes.end())
                {
                    fastgltf::iterateAccessorWithIndex<Vector4>(
                        asset.get(),
                        asset->accessors[tangents->accessorIndex],
                        [&](Vector4 const& tangent, Size index)
                        {
                            RHIVertexPosUvNrmTan& vtx = vertices[verticesOffset + index];
                            vtx.tangent               = tangent;
                        });
                }
                {
                    WS_LOG_WARN("gltf", "{} does not have attribute TANGENT", modelName);

                    // TODO: Compute tangent
                    calculateTangent(vertices, indices);
                }

                newSurface.material = materials[primitive.materialIndex.has_value() ? primitive.materialIndex.value() : 0];
            }

            newMesh->mesh = std::make_unique<Mesh>();
            newMesh->mesh->addGeometry(vertices, indices);
            newMesh->mesh->createGPUBuffers();
            newMesh->mesh->clearCPU();

            meshes.push_back(newMesh);
            model->meshes[std::string{mesh.name}] = newMesh;
        }

        // =====================================================================
        // Read nodes
        // =====================================================================

        std::vector<std::shared_ptr<Node>> nodes;
        for (fastgltf::Node const& node : asset->nodes)
        {
            std::shared_ptr<Node> newNode;

            // glTFMeshNode for node has mesh, otherwise normal Node
            if (node.meshIndex.has_value())
            {
                std::shared_ptr<glTFMeshNode> meshNode = std::make_shared<glTFMeshNode>();

                meshNode->mesh = meshes[node.meshIndex.value()];
                newNode        = meshNode;
            }
            else
            {
                newNode = std::make_shared<Node>();
            }

            nodes.push_back(newNode);
            model->nodes[std::string{node.name}];

            std::visit(
                fastgltf::visitor{
                    [&](fastgltf::math::fmat4x4 matrix)
                    {
                        std::memcpy(newNode->localTransform.data, matrix.data(), sizeof(Matrix4));
                    },
                    [&](fastgltf::TRS tranform)
                    {
                        Vector3 translation{tranform.translation[0], tranform.translation[1], tranform.translation[2]};
                        math::Quaternion rotation{tranform.rotation[3], tranform.rotation[0], tranform.rotation[1], tranform.rotation[2]};
                        Vector3 scale{tranform.scale[0], tranform.scale[1], tranform.scale[2]};

                        Matrix4 tm = math::makeTranslation(translation);
                        Matrix4 rm = rotation.toMat4();
                        Matrix4 sm = math::makeScale(scale);

                        newNode->localTransform = tm * rm * sm;
                    }},
                node.transform);
        }

        // =====================================================================
        // Hierarchy
        // =====================================================================

        for (Size i = 0; i < asset->nodes.size(); ++i)
        {
            fastgltf::Node& node             = asset->nodes[i];
            std::shared_ptr<Node>& sceneNode = nodes[i];

            for (auto& child : node.children)
            {
                sceneNode->children.push_back(nodes[child]);
                nodes[child]->parent = sceneNode;
            }
        }

        for (std::shared_ptr<Node>& node : nodes)
        {
            if (!node->parent.lock())
            {
                model->topNodes.push_back(node);
                node->refreshTransform(Matrix4::IDENTITY);
            }
        }

        return m_modelStorage.emplace(modelName, std::move(model)).first->second.get();
    }

    glTFModel* glTFManager::getModel(std::string const& modelName)
    {
        std::lock_guard<std::mutex> lock(m_mtxParser);

        auto it = m_modelStorage.find(modelName);
        if (it != m_modelStorage.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

} // namespace Worse