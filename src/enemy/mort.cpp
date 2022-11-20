#include "mort.h"
#include "../player.h"
#include "../emote.h"
#include "../mainScene.h"
#include "../world.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>
#include <sp2/random.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/collision/2d/circle.h>




Mort::Mort(sp::P<sp::Node> parent, std::string key)
: Pawn(parent, {1.2, 1.75}, DamageTarget::Enemy), key(key)
{
    setAnimation(sp::SpriteAnimation::load("sprites/dino/mort_male.txt"));
    animationPlay("Idle");
    render_data.order = 0;

    next_action_timer.start(1.5);
}

void Mort::onFixedUpdate()
{
    if (dead) { Pawn::onFixedUpdate(); return; }
    switch(state)
    {
    case State::Idle:
        if (next_action_timer.isExpired()) {
            goal_x = map.rect.position.x + sp::random(8, 22);
            next_action_timer.start(5);
            state = State::WalkToGoal;
        }
        break;
    case State::WalkToGoal:
        move_request.x = (getPosition2D().x < goal_x) ? 0.6 : -0.6;
        if (std::abs(getPosition2D().x - goal_x) < 1.5 || next_action_timer.isExpired()) {
            if (sp::random(0, 100) < 50)
                animationSetFlags(sp::SpriteAnimation::FlipFlag);
            else
                animationSetFlags(0);
            next_action_timer.start(0.3);
            state = State::DelayAtGoal;
        }
        break;
    case State::DelayAtGoal:
        if (next_action_timer.isExpired()) {
            move_request.x = 0.0;
            next_action_timer.start(1.5);
            state= State::Fire;
        }
        break;
    case State::Fire:
        if (!flamethrower) {
            if (animationGetFlags())
                flamethrower = new Flamethrower(this, sp::Vector2d(0.5, 0.1), 0, DamageTarget::Player);
            else
                flamethrower = new Flamethrower(this, sp::Vector2d(-0.5, 0.1), 180, DamageTarget::Player);
        }
        move_request = {0.0, 0.0};
        jump_request = false;
        special_anim = true;
        animationPlay("Fire");
        if (next_action_timer.isExpired()) {
            state = State::Idle;
            next_action_timer.start(0.7);
            flamethrower->stopSpawn();
            flamethrower->auto_destroy = true;
            flamethrower = nullptr;
        }
        break;
    }

    Pawn::onFixedUpdate();
}

void Mort::onCollision(sp::CollisionInfo& info)
{
    Pawn::onCollision(info);
    if (std::abs(info.normal.y) < 0.1 && info.other && info.other->isSolid()) {
        if ((move_request.x < 0 && info.normal.x < 0) || (move_request.x > 0 && info.normal.x > 0)) {
            jump_request = true;
            high_jump_request = false;
        }
    }
}

bool Mort::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    auto ret = Pawn::onDamage(amount, target, source_position);
    if (ret) {
        hp -= amount;
        if (hp == 0) {
            pi->abilities.push_back(PlayerDino::Ability::Fire);
            sp::P<Scene> scene = getScene();
            scene->msg("You kill mort and absorp his power!", [](){});

            state = State::Idle;
            if (flamethrower) {
                flamethrower->stopSpawn();
                flamethrower->auto_destroy = true;
                flamethrower = nullptr;
            }
            pi->flags.emplace(key);
            die();
        }
    }
    return ret;
}