#pragma once

#include <vector>
#include <functional>

namespace fsm
{
    typedef std::function<void()>       Action;
    typedef std::function<void(int)>    UpdateAction;

    class State
    {
    public:
        State(int _name) : name(_name) { }

        virtual ~State() { }

        virtual void release() { delete this; }

        virtual void onEnter() { }

        virtual void onUpdate(int deltaTimeInMillionSeconds) { }

        virtual void onExit() { }

    public:
        int          name;
        Action       enterEvent;
        UpdateAction updateEvent;
        Action       exitEvent;
    };

    class Fsm
    {
    public:
        void    initialize(State* initialState);

        void    clearWithRelease();

        void    addState(State* state);

        void    addTransition(State* stateFrom, State* stateTo, int event, Action fnTransitionCallback);

        void    addTimedTransition(State* stateFrom, State* stateTo, int timeInMillionSeconds, Action fnTransitionCallback);

        void    addTimedTransition(State* stateFrom, State* stateTo, int minIntervalInMillionSeconds, int maxIntervalInMillionSeconds, Action fnTransitionCallback);

        void    trigger(int event);

        void    runMachine(int deltaTimeInMillionSeconds);

        State*  getCurrentState() { return currentState_; }

        int     getCurrentStateName() { return currentState_->name; }

    protected:
        struct Transition
        {
            State*      stateFrom;
            State*      stateTo;
            int         event;
            Action      onTransition;
        };

        struct TimedTransition
        {
            Transition  transition;
            int         leftTime;
            int         minInterval;
            int         maxInterval;
        };

        void    checkTimedTransitions(int deltaTimeInMillionSeconds);

        void    performTransition(Transition* ptr);

    protected:
        bool                            isInitialized_;
        std::vector<State*>             stateArray_;
        std::vector<Transition>         transitionArray_;
        std::vector<TimedTransition>    timedTransitionArray_;
        State*                          currentState_;
    };
}
