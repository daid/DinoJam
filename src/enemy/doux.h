#pragma once

#include "../pawn.h"


class Doux : public Pawn
{
public:
    Doux(sp::P<sp::Node> parent, std::string key);

    void onFixedUpdate() override;

    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override;
private:
    std::string key;
    int hp = 5;
};
