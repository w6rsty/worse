#include "ECS/Entity.hpp"
#include "ECS/QueryView.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Commands.hpp"
#include "Log.hpp"

#include <chrono>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace worse;
using ecs::Commands;
using ecs::QueryView;

// 轻量级组件定义
struct Transform
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
};

struct Physics
{
    float vx = 0.0f, vy = 0.0f, vz = 0.0f;
    float mass = 1.0f;
};

struct Health
{
    int hp    = 100;
    int maxHp = 100;
};

struct Render
{
    int meshId     = 0;
    int materialId = 0;
    bool visible   = true;
};

// 空Tag组件（不会在lambda中使用）
struct Player
{
};
struct Enemy
{
};
struct NPC
{
};
struct Static
{
};

// 简单的性能计时器
class SimpleTimer
{
    std::chrono::high_resolution_clock::time_point start;

public:
    SimpleTimer() : start(std::chrono::high_resolution_clock::now())
    {
    }

    double elapsedMs() const
    {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

// 高精度纳秒计时器
class NanoTimer
{
    std::chrono::high_resolution_clock::time_point start;

public:
    NanoTimer() : start(std::chrono::high_resolution_clock::now())
    {
    }

    double elapsedNs() const
    {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::nano>(end - start).count();
    }

    void reset()
    {
        start = std::chrono::high_resolution_clock::now();
    }
};

// 创建测试实体
void createTestEntities(Commands& commands, int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> velDist(-5.0f, 5.0f);
    std::uniform_int_distribution<int> typeDist(0, 3);

    for (int i = 0; i < count; ++i)
    {
        Transform transform{posDist(gen), posDist(gen), posDist(gen), 0, 0, 0};
        Physics physics{velDist(gen), velDist(gen), velDist(gen), 1.0f};
        Health health{100, 100};
        Render render{i % 10, i % 5, true};

        int type = typeDist(gen);
        switch (type)
        {
        case 0:
            commands.spawn(transform, physics, health, render, Player{});
            break;
        case 1:
            commands.spawn(transform, physics, health, render, Enemy{});
            break;
        case 2:
            commands.spawn(transform, physics, health, render, NPC{});
            break;
        case 3:
            commands.spawn(transform, physics, render, Static{});
            break;
        }
    }
}

// 系统函数
void updateTransform(QueryView<Transform, Physics> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform, Physics const& physics)
        {
            transform.x += physics.vx * 0.016f;
            transform.y += physics.vy * 0.016f;
            transform.z += physics.vz * 0.016f;
        });
}

void updateHealth(QueryView<Health> view)
{
    view.each(
        [](ecs::Entity entity, Health& health)
        {
            if (health.hp < health.maxHp)
            {
                health.hp = std::min(health.maxHp, health.hp + 1);
            }
        });
}

void updateRender(QueryView<Transform, Render> view)
{
    view.each(
        [](ecs::Entity entity, Transform const& transform, Render& render)
        {
            // 简单的视锥剔除模拟
            float distance = transform.x * transform.x +
                             transform.y * transform.y +
                             transform.z * transform.z;
            render.visible = distance < 10000.0f;
        });
}

// 复杂查询测试
void complexQuery(QueryView<Transform, Physics, Health, Render> view)
{
    int count = 0;
    view.each(
        [&count](ecs::Entity entity,
                 Transform& transform,
                 Physics const& physics,
                 Health const& health,
                 Render const& render)
        {
            if (health.hp > 50 && render.visible)
            {
                transform.rotX += 0.01f;
                transform.rotY += 0.01f;
                count++;
            }
        });
}

// 7个不同的组件更新系统
void transformSystem1_Position(QueryView<Transform> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            transform.x += 0.1f;
            transform.y += 0.1f;
            transform.z += 0.1f;
        });
}

void transformSystem2_Rotation(QueryView<Transform> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            transform.rotX += 0.01f;
            transform.rotY += 0.01f;
            transform.rotZ += 0.01f;
        });
}

void transformSystem3_Scale(QueryView<Transform> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            // 模拟缩放更新 - 使用x作为scale
            transform.x *= 1.001f;
            transform.y *= 1.001f;
            transform.z *= 1.001f;
        });
}

void transformSystem4_Normalize(QueryView<Transform> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            // 模拟向量归一化
            float length = std::sqrt(transform.x * transform.x +
                                     transform.y * transform.y +
                                     transform.z * transform.z);
            if (length > 0.001f)
            {
                transform.x /= length;
                transform.y /= length;
                transform.z /= length;
            }
        });
}

