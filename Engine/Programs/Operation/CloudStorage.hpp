#pragma once
#include "../PointCloud/Cloud.hpp"

#include "Mesh.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// 点云子集
struct SubCloud
{
    std::shared_ptr<worse::Mesh> mesh;
};

// 管理原始点云数据和子集
class CloudStorage
{
public:
    using SubCloudKey = std::string;
    using FilterFn    = std::vector<worse::RHIVertexPos> (*)(worse::pc::Cloud const&);

    // 从文件中加载一个点云s
    explicit CloudStorage(std::string_view filePath);
    ~CloudStorage();

    // 从主点云中创建一个子点云
    void createSubCloud(SubCloudKey const& key, FilterFn fn);
    void removeSubCloud(SubCloudKey const& key);
    void clearAll();

    SubCloud& getSubCloud(SubCloudKey const& key)
    {
        return m_subClouds.at(key);
    }

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