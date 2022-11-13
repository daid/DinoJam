#include "pawn.h"
#include "world.h"

#include <sp2/collision/2d/box.h>
#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/spriteAnimation.h>
#include "main.h"

Pawn::Pawn(sp::P<sp::Node> parent, sp::Vector2d collision_size, DamageTarget damage_target)
: Damageable(parent), damage_target(damage_target)
{
    auto shape = sp::collision::Box2D(collision_size.x, collision_size.y);
    shape.setFilterCategory(1);
    shape.setMaskFilterCategory(1);
    shape.linear_damping = 0.8;
    shape.fixed_rotation = true;
    setCollisionShape(shape);
}

void Pawn::onFixedUpdate()
{
    invincible_timer.isExpired();

    auto posi = sp::Vector2i(getPosition2D()) - map.rect.position;
    posi.x = std::clamp(posi.x, 0, map.rect.size.x - 1);
    posi.y = std::clamp(posi.y, 0, map.rect.size.y - 1);
    auto new_water = map.tiles[posi.x + posi.y * map.rect.size.x].water;
    if (new_water != in_water) {
        in_water = new_water;
        if (new_water) {
            auto pe = new sp::ParticleEmitter(getParent(), "splash.particle");
            pe->setPosition(getPosition2D());
        }
    }

    auto velocity = getLinearVelocity2D();
    if (in_water && can_swim) {
        is_jumping = false;
    } else {
        if (is_jumping && velocity.y > 0 && controller.primary_action.get()) {
            velocity.y -= 0.5;
        } else {
            is_jumping = false;
            velocity.y -= 1;
        }
    }
    if (!invincible_timer.isRunning()) {
        velocity.x += (move_request.x * 10.0 - velocity.x) * 0.3;
        if (in_water && can_swim)
            velocity.y += (move_request.y * 7.0 - velocity.y) * 0.3;
        if (jump_request) {
            jump_request = false;
            if (on_floor || (in_water && can_swim)) {
                velocity += sp::Vector2d(0, jump_power);
                is_jumping = true;
                on_floor = false;
            }
        }
    }
    if (in_water) {
        velocity.x *= 0.9;
        if (velocity.y > 0)
            velocity.y *= 0.97;
        else
            velocity.y *= 0.9;
    }
    setLinearVelocity(velocity);

    if (on_floor)
        on_floor -= 1;
    if (move_request.x > 0)
        animationSetFlags(0);
    if (move_request.x < 0)
        animationSetFlags(sp::SpriteAnimation::FlipFlag);
    if (!special_anim || animationIsFinished()) {
        special_anim = false;
        if (can_swim && in_water) {
            if (move_request.x || move_request.y)
                animationPlay("SwimMove");
            else
                animationPlay("SwimIdle");
        } else if (!on_floor)
            animationPlay("Jump");
        else if (move_request.x)
            animationPlay("Walk");
        else
            animationPlay("Idle");
    }
}

void Pawn::onCollision(sp::CollisionInfo& info)
{
    if (!info.other || !info.other->isSolid()) return;
    if (info.normal.y < -0.7) {
        on_floor = 5;
    }
}

bool Pawn::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    if (target != DamageTarget::Any && target != damage_target)
        return false;
    if (invincible_timer.isRunning())
        return false;
    auto v = (getPosition2D() - source_position).normalized();
    setLinearVelocity(getLinearVelocity2D() + v * 5.0);
    special_anim = true;
    animationPlay("Hurt");
    invincible_timer.start(0.3);
    return true;
}