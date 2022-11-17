#pragma once

#include <sp2/scene/node.h>
#include <sp2/timer.h>


enum class DamageTarget
{
    Any,
    Player,
    Enemy,
};

class Damageable : public sp::Node
{
public:
    Damageable(sp::P<sp::Node> parent) : sp::Node(parent) {}

    virtual bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) { return false; }
};

class Pawn : public Damageable
{
public:
    Pawn(sp::P<sp::Node> parent, sp::Vector2d collision_size, DamageTarget damage_target);

    void onFixedUpdate() override;
    void die();

    void onCollision(sp::CollisionInfo& info) override;
    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override;
protected:
    DamageTarget damage_target = DamageTarget::Any;
    double jump_power = 20.0;
    bool in_water = false;
    sp::Vector2d move_request;
    bool jump_request = false;
    bool high_jump_request = true;

    bool can_swim = false;

    bool dead = false;
    int on_floor = 0;
    bool allow_dash = false;
    bool is_jumping = false;
    bool special_anim = false;
    sp::Timer dash_timer;
    sp::Timer invincible_timer;
};
