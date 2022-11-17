#include "olaf.h"
#include "../player.h"
#include "../emote.h"
#include "../mainScene.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>
#include <sp2/random.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/collision/2d/circle.h>



Apple::Apple(sp::P<sp::Node> parent)
: sp::Node(parent)
{
    render_data.type = sp::RenderData::Type::Normal;
    render_data.order = 99;
    render_data.texture = sp::texture_manager.get("sprites/apple.png");
    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = sp::MeshData::createQuad({1, 1});

    sp::collision::Circle2D shape(0.3f);
    shape.type = sp::collision::Shape::Type::Sensor;
    setCollisionShape(shape);
}

void Apple::onCollision(sp::CollisionInfo& info)
{
    if (down) return;
    if (info.other == pi->pawn) {
        down = true;

        sp::collision::Circle2D shape(0.3f);
        shape.setFilterCategory(1);
        shape.setMaskFilterCategory(1);
        shape.angular_damping = 0.1;
        shape.linear_damping = 0.4;
        setCollisionShape(shape);
        setAngularVelocity(sp::random(-100, 100));
    }
}

void Apple::onFixedUpdate()
{
    if (down)
        setLinearVelocity(getLinearVelocity2D() + sp::Vector2d(0, -1));
}

Olaf::Olaf(sp::P<sp::Node> parent)
: Pawn(parent, {1.2, 1.75}, DamageTarget::Enemy)
{
    setAnimation(sp::SpriteAnimation::load("sprites/dino/olaf_male.txt"));
    animationPlay("Idle");
    render_data.order = 0;

    next_action_timer.start(0.5);
}

void Olaf::onFixedUpdate()
{
    /*
    Olaf AI:
        Check if we can walk towards the player, if we can:
            Store his position, walk to that position, wait a bit, bite.
        Check if we can walk towards an apple, if we can:
            Store apple position, walk to that position, wait a bit, bite.
        else:
            Take a random spot at least X distance away, walk to it, wait a bit.
    */
    auto pp = pi->getPosition();
    switch(state)
    {
    case State::Idle:
        if (next_action_timer.isExpired()) {
            if (std::abs(getPosition2D().y - pp.y) < 3 && std::abs(getPosition2D().x - pp.x) < 6)
            {
                goal_x = pp.x;
                next_action_timer.start(2);
                state = State::WalkToGoal;
                goal = Goal::Player;
                new Emote(this, 29);
            } else {
                goal = Goal::None;
                for(auto n : getParent()->getChildren()) {
                    sp::P<Apple> apple = n;
                    if (apple && apple->down) {
                        if (std::abs(getPosition2D().y - apple->getPosition2D().y) < 3 && std::abs(getPosition2D().x - apple->getPosition2D().x) < 8) {
                            goal = Goal::Apple;
                            goal_x = apple->getPosition2D().x;
                        }
                    }
                }
                if (goal == Goal::None) {
                    if (sp::random(0, 100) < 50)
                        goal_x = getPosition2D().x + sp::random(4, 8);
                    else
                        goal_x = getPosition2D().x - sp::random(4, 8);
                }
                next_action_timer.start(5);
                state = State::WalkToGoal;
                if (goal == Goal::Apple)
                    new Emote(this, 22);
                else
                    new Emote(this, 0);
            }
        }
        break;
    case State::WalkToGoal:
        move_request.x = (getPosition2D().x < goal_x) ? 0.3 : -0.3;
        if (std::abs(getPosition2D().x - goal_x) < 1.5 || next_action_timer.isExpired()) {
            move_request.x = 0.0;
            next_action_timer.start(0.3);
            state = State::DelayAtGoal;
        }
        break;
    case State::DelayAtGoal:
        if (next_action_timer.isExpired()) {
            next_action_timer.start(0.5);
            switch(goal)
            {
            case Goal::None: state = State::Idle; break;
            case Goal::Player:
                special_anim = true;
                animationPlay("Bite");
                sp::audio::Sound::play("sfx/chomp.wav");
                state = State::Biting;

                if (std::abs(getPosition2D().y - pp.y) < 1.5 && std::abs(getPosition2D().x - pp.x) < 2)
                    pi->onDamage(1, DamageTarget::Player, getPosition2D());
                break;
            case Goal::Apple:
                special_anim = true;
                animationPlay("Bite");
                sp::audio::Sound::play("sfx/chomp.wav");
                state = State::Biting;

                for(auto n : getParent()->getChildren()) {
                    sp::P<Apple> apple = n;
                    if (apple && apple->down) {
                        if (std::abs(getPosition2D().y - apple->getPosition2D().y) < 1.5 && std::abs(getPosition2D().x - apple->getPosition2D().x) < 2) {
                            apple.destroy();
                            new Emote(this, 26);
                            apple_eaten += 1;
                            break;
                        }
                    }
                }

                break;
            }
        }
        break;
    case State::Biting:
        if (!special_anim) {
            next_action_timer.start(0.3);
            state = State::Idle;
        }
        break;
    }

    Pawn::onFixedUpdate();

    if (apple_eaten == 3) {
        pi->abilities.push_back(PlayerDino::Ability::Bite);
        pi->abilities.push_back(PlayerDino::Ability::Swimming);
        pi->abilities.push_back(PlayerDino::Ability::Fire);
        pi->abilities.push_back(PlayerDino::Ability::Dash);
        pi->dino = new PlayerDino(getParent());
        pi->dino->setPosition(getPosition2D());
        sp::P<Scene> scene = getScene();
        scene->msg("Olaf loves you now,\nyou can ride him!", [](){});
        delete this;
    }
}

void Olaf::onCollision(sp::CollisionInfo& info)
{
    Pawn::onCollision(info);
    if (std::abs(info.normal.y) < 0.1 && info.other && info.other->isSolid()) {
        if ((move_request.x < 0 && info.normal.x < 0) || (move_request.x > 0 && info.normal.x > 0)) {
            jump_request = true;
            high_jump_request = false;
        }
    }
}
