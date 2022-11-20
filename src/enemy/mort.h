#pragma once

#include "../pawn.h"
#include "../fire.h"


class Mort : public Pawn
{
public:
    Mort(sp::P<sp::Node> parent, std::string key);

    void onFixedUpdate() override;

    void onCollision(sp::CollisionInfo& info) override;
    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override;
private:
    sp::Timer next_action_timer;
    enum class State {
        Idle,
        WalkToGoal,
        DelayAtGoal,
        Fire,
    };
    State state = State::Idle;
    double goal_x = 0.0;
    std::string key;
    sp::P<Flamethrower> flamethrower;
    int hp = 5;
};