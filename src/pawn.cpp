#include "pawn.h"
#include "world.h"

#include <sp2/collision/2d/box.h>
#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>
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
    dash_timer.isExpired();

    auto tileflags = map.tileAt(getPosition2D());
    auto new_water = tileflags.water;
    if (new_water != in_water) {
        in_water = new_water;
        if (new_water) {
            sp::audio::Sound::play("sfx/splash.wav");
            auto pe = new sp::ParticleEmitter(getParent(), "splash.particle");
            pe->setPosition(getPosition2D());
        }
    }

    auto velocity = getLinearVelocity2D();
    if (dead) {
        velocity.x *= 0.7;
        velocity.y -= 1;
    } else {
        if ((in_water && can_swim) || dash_timer.isRunning()) {
            is_jumping = false;
        } else {
            if (is_jumping && velocity.y > 0 && high_jump_request) {
                velocity.y -= 0.5;
            } else {
                is_jumping = false;
                velocity.y -= 1;
            }
        }
        if (!invincible_timer.isRunning() && !dash_timer.isRunning()) {
            if (on_floor || (in_water && can_swim))
                velocity.x += (move_request.x * 10.0 - velocity.x) * 0.3;
            else
                velocity.x += (move_request.x * 10.0 - velocity.x) * 0.1;
            if (in_water && can_swim) {
                if (move_request.y <= 0.0f || map.tileAt(getPosition2D() + sp::Vector2d(0, 0.3)).water)
                    velocity.y += (move_request.y * 7.0 - velocity.y) * 0.3;
            }
            if (jump_request) {
                jump_request = false;
                if (on_floor || (in_water && can_swim)) {
                    sp::audio::Sound::play("sfx/jump.wav");
                    velocity.y += jump_power;
                    is_jumping = true;
                    on_floor = false;
                }
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
    if (!dead) {
        if (move_request.x < 0)
            animationSetFlags(0);
        if (move_request.x > 0)
            animationSetFlags(sp::SpriteAnimation::FlipFlag);
        if ((!special_anim || animationIsFinished()) && !dash_timer.isRunning()) {
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
}

void Pawn::onCollision(sp::CollisionInfo& info)
{
    if (!info.other || !info.other->isSolid()) return;
    if (info.normal.y < -0.7) {
        on_floor = 5;
        allow_dash = true;
    } else {
        dash_timer.stop();
    }
}

bool Pawn::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    if (dead)
        return false;
    if (target != DamageTarget::Any && target != damage_target)
        return false;
    invincible_timer.isExpired();
    if (invincible_timer.isRunning())
        return false;
    auto v = (getPosition2D() - source_position).normalized();
    setLinearVelocity(getLinearVelocity2D() + v * 5.0);
    special_anim = true;
    animationPlay("Hurt");
    invincible_timer.start(0.3);
    return true;
}

void Pawn::die()
{
    animationPlay("Death");
    dead = true;
}
