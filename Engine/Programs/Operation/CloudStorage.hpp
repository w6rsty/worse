#pragma once
#include "../PointCloud/Cloud.hpp"

#include "ECS/Commands.hpp"
#include "ECS/Entity.hpp"
#include "Mesh.hpp"

#include <memory>
#include <string>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

// 点云子集
struct SubCloud
{
    std::size_t count = 0;

    worse::ecs::Entity entity = worse::ecs::Entity::null();
    std::shared_ptr<worse::Mesh> mesh;
};

// 管理原始点云数据和子集
class CloudStorage
{
public:
    using SubCloudKey = std::string;
    using FilterFn    = std::function<std::vector<worse::RHIVertexPos>(worse::pc::Cloud const&)>;

    // 从文件中加载一个点云s
    explicit CloudStorage(std::string_view filePath);
    ~CloudStorage();

    // 从主点云中创建一个子点云
    void createSubCloud(SubCloudKey const& key, FilterFn fn);
    void removeSubCloud(SubCloudKey const& key);
    void destroyEntities(worse::ecs::Commands commands);
    void clearAll();

    SubCloud& getSubCloud(SubCloudKey const& key)
    {
        return m_subClouds.at(key);
    }

    // 获取所有子点云的键名
    std::vector<SubCloudKey> getSubCloudKeys() const
    {
        std::vector<SubCloudKey> keys;
        keys.reserve(m_subClouds.size());
        for (auto const& pair : m_subClouds)
        {
            keys.push_back(pair.first);
        }
        return keys;
    }

    // 获取子点云数量
    std::size_t getSubCloudCount() const
    {
        return m_subClouds.size();
    }

    // 获取活跃Entity数量
    std::size_t getActiveEntityCount() const
    {
        std::size_t count = 0;
        for (const auto& [key, subCloud] : m_subClouds)
        {
            if (subCloud.entity != worse::ecs::Entity::null())
            {
                count++;
            }
        }
        return count;
    }

    // 设置SubCloud的可见性
    void setSubCloudVisibility(SubCloudKey const& key, bool visible)
    {
        auto it = m_subClouds.find(key);
        if (it != m_subClouds.end() && it->second.mesh)
        {
            it->second.mesh->setVisible(visible);
        }
    }

    // 自动隐藏大型子点云（点数远大于其他子点云的）
    void autoHideLargeSubClouds(double threshold_ratio = 5.0);

    worse::pc::Cloud const& getMasterCloud() const
    {
        return m_masterCloud;
    }

private:
    worse::pc::Cloud m_masterCloud;
    std::unordered_map<SubCloudKey, SubCloud> m_subClouds;
};

class CloudStorageManager
{
public:
    bool has(std::string const& filePath) const;
    CloudStorage& load(std::string const& filePath);
    void remove(std::string const& filePath);
    CloudStorage* get(std::string const& filePath);
    void destroyEntities(worse::ecs::Commands commands);
    void clear();

    std::size_t size() const
    {
        return m_storageMap.size();
    }

    bool empty() const
    {
        return m_storageMap.empty();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<CloudStorage>> m_storageMap;
};