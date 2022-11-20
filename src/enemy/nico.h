#pragma once

#include "../pawn.h"


class Nico : public Pawn
{
public:
    Nico(sp::P<sp::Node> parent, std::string key);

    void onFixedUpdate() override;

    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override;
private:
    sp::Timer next_action_timer;
    enum class State {
        Idle,
        JumpToCenter,
        JumpToLeft,
        JumpToRight,
    };
    State state = State::Idle;
    std::string key;
    int hp = 5;
};