void transformSystem5_Clamp(QueryView<Transform> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            // 模拟边界限制
            transform.x = std::clamp(transform.x, -100.0f, 100.0f);
            transform.y = std::clamp(transform.y, -100.0f, 100.0f);
            transform.z = std::clamp(transform.z, -100.0f, 100.0f);
        });
}

void transformSystem6_Interpolation(QueryView<Transform> view)
{
    static float target = 50.0f;
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            // 模拟插值更新
            const float lerpFactor = 0.1f;
            transform.x = transform.x + (target - transform.x) * lerpFactor;
            transform.y = transform.y + (target - transform.y) * lerpFactor;
            transform.z = transform.z + (target - transform.z) * lerpFactor;
        });
}

void transformSystem7_Complex(QueryView<Transform> view)
{
    view.each(
        [](ecs::Entity entity, Transform& transform)
        {
            // 模拟复杂的数学运算
            float temp =
                std::sin(transform.x * 0.01f) * std::cos(transform.y * 0.01f);
            transform.rotX = temp * 0.1f;
            transform.rotY = std::atan2(transform.y, transform.x) * 0.1f;
            transform.rotZ = std::fmod(transform.z * 0.1f, 6.28318f); // 2*PI
        });
}

// 主基准测试函数
void runProfessionalBenchmark()
{
    WS_LOG_INFO("PERF", "=== ECS专业性能测试 ===");

    // 测试不同规模的实体数量
    std::vector<int> entityCounts = {1000, 5000, 10000, 20000, 5'0000, 10'0000};

    for (int entityCount : entityCounts)
    {
        WS_LOG_INFO("PERF", "测试实体数量: {}", entityCount);

        ecs::Registry registry;
        Commands commands(registry);

        // 1. 实体创建性能测试
        SimpleTimer createTimer;
        createTestEntities(commands, entityCount);
        double createTime = createTimer.elapsedMs();

        WS_LOG_INFO("PERF",
                    "  创建时间: {:.2f}ms ({:.0f} 实体/ms)",
                    createTime,
                    entityCount / createTime);

        // 2. 单组件查询性能测试
        SimpleTimer singleQueryTimer;
        for (int i = 0; i < 1000; ++i)
        {
            auto view = registry.query<Transform>();
            view.each(
                [](ecs::Entity entity, Transform& transform)
                {
                    transform.rotZ += 0.001f;
                });
        }
        double singleQueryTime = singleQueryTimer.elapsedMs();

        WS_LOG_INFO("PERF",
                    "  单组件查询(1000次): {:.2f}ms (平均 {:.3f}ms/次)",
                    singleQueryTime,
                    singleQueryTime / 1000.0);

        // 3. 多组件查询性能测试
        SimpleTimer multiQueryTimer;
        for (int i = 0; i < 1000; ++i)
        {
            auto view = registry.query<Transform, Physics>();
            updateTransform(view);
        }
        double multiQueryTime = multiQueryTimer.elapsedMs();

        WS_LOG_INFO("PERF",
                    "  双组件查询(1000次): {:.2f}ms (平均 {:.3f}ms/次)",
                    multiQueryTime,
                    multiQueryTime / 1000.0);

        // 4. 复杂查询性能测试
        SimpleTimer complexQueryTimer;
        for (int i = 0; i < 1000; ++i)
        {
            auto view = registry.query<Transform, Physics, Health, Render>();
            complexQuery(view);
        }
        double complexQueryTime = complexQueryTimer.elapsedMs();

        WS_LOG_INFO("PERF",
                    "  四组件查询(1000次): {:.2f}ms (平均 {:.3f}ms/次)",
                    complexQueryTime,
                    complexQueryTime / 1000.0);

        // 5. 混合系统性能测试
        SimpleTimer mixedSystemTimer;
        for (int i = 0; i < 1000; ++i)
        {
            updateTransform(registry.query<Transform, Physics>());
            updateHealth(registry.query<Health>());
            updateRender(registry.query<Transform, Render>());
        }
        double mixedSystemTime = mixedSystemTimer.elapsedMs();

        WS_LOG_INFO("PERF",
                    "  混合系统(1000次): {:.2f}ms (平均 {:.3f}ms/次)",
                    mixedSystemTime,
                    mixedSystemTime / 1000.0);

        // 6. 实体销毁性能测试
        std::vector<ecs::Entity> toDestroy;
        auto destroyView = registry.query<Health>();
        destroyView.each(
            [&toDestroy](ecs::Entity entity, Health const& health)
            {
                if (health.hp < 75) // 销毁一部分实体
                {
                    toDestroy.push_back(entity);
                }
            });

        SimpleTimer destroyTimer;
        for (auto entity : toDestroy)
        {
            commands.destroy(entity);
        }
        double destroyTime = destroyTimer.elapsedMs();

        WS_LOG_INFO("PERF",
                    "  销毁 {} 实体: {:.2f}ms ({:.0f} 实体/ms)",
                    toDestroy.size(),
                    destroyTime,
                    toDestroy.empty() ? 0.0 : toDestroy.size() / destroyTime);

        WS_LOG_INFO("PERF", "");
    }

    // 缓存友好性测试
    WS_LOG_INFO("PERF", "=== 缓存友好性测试 ===");

    ecs::Registry registry;
    Commands commands(registry);
    createTestEntities(commands, 10000);

    // 连续内存访问 vs 随机访问对比
    auto view = registry.query<Transform>();

    // 连续访问
    SimpleTimer sequentialTimer;
    for (int iter = 0; iter < 1000; ++iter)
    {
        view.each(
            [](ecs::Entity entity, Transform& transform)
            {
                transform.x += 1.0f;
            });
    }
    double sequentialTime = sequentialTimer.elapsedMs();

    WS_LOG_INFO("PERF", "连续访问(1000次): {:.2f}ms", sequentialTime);

    // 模拟Cache Miss场景
    SimpleTimer cacheStressTimer;
    for (int iter = 0; iter < 100; ++iter)
    {
        // 同时访问多个不相关的组件类型
        auto transformView = registry.query<Transform>();
        auto physicsView   = registry.query<Physics>();
        auto healthView    = registry.query<Health>();
        auto renderView    = registry.query<Render>();

        transformView.each(
            [](ecs::Entity entity, Transform& t)
            {
                t.x += 0.1f;
            });
        physicsView.each(
            [](ecs::Entity entity, Physics& p)
            {
                p.vx += 0.1f;
            });
        healthView.each(
            [](ecs::Entity entity, Health& h)
            {
                h.hp = std::max(0, h.hp - 1);
            });
        renderView.each(
            [](ecs::Entity entity, Render& r)
            {
                r.visible = !r.visible;
            });
    }
    double cacheStressTime = cacheStressTimer.elapsedMs();

    WS_LOG_INFO("PERF", "缓存压力测试(100次): {:.2f}ms", cacheStressTime);
    WS_LOG_INFO("PERF",
                "缓存友好比率: {:.2f}:1",
                cacheStressTime / (sequentialTime / 10.0));
}

