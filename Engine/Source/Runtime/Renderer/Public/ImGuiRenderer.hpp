#pragma once
#include "Prefab.hpp"
#include "ECS/Commands.hpp"
#include "ECS/Resource.hpp"

#include <functional>
#include <type_traits>

namespace worse
{

    void defaultPage(int state);

    template <typename State>
    concept StateTraits = std::is_enum_v<State> && requires {
        State::Undefined;
        State::Begin;
        State::End;
    };

    template <StateTraits State> class PageRouter
    {
    public:
        using Page =
            std::function<void(ecs::Commands, ecs::Resource<GlobalContext>)>;

        void transfer(State newState)
        {
            m_nextState = newState;
        }

        void registerPage(State state, Page page)
        {
            m_pages[state] = std::move(page);
        }

        void renderPage(ecs::Commands commands,
                        ecs::Resource<GlobalContext> globalContext)
        {
            m_currState = m_nextState;

            if (auto it = m_pages.find(m_currState); it != m_pages.end())
            {
                it->second(commands, globalContext);
            }
            else
            {
                defaultPage(static_cast<int>(m_currState));
            }
        }

        State getCurrentState() const
        {
            return m_currState;
        }

    private:
        State m_currState = State::Undefined;
        State m_nextState = State::Begin;
        std::unordered_map<State, Page> m_pages;
    };

    class ImGuiRenderer
    {
    public:
        using Page =
            std::function<void(ecs::Commands, ecs::Resource<GlobalContext>)>;

        static void initialize();
        static void shutdown();
        static void tick(ecs::Commands commands,
                         ecs::Resource<GlobalContext> globalContext);

        template <StateTraits State>
        static void registerStates(ecs::Commands commands,
                                   State startState = State::Begin)
        {
            PageRouter<State>& rounter =
                commands.emplaceResource<PageRouter<State>>();

            rounter.transfer(startState);

            activePage = [&rounter](ecs::Commands commands,
                                    ecs::Resource<GlobalContext>
                                        globalContext)
            {
                rounter.renderPage(commands, globalContext);
            };
        }

    private:
        inline static Page activePage = nullptr;
    };

} // namespace worse