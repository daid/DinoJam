#pragma once

#include "../pawn.h"


class Apple : public sp::Node
{
public:
    Apple(sp::P<sp::Node> parent);

    void onCollision(sp::CollisionInfo& info) override;
    void onFixedUpdate() override;

    bool down = false;
};

class Olaf : public Pawn
{
public:
    Olaf(sp::P<sp::Node> parent);

    void onFixedUpdate() override;

    void onCollision(sp::CollisionInfo& info) override;
private:
    sp::Timer next_action_timer;
    enum class State {
        Idle,
        WalkToGoal,
        DelayAtGoal,
        Biting,
    };
    enum class Goal {
        None,
        Player,
        Apple,
    };
    State state = State::Idle;
    Goal goal = Goal::None;
    double goal_x = 0.0;
    int apple_eaten = 0;
};