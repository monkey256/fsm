#include "Fsm.h"
#include <time.h>

namespace fsm
{
    void Fsm::initialize(State* initialState)
    {
        stateArray_.reserve(32);
        transitionArray_.reserve(64);
        timedTransitionArray_.reserve(32);

        isInitialized_ = false;
        stateArray_ = { initialState };
        transitionArray_.clear();
        timedTransitionArray_.clear();
        currentState_ = initialState;
    }

    void Fsm::clearWithRelease()
    {
        isInitialized_ = false;
        for (State* state : stateArray_)
            state->release();
        stateArray_.clear();
        transitionArray_.clear();
        timedTransitionArray_.clear();
        currentState_ = nullptr;
    }

    void Fsm::addState(State* state)
    {
        stateArray_.push_back(state);
    }

    void Fsm::addTransition(State* stateFrom, State* stateTo, int event, Action fnTransitionCallback)
    {
        transitionArray_.push_back(Transition());
        Transition& transition = transitionArray_.back();
        transition.stateFrom = stateFrom;
        transition.stateTo = stateTo;
        transition.event = event;
        transition.onTransition = fnTransitionCallback;
    }

    void Fsm::addTimedTransition(State* stateFrom, State* stateTo, int timeInMillionSeconds, Action fnTransitionCallback)
    {
        timedTransitionArray_.push_back(TimedTransition());
        TimedTransition& timedTransition = timedTransitionArray_.back();
        timedTransition.transition.stateFrom = stateFrom;
        timedTransition.transition.stateTo = stateTo;
        timedTransition.transition.event = 0;
        timedTransition.transition.onTransition = fnTransitionCallback;
        timedTransition.minInterval = timeInMillionSeconds;
        timedTransition.maxInterval = timeInMillionSeconds;
        timedTransition.leftTime = timeInMillionSeconds;
    }

    void Fsm::addTimedTransition(State* stateFrom, State* stateTo, int minIntervalInMillionSeconds, int maxIntervalInMillionSeconds, Action fnTransitionCallback)
    {
        static bool s_srandInit = false;
        if (!s_srandInit)
        {
            s_srandInit = true;
            srand((unsigned int)time(0));
        }

        timedTransitionArray_.push_back(TimedTransition());
        TimedTransition& timedTransition = timedTransitionArray_.back();
        timedTransition.transition.stateFrom = stateFrom;
        timedTransition.transition.stateTo = stateTo;
        timedTransition.transition.event = 0;
        timedTransition.transition.onTransition = fnTransitionCallback;
        timedTransition.minInterval = minIntervalInMillionSeconds;
        timedTransition.maxInterval = maxIntervalInMillionSeconds;
        timedTransition.leftTime = minIntervalInMillionSeconds + rand() % (maxIntervalInMillionSeconds - minIntervalInMillionSeconds + 1);
    }

    void Fsm::trigger(int event)
    {
        if (isInitialized_)
        {
            for (Transition& t : transitionArray_)
            {
                if (t.event == event && (!t.stateFrom || t.stateFrom == currentState_))
                    return performTransition(&t);
            }
        }
    }

    void Fsm::runMachine(int deltaTimeInMillionSeconds)
    {
        if (!isInitialized_)
        {
            isInitialized_ = true;
            currentState_->onEnter();
            if (currentState_->enterEvent)
                currentState_->enterEvent();
        }

        currentState_->onUpdate(deltaTimeInMillionSeconds);
        if (currentState_->updateEvent)
            currentState_->updateEvent(deltaTimeInMillionSeconds);

        checkTimedTransitions(deltaTimeInMillionSeconds);
    }

    void Fsm::checkTimedTransitions(int deltaTimeInMillionSeconds)
    {
        for (TimedTransition& t : timedTransitionArray_)
        {
            if (!t.transition.stateFrom || t.transition.stateFrom == currentState_)
            {
                if (t.leftTime <= 0)
                {
                    return performTransition(&t.transition);
                }
                else
                {
                    t.leftTime -= deltaTimeInMillionSeconds;
                }
            }
        }
    }

    void Fsm::performTransition(Transition* ptr)
    {
        currentState_->onExit();
        if (currentState_->exitEvent)
            currentState_->exitEvent();

        if (ptr->onTransition)
            ptr->onTransition();

        State* state = ptr->stateTo;

        state->onEnter();
        if (state->enterEvent)
            state->enterEvent();

        if (currentState_)
            currentState_ = state;

        for (TimedTransition& t : timedTransitionArray_)
        {
            if (!t.transition.stateFrom || t.transition.stateFrom == currentState_)
            {
                if (t.minInterval == t.maxInterval)
                    t.leftTime = t.minInterval;
                else
                    t.leftTime = t.minInterval + rand() % (t.maxInterval - t.minInterval + 1);
            }
        }
    }
}
