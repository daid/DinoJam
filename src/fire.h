#pragma once

#include "pawn.h"
#include <sp2/scene/particleEmitter.h>


class FireEffect : public sp::ParticleEmitter
{
public:
    FireEffect(sp::P<sp::Node> parent, sp::Vector2d position);

    void onUpdate(float delta) override;

private:
    sp::Timer lifetime;
};


class TileFire : public sp::ParticleEmitter
{
public:
    TileFire(sp::P<sp::Node> parent, sp::Vector2d position);

    void onUpdate(float delta) override;

    static bool tryStartFireAt(sp::P<sp::Node> root, sp::Vector2d position);
private:
    sp::Timer lifetime;
    sp::Timer spreadstart;
};

class Flamethrower : public sp::ParticleEmitter
{
public:
    Flamethrower(sp::P<sp::Node> parent, sp::Vector2d position, double direction, DamageTarget damage_target);
    virtual ~Flamethrower();

    void onFixedUpdate() override;
private:
    sp::Timer fire_start_timer;
    bool has_noise = true;
    DamageTarget damage_target;
};

class Fireball : public sp::ParticleEmitter
{
public:
    Fireball(sp::P<sp::Node> parent, sp::Vector2d position, sp::Vector2d velocity);

    void onFixedUpdate() override;

    sp::Vector2d velocity;
    int lifetime = 100;
};