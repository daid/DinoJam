#pragma once

#include "pawn.h"

#include <unordered_set>


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
    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override;

    bool has_rider = false;

    Ability ability = Ability::Bite;
private:
    bool is_jumping = false;
    sp::P<Flamethrower> flamethrower;
};

class PlayerPawn : public Pawn
{
public:
    PlayerPawn(sp::P<sp::Node> parent);

    void stepOffDino();
    void onUpdate(float delta) override;
    void onFixedUpdate() override;
    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override;

    float oxygen = 1.0;
    sp::P<PlayerDino> dino;
    sp::P<sp::Node> oxygen_bar;
};

class PlayerInfo
{
public:
    int health = 6;
    int health_max = 6;

    sp::P<PlayerPawn> pawn;
    sp::P<PlayerDino> dino;
    sp::Vector2d dino_location{-100000, -100000};
    std::vector<PlayerDino::Ability> abilities;

    int coins = 0;
    int keys = 0;
    std::unordered_set<std::string> flags;

    PlayerDino::Ability nextAbility(PlayerDino::Ability);
    sp::Vector2d getPosition() { if (pawn->dino) return pawn->dino->getPosition2D(); return pawn->getPosition2D(); }
    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) { if (pawn->dino) return pawn->dino->onDamage(amount, target, source_position); return pawn->onDamage(amount, target, source_position); }
};
extern std::unique_ptr<PlayerInfo> pi;
