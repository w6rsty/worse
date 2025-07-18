#pragma once
#include "Log.hpp"
#include "Registry.hpp"
#include "System.hpp"

#include <string>
#include <vector>
#include <typeindex>
#include <algorithm>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

namespace worse::ecs
{

    class Stage
    {
        using SystemType = SystemWrapper::Function;

    public:
        Stage() = default;

        void addSystem(SystemType&& system)
        {
            if (m_systemTypes.count(system.index) > 0)
            {
                for (auto& existingSystem : m_systems)
                {
                    if (existingSystem.index == system.index)
                    {
                        existingSystem.ptr = system.ptr;
                        return;
                    }
                }
            }
            else
            {
                m_systemTypes.insert(system.index);
                m_systems.push_back(std::move(system));
            }
        }

        // 昂贵的操作，不要在每帧调用
        void removeSystem(std::uintptr_t index)
        {
            if (m_systemTypes.count(index) == 0)
            {
                return;
            }

            // clang-format off
            m_systems.erase(
                std::remove_if(m_systems.begin(), m_systems.end(),
                    [index](SystemType const& system)
                    {
                        return system.index == index;
                    }
                ),
                m_systems.end()
            );
            // clang-format on
            m_systemTypes.erase(index);
        }

        void run(Registry& registry) const
        {
            for (SystemType const& system : m_systems)
            {
                system(registry);
            }
        }

    private:
        std::vector<SystemType> m_systems;
        std::unordered_set<std::uintptr_t> m_systemTypes;
    };

    namespace CoreStage
    {
        // clang-format off
        // initialization state, run once
        struct StartUp {};

        // main update loop, run every frame
        struct Update {};
        struct PostUpdate {};

        struct CleanUp {};

        // clang-format on
    } // namespace CoreStage

    class Schedule
    {
        using StageLabelType = std::type_index;
        using SystemType     = SystemWrapper::Function;

    public:
        Schedule(std::string_view name = "DefaultSchedule")
        {
            m_name = name;

            // clang-format off
            m_startUpStage = std::make_unique<Stage>();

            addStage<CoreStage::Update>();
            addStage<CoreStage::PostUpdate>();

            m_cleanUpStage = std::make_unique<Stage>();
            // clang-format on
        }

        template <typename StageLabel> Schedule& addStage()
        {
            if constexpr (std::is_same_v<StageLabel, CoreStage::StartUp> ||
                          std::is_same_v<StageLabel, CoreStage::CleanUp>)
            {
                WS_LOG_WARN("ECS", "Cannot add CoreStage");
                return *this;
            }

            std::type_index const label(typeid(StageLabel));
            if (m_stages.count(label))
            {
                WS_LOG_WARN("ECS", "Stage {} already exists.", label.name());
                return *this;
            }

            m_stageOrder.push_back(label);
            m_stages.emplace(label, std::make_unique<Stage>());
            return *this;
        }

        template <typename StageLabel, auto Func> Schedule& addSystem()
        {
            if constexpr (std::is_same_v<StageLabel, CoreStage::StartUp>)
            {
                m_startUpStage->addSystem(SystemWrapper::wrap<Func>());
                return *this;
            }
            else if constexpr (std::is_same_v<StageLabel, CoreStage::CleanUp>)
            {
                m_cleanUpStage->addSystem(SystemWrapper::wrap<Func>());
                return *this;
            }
            else
            {
                std::type_index const label(typeid(StageLabel));
                if (!m_stages.count(label))
                {
                    WS_LOG_WARN("ECS",
                                "Stage {} does not exist.",
                                label.name());
                    return *this;
                }

                m_stages.at(label)->addSystem(SystemWrapper::wrap<Func>());
                return *this;
            }
        }

        template <typename StageLabel, auto Func> void removeSystem()
        {
            if constexpr (std::is_same_v<StageLabel, CoreStage::StartUp>)
            {
                m_startUpStage->removeSystem(
                    reinterpret_cast<std::uintptr_t>(Func));
            }
            else if constexpr (std::is_same_v<StageLabel, CoreStage::CleanUp>)
            {
                m_cleanUpStage->removeSystem(
                    reinterpret_cast<std::uintptr_t>(Func));
            }
            else
            {
                std::type_index const label(typeid(StageLabel));
                if (!m_stages.count(label))
                {
                    WS_LOG_WARN("ECS",
                                "Stage {} does not exist.",
                                label.name());
                    return;
                }

                m_stages.at(label)->removeSystem(
                    reinterpret_cast<std::uintptr_t>(Func));
            }
        }

