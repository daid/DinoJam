#pragma once

#include "../pawn.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/collision/2d/box.h>

class Bat : public Damageable
{
public:
    Bat(sp::P<sp::Node> parent)
    : Damageable(parent)
    {
        setAnimation(sp::SpriteAnimation::load("sprites/bat.txt"));
        animationPlay("Idle");
        auto shape = sp::collision::Box2D(1.0, 1.0);
        shape.type = sp::collision::Shape::Type::Kinematic;
        setCollisionShape(shape);
    }

    void onCollision(sp::CollisionInfo& info) override
    {
        sp::P<Damageable> obj = info.other;
        if (obj)
            obj->onDamage(1, DamageTarget::Player, getPosition2D());
    }

    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override
    {
        if (target == DamageTarget::Player) return false;
        delete this;
        return true;
    }
};
