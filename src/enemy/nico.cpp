#include "nico.h"
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




Nico::Nico(sp::P<sp::Node> parent, std::string key)
: Pawn(parent, {1.2, 1.75}, DamageTarget::Enemy), key(key)
{
    setAnimation(sp::SpriteAnimation::load("sprites/dino/nico_male.txt"));
    animationPlay("Idle");
    render_data.order = 0;

    next_action_timer.start(1.5);
}

void Nico::onFixedUpdate()
{
    if (dead) { Pawn::onFixedUpdate(); return; }
    jump_request = false;
    high_jump_request = true;
    double x = getPosition2D().x - map.rect.position.x;
    switch(state)
    {
    case State::Idle:
        move_request.x = 0.0;
        if (next_action_timer.isExpired()) {
            if (x > 38) {
                if (std::abs(x - (pi->pawn->getPosition2D().x - map.rect.position.x)) < 10.0)
                    state = State::JumpToCenter;
            } else if (x < 19) {
                if (std::abs(x - (pi->pawn->getPosition2D().x - map.rect.position.x)) < 10.0)
                    state = State::JumpToCenter;
            } else if (x < pi->pawn->getPosition2D().x - map.rect.position.x) {
                state = State::JumpToLeft;
            } else {
                state = State::JumpToRight;
            }
            next_action_timer.start(0.5);
        }
        break;
    case State::JumpToCenter:
        move_request.x = (x < 28) ? 1.0 : -1.0;
        if (std::abs(x - 13) < 0.5) {
            jump_request = true;
        }
        if (std::abs(x - 43) < 0.5) {
            jump_request = true;
        }
        if (std::abs(x - 28) < 1.0) {
            state = State::Idle;
            next_action_timer.start(0.5);
        }
        break;
    case State::JumpToLeft:
        move_request.x = -1.0;
        if (std::abs(x - 27) < 0.5) {
            jump_request = true;
        }
        if (std::abs(x - 11) < 1.0) {
            state = State::Idle;
            next_action_timer.start(0.5);
        }
        break;
    case State::JumpToRight:
        move_request.x = 1.0;
        if (std::abs(x - 29) < 0.5) {
            jump_request = true;
        }
        if (std::abs(x - 45) < 1.0) {
            state = State::Idle;
            next_action_timer.start(0.5);
        }
        break;
    }

    if (is_jumping && getLinearVelocity2D().y < 1.5 && allow_dash) {
        if (animationGetFlags())
            setLinearVelocity(sp::Vector2d(20, 0));
        else
            setLinearVelocity(sp::Vector2d(-20, 0));
        dash_timer.start(0.3);
        animationPlay("Dash");
        allow_dash = false;
    }

    Pawn::onFixedUpdate();
}

bool Nico::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    auto ret = Pawn::onDamage(amount, target, source_position);
    if (ret) {
        hp -= amount;
        if (hp == 0) {
            pi->abilities.push_back(PlayerDino::Ability::Dash);
            sp::P<Scene> scene = getScene();
            scene->msg("You kill nico and absorp his power!", [](){});

            state = State::Idle;
            pi->flags.emplace(key);
            die();
        }
    }
    return ret;
}