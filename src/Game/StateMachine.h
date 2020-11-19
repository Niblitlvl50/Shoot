
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

    StateMachine()
    { }

    StateMachine(const std::unordered_map<StateId, State>& state_table)
    {
        SetStateTable(state_table);
    }

    void SetStateTable(const std::unordered_map<StateId, State>& state_table)
    {
        m_states = state_table;
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

    StateId m_active_state{};
    StateId m_wanted_state{};
    std::unordered_map<StateId, State> m_states;
};
