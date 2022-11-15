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
    if (next_action_timer.isExpired()) {
        switch(sp::irandom(0, 2)) {
        case 0:
            move_request.x = 0.3;
            next_action_timer.start(3);
            break;
        case 1:
            move_request.x = -0.3;
            next_action_timer.start(3);
            break;
        case 2:
            move_request.x = 0.0;
            next_action_timer.start(3);
            break;
        }
    }
    Pawn::onFixedUpdate();
}