
#pragma once

#include <functional>
#include <unordered_map>


template <typename StateId, typename UpdateContext>
class StateMachine
{
public:

    struct State
    {
        std::function<void ()> enter_state = nullptr;
        std::function<void (const UpdateContext&)> update_state = nullptr;
    };

    template <typename T>
    static std::pair<StateId, State> MakeState(StateId id, void(T::*enter_func)(), void(T::*update_func)(const UpdateContext& context), T* this_ptr)
    {
        using namespace std::placeholders;
        return { id, { std::bind(enter_func, this_ptr), std::bind(update_func, this_ptr, _1) } };
    }

    template <typename T>
    static std::pair<StateId, State> MakeState(StateId id, void(T::*enter_func)(), T* this_ptr)
    {
        using namespace std::placeholders;
        return { id, { std::bind(enter_func, this_ptr), nullptr } };
    }

    using StateTable = std::unordered_map<StateId, State>;

    StateMachine()
    { }

    StateMachine(const StateTable& state_table)
    {
        SetStateTable(state_table);
    }

    void SetStateTable(const StateTable& state_table)
    {
        m_states = state_table;
    }

    void SetStateTableAndState(const StateTable& state_table, StateId initial_state)
    {
        m_states = state_table;
        m_wanted_state = initial_state;
    }

    void TransitionTo(StateId new_state)
    {
        m_wanted_state = new_state;
    }

    StateId ActiveState() const
    {
        return m_active_state;
    }

    void UpdateState(const UpdateContext& argument)
    {
        if(m_active_state != m_wanted_state)
        {
            State& state = m_states.at(m_wanted_state);
            if(state.enter_state)
                state.enter_state();

            m_active_state = m_wanted_state;
        }

        State& state = m_states.at(m_active_state);
        if(state.update_state)
            state.update_state(argument);
    }

private:

    StateId m_active_state{ -1 };
    StateId m_wanted_state{ -1 };
    StateTable m_states;
};
