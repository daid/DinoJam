#include "olaf.h"

#include <sp2/graphics/spriteAnimation.h>
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
    switch(state)
    {
    case State::Idle:
        if (next_action_timer.isExpired()) {
            goal_x = getPosition2D().x + sp::random(-5, 5);
            next_action_timer.start(5);
            state = State::WalkToGoal;
        }
        break;
    case State::WalkToGoal:
        move_request.x = (getPosition2D().x < goal_x) ? 0.3 : -0.3;
        if (std::abs(getPosition2D().x - goal_x) < 1.5 || next_action_timer.isExpired()) {
            move_request.x = 0.0;
            next_action_timer.start(1);
            state = State::DelayAtGoal;
        }
        break;
    case State::DelayAtGoal:
        if (next_action_timer.isExpired()) {
            next_action_timer.start(1);
            switch(goal)
            {
            case Goal::None: state = State::Idle; break;
            case Goal::Player: state = State::Biting; break;
            case Goal::Apple: state = State::Biting; break;
            }
        }
        break;
    }

    Pawn::onFixedUpdate();
}