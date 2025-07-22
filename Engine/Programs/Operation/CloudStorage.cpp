#include "CloudStorage.hpp"
#include "Cloud.hpp"

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

    SubCloud subCloud{};
    subCloud.mesh = std::make_shared<worse::Mesh>();
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

void CloudStorage::clearAll()
{
    for (auto& [key, subCloud] : m_subClouds)
    {
        subCloud.mesh->clearAll();
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

void CloudStorageManager::clear()
{
    for (auto& [_, storage] : m_storageMap)
    {
        storage->clearAll();
    }
}
