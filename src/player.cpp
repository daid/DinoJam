#include "player.h"
#include "main.h"
#include "world.h"
#include "fire.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/collision/2d/box.h>
#include <sp2/scene/scene.h>


std::unique_ptr<PlayerInfo> pi;

PlayerPawn::PlayerPawn(sp::P<sp::Node> parent)
: Pawn(parent, {0.5, 0.8}, DamageTarget::Player)
{
    setAnimation(sp::SpriteAnimation::load("sprites/player.txt"));
    animationPlay("Idle");
    render_data.order = 1;
    jump_power = 17;
}

void PlayerPawn::onUpdate(float delta)
{
    if (dino) {
        if (dino->animationGetFlags())
            setPosition(dino->getPosition2D() + sp::Vector2d(0.5, 0.5));
        else
            setPosition(dino->getPosition2D() + sp::Vector2d(-0.5, 0.5));
    }
}

void PlayerPawn::onFixedUpdate()
{
    if (dino) {
        if (controller.secondary_action.getDown()) {
            auto shape = sp::collision::Box2D(0.5, 0.8);
            shape.setFilterCategory(1);
            shape.setMaskFilterCategory(1);
            shape.linear_damping = 0.8;
            shape.fixed_rotation = true;
            setCollisionShape(shape);
            if (dino->animationGetFlags())
                setLinearVelocity(dino->getLinearVelocity2D() + sp::Vector2d(15, 10));
            else
                setLinearVelocity(dino->getLinearVelocity2D() + sp::Vector2d(-15, 10));
            render_data.order = 1;
            dino->has_rider = false;
            dino = nullptr;
        }
    } else {
        move_request.x = controller.right.getValue() - controller.left.getValue();
        move_request.y = controller.up.getValue() - controller.down.getValue();
        jump_request = controller.primary_action.getDown();

        if (controller.secondary_action.getDown()) {
            getScene()->queryCollision({getPosition2D(), {0, 0}}, [this](sp::P<sp::Node> node) {
                dino = node;
                if (dino)
                    return false;
                return true;
            });
            if (dino) {
                removeCollisionShape();
                on_floor = 0;
                dino->has_rider = true;
                render_data.order = -1;
                setPosition(dino->getPosition2D() + sp::Vector2d(0.5, 0.5));
            }
        }

        Pawn::onFixedUpdate();
    }
}

PlayerDino::PlayerDino(sp::P<sp::Node> parent)
: Pawn(parent, {1.4, 1.75}, DamageTarget::Player)
{
    setAnimation(sp::SpriteAnimation::load("sprites/dino/olaf_male.txt"));
    animationPlay("Idle");
    render_data.order = 0;
}

void PlayerDino::onFixedUpdate()
{
    if (has_rider) {
        move_request.x = controller.right.getValue() - controller.left.getValue();
        move_request.y = controller.up.getValue() - controller.down.getValue();
        jump_request = controller.primary_action.getDown();

        if (controller.special_action.getDown() && (on_floor || (can_swim && in_water))) {
            auto flags = animationGetFlags();
            ability = pi->nextAbility(ability);
            switch(ability)
            {
            case Ability::Bite: setAnimation(sp::SpriteAnimation::load("sprites/dino/olaf_male.txt")); break;
            case Ability::Swimming: setAnimation(sp::SpriteAnimation::load("sprites/dino/doux_male.txt")); break;
            case Ability::Dash: setAnimation(sp::SpriteAnimation::load("sprites/dino/nico_male.txt")); break;
            case Ability::Fire: setAnimation(sp::SpriteAnimation::load("sprites/dino/mort_male.txt")); break;
            }
            animationSetFlags(flags);
            can_swim = ability == Ability::Swimming;
        }

        if (controller.protect_action.getDown()) {
            switch(ability)
            {
            case Ability::Bite:
                special_anim = true;
                animationPlay("Bite");
                break;
            case Ability::Swimming:
                break;
            case Ability::Dash:
                break;
            case Ability::Fire:
                break;
            }
        }
        if (ability == Ability::Fire && controller.protect_action.get() && on_floor) {
            if (!flamethrower) {
                if (animationGetFlags())
                    //new Fireball(getParent(), getPosition2D(), sp::Vector2d(-0.3, 0.0));
                    flamethrower = new Flamethrower(this, sp::Vector2d(-0.5, 0.4), 180);
                else
                    //new Fireball(getParent(), getPosition2D() + sp::Vector2d(0.5, 0.6), sp::Vector2d(0.3, 0.0));
                    flamethrower = new Flamethrower(this, sp::Vector2d(0.5, 0.4), 0);
            }
            move_request = {0.0, 0.0};
            jump_request = false;
            special_anim = true;
            animationPlay("Fire");
        } else if (flamethrower) {
            flamethrower->stopSpawn();
            flamethrower->auto_destroy = true;
            flamethrower = nullptr;
        }
    } else {
        move_request = {0.0, 0.0};
        if (flamethrower) {
            flamethrower->stopSpawn();
            flamethrower->auto_destroy = true;
            flamethrower = nullptr;
        }
    }

    Pawn::onFixedUpdate();
    pi->dino_location = getPosition2D();
}

PlayerDino::Ability PlayerInfo::nextAbility(PlayerDino::Ability ability)
{
    for(size_t n=0; n<abilities.size(); n++)
        if (abilities[n] == ability)
            return abilities[(n + 1) % abilities.size()];
    return PlayerDino::Ability::Bite;
}
