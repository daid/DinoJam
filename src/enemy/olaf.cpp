#include "olaf.h"
#include "../player.h"
#include "../emote.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/audio/sound.h>
#include <sp2/random.h>


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
                if (sp::random(0, 100) < 50)
                    goal_x = getPosition2D().x + sp::random(3, 5);
                else
                    goal_x = getPosition2D().x - sp::random(3, 5);
                next_action_timer.start(5);
                state = State::WalkToGoal;
                goal = Goal::None;
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

                if (std::abs(getPosition2D().y - pp.y) < 1 && std::abs(getPosition2D().x - pp.x) < 2)
                    pi->onDamage(1, DamageTarget::Player, getPosition2D());
                break;
            case Goal::Apple:
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
}
