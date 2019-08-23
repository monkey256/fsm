#include "Fsm.h"
#include <time.h>
#include <iostream>
#include <Windows.h>

//return range:[val1, val2]
inline static int randomInt(int val1, int val2)
{
    return val1 + rand() % (val2 - val1 + 1);
}

class Robot
{
public:
    enum state_type
    {
        state_idle = 1,         //空闲
        state_recover,          //回血
        state_find_monster,     //寻怪
        state_attack_monster,   //打怪
        state_dead,             //死亡
    };
    enum event_type
    {
        event_recover = 1,      //回血
        event_recover_finish,   //回血结束
        event_find_monster,     //寻怪
        event_monster_die,      //怪死亡
        event_die,              //死亡
    };

    Robot()
    {
        maxHp_ = 5;
        currentHp_ = maxHp_;
        monsterHp_ = 0;
        buildFsm();
    }

    ~Robot()
    {
        fsm_.clearWithRelease();
    }

    void buildFsm()
    {
        //创建状态机
        fsm::State* stateIdle = new fsm::State(state_idle);
        stateIdle->updateEvent = [this](int)
        {
            std::cout << "state idle" << std::endl;
            if (currentHp_ < maxHp_)
                return fsm_.trigger(event_recover);
            else
                return fsm_.trigger(event_find_monster);
        };

        fsm::State* stateRecover = new fsm::State(state_recover);
        stateRecover->updateEvent = [this](int)
        {
            std::cout << "state recover myHp: " << currentHp_ << std::endl;
            if (currentHp_ >= maxHp_)
                return fsm_.trigger(event_recover_finish);
            currentHp_++;
        };

        fsm::State* stateFindMonter = new fsm::State(state_find_monster);
        stateFindMonter->enterEvent = [this]()
        {
            monsterHp_ = randomInt(3, 8);
        };
        stateFindMonter->updateEvent = [this](int)
        {
            std::cout << "state find monster" << std::endl;
        };

        fsm::State* stateAttackMonster = new fsm::State(state_attack_monster);
        stateAttackMonster->updateEvent = [this](int)
        {
            std::cout << "state attack monster myHp: " << currentHp_ << " monsterHp: " << monsterHp_ << std::endl;
            monsterHp_--;
            if (monsterHp_ <= 0)
                return fsm_.trigger(event_monster_die);
            currentHp_--;
            if (currentHp_ <= 0)
                return fsm_.trigger(event_die);
        };

        fsm::State* stateDead = new fsm::State(state_dead);
        stateDead->exitEvent = [this]()
        {
            currentHp_ = maxHp_;
        };
        stateDead->updateEvent = [this](int)
        {
            std::cout << "state dead" << std::endl;
        };

        //状态机路径
        fsm_.initialize(stateIdle);
        fsm_.addState(stateRecover);
        fsm_.addState(stateFindMonter);
        fsm_.addState(stateAttackMonster);
        fsm_.addState(stateDead);
        
        //idle -> recover
        fsm_.addTransition(stateIdle, stateRecover, event_recover, nullptr);
        //recover -> idle
        fsm_.addTransition(stateRecover, stateIdle, event_recover_finish, nullptr);
        //idle -> find monster
        fsm_.addTransition(stateIdle, stateFindMonter, event_find_monster, nullptr);
        //find monster -> attack monster
        fsm_.addTimedTransition(stateFindMonter, stateAttackMonster, 2000, 5000, nullptr);
        //attack monster -> idle
        fsm_.addTransition(stateAttackMonster, stateIdle, event_monster_die, nullptr);
        //attack monster -> dead
        fsm_.addTransition(stateAttackMonster, stateDead, event_die, nullptr);
        //dead -> idle
        fsm_.addTimedTransition(stateDead, stateIdle, 3000, [this]()
        {
            currentHp_ = maxHp_;
        });
    }

    void run(int timeInMillionSeconds)
    {
        fsm_.runMachine(timeInMillionSeconds);
    }


protected:
    int         currentHp_;     //当前血量
    int         maxHp_;         //最大血量
    fsm::Fsm    fsm_;           //状态机
    int         monsterHp_;     //怪物血量
};

int main()
{
    srand((unsigned int)time(0));

    Robot robot;

    while (true)
    {
        robot.run(1000);
        Sleep(1000);
    }

    return 0;
}
