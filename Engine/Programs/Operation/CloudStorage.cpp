#include "CloudStorage.hpp"
#include "Cloud.hpp"
#include <algorithm>
#include <vector>

using namespace worse;

CloudStorage::CloudStorage(std::string_view filePath)
{
    m_masterCloud = pc::load(filePath);
}

CloudStorage::~CloudStorage()
{
    clearAll();
}

void CloudStorage::createSubCloud(SubCloudKey const& key, FilterFn fn)
{
    if (m_subClouds.find(key) != m_subClouds.end())
    {
        return;
    }

    std::vector<worse::RHIVertexPos> subPoints = fn(m_masterCloud);

    SubCloud subCloud{
        .count = subPoints.size(),
        .mesh  = std::make_shared<worse::Mesh>()};
    subCloud.mesh->addGeometry(
        RHIVertexType::Pos,
        {reinterpret_cast<std::byte*>(subPoints.data()), subPoints.size() * sizeof(worse::RHIVertexPos)},
        std::vector<std::uint32_t>{});
    subCloud.mesh->createGPUBuffers();
    subCloud.mesh->clearCPU();

    m_subClouds.emplace(key, std::move(subCloud));
}

void CloudStorage::removeSubCloud(SubCloudKey const& key)
{
    auto it = m_subClouds.find(key);
    if (it != m_subClouds.end())
    {
        it->second.mesh->clearAll();
        m_subClouds.erase(it);
    }
}

void CloudStorage::destroyEntities(ecs::Commands commands)
{
    for (auto& [key, subCloud] : m_subClouds)
    {
        if (subCloud.entity != ecs::Entity::null())
        {
            commands.destroy(subCloud.entity);
            subCloud.entity = ecs::Entity::null();
        }
    }
}

void CloudStorage::clearAll()
{
    for (auto& [key, subCloud] : m_subClouds)
    {
        subCloud.mesh->clearAll();
    }
}

void CloudStorage::autoHideLargeSubClouds(double threshold_ratio)
{
    if (m_subClouds.size() < 2)
    {
        return;
    }

    // 计算所有子点云的点数统计
    std::vector<std::pair<std::string, std::size_t>> cloudSizes;
    std::size_t totalPoints = 0;

    for (const auto& [key, subCloud] : m_subClouds)
    {
        cloudSizes.emplace_back(key, subCloud.count);
        totalPoints += subCloud.count;
    }

    if (cloudSizes.empty())
    {
        return;
    }

    // 按点数排序
    std::sort(cloudSizes.begin(), cloudSizes.end(), [](const auto& a, const auto& b)
              {
                  return a.second > b.second;
              });

    // 计算中位数大小
    std::size_t medianSize = cloudSizes[cloudSizes.size() / 2].second;

    // 隐藏远大于中位数的子点云
    int hiddenCount = 0;
    for (const auto& [key, size] : cloudSizes)
    {
        if (size > medianSize * threshold_ratio && size > 10000)
        {
            setSubCloudVisibility(key, false);
            hiddenCount++;
        }
    }

    // 输出日志信息
    if (hiddenCount > 0)
    {
        WS_LOG_INFO("CloudStorage", "Auto-hidden {} large SubClouds (threshold: {:.1f}x median)", hiddenCount, threshold_ratio);
    }
}

bool CloudStorageManager::has(std::string const& filePath) const
{
    return m_storageMap.find(filePath) != m_storageMap.end();
}

CloudStorage& CloudStorageManager::load(std::string const& filePath)
{
    if (m_storageMap.find(filePath) != m_storageMap.end())
    {
        return *m_storageMap[filePath];
    }

    auto storage = std::make_shared<CloudStorage>(filePath);
    m_storageMap.emplace(filePath, std::move(storage));
    return *m_storageMap[filePath];
}

void CloudStorageManager::remove(std::string const& filePath)
{
    auto it = m_storageMap.find(filePath);
    if (it != m_storageMap.end())
    {
        m_storageMap.erase(it);
    }
}

CloudStorage* CloudStorageManager::get(std::string const& filePath)
{
    auto it = m_storageMap.find(filePath);
    if (it != m_storageMap.end())
    {
        return it->second.get();
    }
    else
    {
        return nullptr;
    }
}

void CloudStorageManager::destroyEntities(worse::ecs::Commands commands)
{
    for (auto& [_, storage] : m_storageMap)
    {
        storage->destroyEntities(commands);
    }
}

void CloudStorageManager::clear()
{
    for (auto& [_, storage] : m_storageMap)
    {
        storage->clearAll();
    }
}
