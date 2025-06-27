#include "ECS/Entity.hpp"
#include "ECS/EventBus.hpp"
#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"
#include "ECS/Schedule.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Commands.hpp"
#include "Log.hpp"

using namespace worse;
using ecs::Commands;
using ecs::QueryView;
using ecs::Resource;

struct PlayerTag
{
};

struct HP
{
    int value = 0;
};

struct MP
{
    int value = 0;
};

struct EntitySpawnEvent
{
    ecs::Entity entity;
};

struct Counter
{
    std::size_t value = 0;
};

void startup(Commands commands)
{
    WS_LOG_INFO("ECS", "Hi");
    ecs::Entity e0 = commands.spawn(PlayerTag{}, HP{100}, MP{100});
    ecs::Entity e1 = commands.spawn(HP{100}, MP{120});

    commands.emplaceResource<Counter>(0); // 直接传递参数，避免临时对象
}

void update(Commands commands, QueryView<HP> view, Resource<Counter> counter)
{
    view.each(
        [](ecs::Entity entity, HP const& hp)
        {
            WS_LOG_INFO("ECS", "Entity {}, HP: {}", entity.value, hp.value);
        });

    counter->value += 1;

    ecs::Entity e = commands.spawn(PlayerTag{}, HP{200}, MP{200});
    commands.emitEvent(EntitySpawnEvent{e});
}

void postupdate(Commands commands, QueryView<HP> view0, QueryView<MP> view1,
                std::shared_ptr<ecs::EventReader<EntitySpawnEvent>> spawnEvent,
                Resource<Counter> counter)
{
    view0.each(
        [](ecs::Entity entity, HP& hp)
        {
            hp.value += 10;
        });

    WS_LOG_INFO("ECS", "Counter value: {}", counter->value);

    for (auto const& e : spawnEvent->read())
    {
        WS_LOG_INFO("ECS", "Spawned {}", e.data.entity.value);
    }

    view1.each(
        [&](ecs::Entity entity, MP const& _mp)
        {
            commands.destroy(entity);
        });
}

int main()
{
    Logger::initialize();

    ecs::Registry reg;
    ecs::Schedule schedule;

    schedule.addSystem<ecs::CoreStage::StartUp, startup>();
    schedule.addSystem<ecs::CoreStage::Update, update>();
    schedule.addSystem<ecs::CoreStage::PostUpdate, postupdate>();
    schedule.initialize(reg);

    for (std::size_t i = 0; i < 3; ++i)
    {
        WS_LOG_INFO("ECS", "=================================================");
        schedule.run(reg);
    }

    Logger::shutdown();
}