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
            m_prevState = m_currState;
            m_nextState = newState;
            WS_LOG_INFO("Page",
                        "Transfer to state: {}",
                        static_cast<int>(m_nextState));
        }

        void back()
        {
            if (m_prevState != State::Undefined)
            {
                m_nextState = m_prevState;
                WS_LOG_INFO("Page",
                            "Back to state: {}",
                            static_cast<int>(m_nextState));
            }
            else
            {
                WS_LOG_WARN("Page", "No previous state to go back to.");
            }
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
        State m_prevState = State::Undefined;
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
        static PageRouter<State>&
        registerStates(ecs::Commands commands, State startState = State::Begin)
        {
            PageRouter<State>& router =
                commands.emplaceResource<PageRouter<State>>();

            router.transfer(startState);

            activePage = [&router](ecs::Commands commands,
                                   ecs::Resource<GlobalContext>
                                       globalContext)
            {
                router.renderPage(commands, globalContext);
            };

            return router;
        }

        static void registerAlwaysRenderPage(Page page)
        {
            alwaysRenderPages.push_back(std::move(page));
        }

    private:
        inline static Page activePage = nullptr;
        inline static std::vector<Page> alwaysRenderPages;
    };

} // namespace worse