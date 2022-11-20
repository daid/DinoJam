#include "doux.h"
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




Doux::Doux(sp::P<sp::Node> parent, std::string key)
: Pawn(parent, {1.2, 1.75}, DamageTarget::Enemy), key(key)
{
    setAnimation(sp::SpriteAnimation::load("sprites/dino/doux_male.txt"));
    animationPlay("Idle");
    render_data.order = 0;
    can_swim = true;

    move_request.y = 1.0;
}

void Doux::onFixedUpdate()
{
    if (dead) { Pawn::onFixedUpdate(); return; }
    
    if (getPosition2D().y - map.rect.position.y < 5) {
        move_request.y = 1.0;
        move_request.x = 0.1;
    }
    if (getPosition2D().y - map.rect.position.y > 14) {
        move_request.y = -1.0;
        move_request.x = -0.1;
    }

    Pawn::onFixedUpdate();
}

bool Doux::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    auto ret = Pawn::onDamage(amount, target, source_position);
    if (ret) {
        hp -= amount;
        if (hp == 0) {
            pi->abilities.push_back(PlayerDino::Ability::Swimming);
            sp::P<Scene> scene = getScene();
            scene->msg("You kill doux and absorp his power!", [](){});

            pi->flags.emplace(key);
            die();
        }
    }
    return ret;
}