#pragma once
#include "Entity.hpp"
#include "Storage.hpp"
#include <tuple>
#include <type_traits>

namespace worse::ecs
{

    class World;

    template <typename... Components> class QueryView
    {
    public:
        QueryView(World& world, Storage<Entity>& entityStorage,
                  std::tuple<Storage<Components>&...> storages)
            : m_world(world), m_entityStorage(entityStorage),
              m_storages(std::move(storages))
        {
        }

        template <typename Func> void each(Func&& func)
        {
            // Find the storage with minimum size for optimal iteration
            auto minStorage = findMinimumSizeStorage();

            // Iterate over entities in the smallest storage
            iterateOverStorage(std::forward<Func>(func), minStorage);
        }

    private:
        // Helper to find the storage with minimum size for optimal iteration
        auto findMinimumSizeStorage()
        {
            // Start with entity storage size as baseline
            std::size_t minSize = m_entityStorage.size();
            std::size_t minIndex =
                std::numeric_limits<std::size_t>::max(); // Use max to indicate
                                                         // entity storage

            // Check all component storages to find the one with minimum size
            std::apply(
                [&minSize, &minIndex](auto&... storages)
                {
                    std::size_t currentIndex = 0;
                    ((storages.size() < minSize ? (minSize  = storages.size(),
                                                   minIndex = currentIndex,
                                                   0)
                                                : 0,
                      ++currentIndex),
                     ...);
                },
                m_storages);

            return std::make_pair(minSize, minIndex);
        }

        template <typename Func>
        void iterateOverStorage(
            Func&& func,
            const std::pair<std::size_t, std::size_t>& minStorageInfo)
        {
            std::size_t minIndex = minStorageInfo.second;

            if (minIndex == std::numeric_limits<std::size_t>::max())
            {
                // Entity storage is the smallest, iterate over it
                iterateOverEntityStorage(std::forward<Func>(func));
            }
            else
            {
                // One of the component storages is smallest, iterate over it
                iterateOverComponentStorage(std::forward<Func>(func), minIndex);
            }
        }

        template <typename Func> void iterateOverEntityStorage(Func&& func)
        {
            for (auto it = m_entityStorage.IndexSet::begin();
                 it != m_entityStorage.IndexSet::end();
                 ++it)
            {
                Entity entity = *it;

                // Check if entity exists in all component storages
                bool hasAllComponents = std::apply(
                    [entity](auto&... storages)
                    {
                        return (storages.contains(entity) && ...);
                    },
                    m_storages);

                if (hasAllComponents)
                {
                    // Call function with entity first, then filtered components
                    callWithEntityAndFilteredComponents(
                        std::forward<Func>(func),
                        entity);
                }
            }
        }

        template <typename Func>
        void iterateOverComponentStorage(Func&& func, std::size_t storageIndex)
        {
            // Use index-based approach to iterate over the specific storage
            iterateOverStorageAtIndex(std::forward<Func>(func),
                                      storageIndex,
                                      std::index_sequence_for<Components...>{});
        }

        template <typename Func, std::size_t... Is>
        void iterateOverStorageAtIndex(Func&& func, std::size_t targetIndex,
                                       std::index_sequence<Is...>)
        {
            // Find and iterate over the storage at the target index
            ((Is == targetIndex
                  ? iterateOverSpecificStorage(std::forward<Func>(func),
                                               std::get<Is>(m_storages))
                  : void()),
             ...);
        }

        template <typename Func, typename Storage>
        void iterateOverSpecificStorage(Func&& func, Storage& storage)
        {
            // Iterate over entities in the specific storage
            for (auto it = storage.IndexSet::begin();
                 it != storage.IndexSet::end();
                 ++it)
            {
                Entity entity = *it;

                // Check if entity exists in entity storage (should always be
                // true for valid entities)
                if (!m_entityStorage.contains(entity))
                {
                    continue;
                }

                // Check if entity exists in all OTHER component storages
                bool hasAllComponents = checkOtherStorages(
                    entity,
                    static_cast<void*>(std::addressof(storage)));

                if (hasAllComponents)
                {
                    // Call function with entity first, then filtered components
                    callWithEntityAndFilteredComponents(
                        std::forward<Func>(func),
                        entity);
                }
            }
        }

        bool checkOtherStorages(Entity entity, void* skipStoragePtr)
        {
            // Check all storages except the one we're iterating over
            return std::apply(
                [entity, skipStoragePtr](auto&... storages)
                {
                    return ((static_cast<void*>(std::addressof(storages)) ==
                                 skipStoragePtr ||
                             storages.contains(entity)) &&
                            ...);
                },
                m_storages);
        }

        template <typename Func>
        void callWithEntityAndFilteredComponents(Func&& func, Entity entity)
        {
            // Count number of non-empty components
            constexpr std::size_t numNonEmpty =
                ((!std::is_empty_v<Components>)+...);

            if constexpr (numNonEmpty == 0)
            {
                // All components are empty, call with only entity
                func(entity);
            }
            else
            {
                // Build tuple with entity first, then non-empty component
                // references
                auto componentsTuple = buildNonEmptyTuple(
                    entity,
                    std::index_sequence_for<Components...>{});
                auto fullTuple =
                    std::tuple_cat(std::make_tuple(entity), componentsTuple);
                std::apply(std::forward<Func>(func), fullTuple);
            }
        }

        template <typename Func>
        void callWithFilteredComponents(Func&& func, Entity entity)
        {
            // Count number of non-empty components
            constexpr std::size_t numNonEmpty =
                ((!std::is_empty_v<Components>)+...);

            if constexpr (numNonEmpty == 0)
            {
                // All components are empty, call with no arguments
                func();
            }
            else
            {
                // Build tuple with only non-empty component references
                auto componentsTuple = buildNonEmptyTuple(
                    entity,
                    std::index_sequence_for<Components...>{});
                std::apply(std::forward<Func>(func), componentsTuple);
            }
        }

        template <std::size_t... Is>
        auto buildNonEmptyTuple(Entity entity, std::index_sequence<Is...>)
        {
            // Helper to get component if not empty, otherwise return nothing
            auto getIfNotEmpty = [entity](auto& storage)
            {
                using ComponentType =
                    typename std::decay_t<decltype(storage)>::ValueType;
                if constexpr (std::is_empty_v<ComponentType>)
                {
                    return std::tuple<>{}; // Empty tuple for empty types
                }
                else
                {
                    return std::make_tuple(std::ref(storage.get(entity)));
                }
            };

            // Concatenate all non-empty components
            return std::tuple_cat(getIfNotEmpty(std::get<Is>(m_storages))...);
        }

    private:
        World& m_world;
        Storage<Entity>& m_entityStorage;
        std::tuple<Storage<Components>&...> m_storages;
    };
} // namespace worse::ecs