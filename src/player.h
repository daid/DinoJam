#pragma once

#include "pawn.h"


class Flamethrower;
class PlayerDino : public Pawn
{
public:
    enum class Ability {
        Bite,
        Swimming,
        Dash,
        Fire,
    };

    PlayerDino(sp::P<sp::Node> parent);

    void onFixedUpdate() override;

    bool has_rider = false;
private:
    Ability ability = Ability::Bite;
    bool is_jumping = false;
    sp::P<Flamethrower> flamethrower;
};

class PlayerPawn : public Pawn
{
public:
    PlayerPawn(sp::P<sp::Node> parent);

    void onUpdate(float delta) override;
    void onFixedUpdate() override;

    sp::P<PlayerDino> dino;
};

class PlayerInfo
{
public:
    sp::P<PlayerPawn> pawn;
    sp::P<PlayerDino> dino;
    sp::Vector2d dino_location;
    std::vector<PlayerDino::Ability> abilities;

    PlayerDino::Ability nextAbility(PlayerDino::Ability);
};
extern std::unique_ptr<PlayerInfo> pi;