// 组件更新性能测试
void runComponentUpdateBenchmark()
{
    WS_LOG_INFO("PERF", "=== 组件更新性能测试 ===");

    std::vector<int> entityCounts = {1000, 5000, 10000, 20000, 50000};
    const int iterations = 1000; // 每个系统运行1000次来获得更准确的平均值

    for (int entityCount : entityCounts)
    {
        WS_LOG_INFO("PERF", "测试实体数量: {}", entityCount);

        ecs::Registry registry;
        Commands commands(registry);

        // 创建测试实体（只包含Transform组件）
        for (int i = 0; i < entityCount; ++i)
        {
            Transform transform{static_cast<float>(i % 100),
                                static_cast<float>(i % 100),
                                static_cast<float>(i % 100),
                                0.0f,
                                0.0f,
                                0.0f};
            commands.spawn(transform);
        }

        auto view = registry.query<Transform>();

        // 定义系统函数指针数组
        void (*systems[])(QueryView<Transform>) = {
            transformSystem1_Position,
            transformSystem2_Rotation,
            transformSystem3_Scale,
            transformSystem4_Normalize,
            transformSystem5_Clamp,
            transformSystem6_Interpolation,
            transformSystem7_Complex};

        const char* systemNames[] = {"位置更新系统",
                                     "旋转更新系统",
                                     "缩放更新系统",
                                     "归一化系统",
                                     "边界限制系统",
                                     "插值更新系统",
                                     "复杂运算系统"};

        // 测试每个系统
        for (int systemIndex = 0; systemIndex < 7; ++systemIndex)
        {
            std::vector<double> timings;
            timings.reserve(iterations);

            // 预热
            for (int i = 0; i < 10; ++i)
            {
                systems[systemIndex](view);
            }

            // 实际测试
            for (int i = 0; i < iterations; ++i)
            {
                NanoTimer timer;
                systems[systemIndex](view);
                double elapsed = timer.elapsedNs();
                timings.push_back(elapsed);
            }

            // 计算统计数据
            std::sort(timings.begin(), timings.end());
            double sum = 0.0;
            for (double time : timings)
            {
                sum += time;
            }

            double average = sum / timings.size();
            double median  = timings.size() % 2 == 0
                                 ? (timings[timings.size() / 2 - 1] +
                                   timings[timings.size() / 2]) /
                                      2.0
                                 : timings[timings.size() / 2];

            WS_LOG_INFO("PERF",
                        "  {}: 平均 {:.0f}ns, 中位数 {:.0f}ns, 最小 {:.0f}ns, "
                        "最大 {:.0f}ns",
                        systemNames[systemIndex],
                        average,
                        median,
                        timings.front(),
                        timings.back());

            // 计算每实体平均性能
            double nsPerEntity = average / entityCount;
            WS_LOG_INFO("PERF",
                        "    每实体平均: {:.2f}ns/实体, 吞吐量: {:.0f}M实体/秒",
                        nsPerEntity,
                        1000.0 / nsPerEntity);
        }

        WS_LOG_INFO("PERF", "");
    }

    // 系统对比测试 - 使用固定数量的实体
    WS_LOG_INFO("PERF", "=== 系统性能对比 (10000实体) ===");

    ecs::Registry registry;
    Commands commands(registry);

    for (int i = 0; i < 10000; ++i)
    {
        Transform transform{static_cast<float>(i % 100),
                            static_cast<float>(i % 100),
                            static_cast<float>(i % 100),
                            0.0f,
                            0.0f,
                            0.0f};
        commands.spawn(transform);
    }

    auto view = registry.query<Transform>();

    void (*systems[])(QueryView<Transform>) = {transformSystem1_Position,
                                               transformSystem2_Rotation,
                                               transformSystem3_Scale,
                                               transformSystem4_Normalize,
                                               transformSystem5_Clamp,
                                               transformSystem6_Interpolation,
                                               transformSystem7_Complex};

    const char* systemNames[] = {"位置更新",
                                 "旋转更新",
                                 "缩放更新",
                                 "归一化",
                                 "边界限制",
                                 "插值更新",
                                 "复杂运算"};

    struct SystemPerformance
    {
        int index;
        double averageNs;
        const char* name;
    };

    std::vector<SystemPerformance> performances;

    for (int systemIndex = 0; systemIndex < 7; ++systemIndex)
    {
        std::vector<double> timings;
        timings.reserve(iterations);

        // 预热
        for (int i = 0; i < 10; ++i)
        {
            systems[systemIndex](view);
        }

        // 测试
        for (int i = 0; i < iterations; ++i)
        {
            NanoTimer timer;
            systems[systemIndex](view);
            timings.push_back(timer.elapsedNs());
        }

        double sum = 0.0;
        for (double time : timings)
        {
            sum += time;
        }
        double average = sum / timings.size();

        performances.push_back(
            {systemIndex, average, systemNames[systemIndex]});
    }

    // 按性能排序（从快到慢）
    std::sort(performances.begin(),
              performances.end(),
              [](const SystemPerformance& a, const SystemPerformance& b)
              {
                  return a.averageNs < b.averageNs;
              });

    WS_LOG_INFO("PERF", "系统性能排名（从快到慢）:");
    for (size_t i = 0; i < performances.size(); ++i)
    {
        const auto& perf   = performances[i];
        double nsPerEntity = perf.averageNs / 10000.0;
        double throughput  = 1000.0 / nsPerEntity; // M entities/sec

        WS_LOG_INFO("PERF",
                    "  {}. {}: {:.0f}ns ({:.2f}ns/实体, {:.0f}M实体/秒)",
                    i + 1,
                    perf.name,
                    perf.averageNs,
                    nsPerEntity,
                    throughput);
    }

    // 性能差异分析
    double fastest = performances[0].averageNs;
    double slowest = performances.back().averageNs;
    WS_LOG_INFO("PERF", "最快与最慢系统性能差异: {:.1f}倍", slowest / fastest);
}

int main()
{
    Logger::initialize();

    runProfessionalBenchmark();

    WS_LOG_INFO("PERF", "");
    WS_LOG_INFO("PERF", "========================================");
    WS_LOG_INFO("PERF", "");

    runComponentUpdateBenchmark();

    Logger::shutdown();
    return 0;
}