        template <typename StageLabel> bool removeStage()
        {
            if constexpr (std::is_same_v<StageLabel, CoreStage::StartUp> ||
                          std::is_same_v<StageLabel, CoreStage::Update> ||
                          std::is_same_v<StageLabel, CoreStage::PostUpdate> ||
                          std::is_same_v<StageLabel, CoreStage::CleanUp>)
            {
                WS_LOG_WARN("ECS", "Cannot remove CoreStage");
            }

            std::type_index const label(typeid(StageLabel));
            if (!m_stages.count(label))
            {
                return false;
            }

            // Remove from stage order
            auto it =
                std::find(m_stageOrder.begin(), m_stageOrder.end(), label);
            if (it != m_stageOrder.end())
            {
                m_stageOrder.erase(it);
            }

            // Remove from stages map
            m_stages.erase(label);
            return true;
        }

        template <typename StageLabel, typename BeforeStageLabel>
        Schedule& insertStageBefore()
        {
            if constexpr (std::is_same_v<StageLabel, CoreStage::StartUp> ||
                          std::is_same_v<BeforeStageLabel, CoreStage::StartUp>)
            {
                WS_LOG_WARN("ECS", "Cannot insert with CoreStage::StartUp.");
                return *this;
            }
            else if constexpr (std::is_same_v<StageLabel, CoreStage::CleanUp> ||
                               std::is_same_v<BeforeStageLabel,
                                              CoreStage::CleanUp>)
            {
                WS_LOG_WARN("ECS", "Cannot insert with CoreStage::CleanUp.");
                return *this;
            }

            std::type_index const newLabel(typeid(StageLabel));
            std::type_index const beforeLabel(typeid(BeforeStageLabel));

            // Check if new stage already exists
            if (m_stages.count(newLabel))
            {
                WS_LOG_WARN("ECS", "Stage {} already exists.", newLabel.name());
                return *this;
            }

            // Find the position to insert before
            auto it = std::find(m_stageOrder.begin(),
                                m_stageOrder.end(),
                                beforeLabel);
            if (it == m_stageOrder.end())
            {
                WS_LOG_WARN("ECS", "Stage {} not found.", beforeLabel.name());
                return *this;
            }

            // Insert the new stage
            m_stageOrder.insert(it, newLabel);
            m_stages.emplace(newLabel, std::make_unique<Stage>());
            return *this;
        }

        template <typename StageLabel, typename AfterStageLabel>
        Schedule& insertStageAfter()
        {
            if constexpr (std::is_same_v<StageLabel, CoreStage::StartUp> ||
                          std::is_same_v<AfterStageLabel, CoreStage::StartUp>)
            {
                WS_LOG_WARN("ECS", "Cannot insert with CoreStage::StartUp.");
                return *this;
            }
            else if constexpr (std::is_same_v<StageLabel, CoreStage::CleanUp> ||
                               std::is_same_v<AfterStageLabel,
                                              CoreStage::CleanUp>)
            {
                WS_LOG_WARN("ECS", "Cannot insert with CoreStage::CleanUp.");
                return *this;
            }

            std::type_index const newLabel(typeid(StageLabel));
            std::type_index const afterLabel(typeid(AfterStageLabel));

            // Check if new stage already exists
            if (m_stages.count(newLabel))
            {
                WS_LOG_WARN("ECS", "Stage {} already exists.", newLabel.name());
                return *this;
            }

            // Find the position to insert after
            auto it =
                std::find(m_stageOrder.begin(), m_stageOrder.end(), afterLabel);
            if (it == m_stageOrder.end())
            {
                WS_LOG_WARN("ECS", "Stage {} not found.", afterLabel.name());
                return *this;
            }

            // Insert the new stage after the found position
            m_stageOrder.insert(it + 1, newLabel);
            m_stages.emplace(newLabel, std::make_unique<Stage>());
            return *this;
        }

        bool hasStage(std::type_index const& label) const
        {
            return m_stages.count(label) > 0;
        }

        template <typename StageLabel> bool hasStage() const
        {
            std::type_index const label(typeid(StageLabel));
            return hasStage(label);
        }

        std::size_t getStageCount() const
        {
            return m_stages.size();
        }

        void initialize(Registry& registry) const
        {
            m_startUpStage->run(registry);
        }

        // run once
        // TODO: Support state machine
        void run(Registry& registry) const
        {
            for (StageLabelType const& label : m_stageOrder)
            {
                m_stages.at(label)->run(registry);
            }

            registry.dipatchEvents();
        }

        void shutdown(Registry& registry) const
        {
            m_cleanUpStage->run(registry);
        }

    private:
        std::string m_name;

        std::unique_ptr<Stage> m_startUpStage;
        std::unordered_map<StageLabelType, std::unique_ptr<Stage>> m_stages;
        std::unique_ptr<Stage> m_cleanUpStage;
        std::vector<StageLabelType> m_stageOrder;
    };
} // namespace worse::ecs