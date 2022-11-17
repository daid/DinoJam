#include "player.h"
#include "main.h"
#include "world.h"
#include "fire.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/collision/2d/box.h>
#include <sp2/scene/scene.h>
#include <sp2/audio/sound.h>


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
            setPosition(dino->getPosition2D() + sp::Vector2d(-0.5, 0.3));
        else
            setPosition(dino->getPosition2D() + sp::Vector2d(0.5, 0.3));
    }
    bool under_water = map.tileAt(getPosition2D()).water;
    if (dino && dino->ability == PlayerDino::Ability::Swimming) under_water = false;

    if (under_water) {
        oxygen -= delta * 0.2f;
        if (oxygen < -0.1f) {
            oxygen += 0.2f;
            if (dino)
                dino->onDamage(1, DamageTarget::Player, dino->getPosition2D());
            else
                onDamage(1, DamageTarget::Player, getPosition2D());
        }
    } else {
        oxygen = std::min(1.0f, oxygen + delta);
    }
    if (oxygen == 1.0f || dead) {
        oxygen_bar.destroy();
    } else {
        if (!oxygen_bar) {
            oxygen_bar = new sp::Node(this);
            oxygen_bar->setPosition(sp::Vector2d(0, 1));
            oxygen_bar->render_data.type = sp::RenderData::Type::Normal;
            oxygen_bar->render_data.order = 100;
            oxygen_bar->render_data.shader = sp::Shader::get("internal:color.shader");
            oxygen_bar->render_data.mesh = sp::MeshData::createQuad({1, 0.2});
            oxygen_bar->render_data.color = sp::Color(0.2, 0.4, 1.0);
        }
        oxygen_bar->render_data.mesh = sp::MeshData::createQuad({std::max(0.1f, oxygen * 2.0f), 0.2});
    }
}

void PlayerPawn::onFixedUpdate()
{
    if (dino) {
        if (controller.protect_action.getDown()) {
            stepOffDino();
        }
    } else {
        move_request.x = controller.right.getValue() - controller.left.getValue();
        move_request.y = controller.up.getValue() - controller.down.getValue();
        jump_request = controller.primary_action.getDown();
        high_jump_request = controller.primary_action.get();

        if (controller.protect_action.getDown()) {
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

void PlayerPawn::stepOffDino()
{
    auto shape = sp::collision::Box2D(0.5, 0.8);
    shape.setFilterCategory(1);
    shape.setMaskFilterCategory(1);
    shape.linear_damping = 0.8;
    shape.fixed_rotation = true;
    setCollisionShape(shape);
    if (dino->animationGetFlags())
        setLinearVelocity(dino->getLinearVelocity2D() + sp::Vector2d(-15, 10));
    else
        setLinearVelocity(dino->getLinearVelocity2D() + sp::Vector2d(15, 10));
    render_data.order = 1;
    dino->has_rider = false;
    dino = nullptr;
}

bool PlayerPawn::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    auto res = Pawn::onDamage(amount, target, source_position);
    if (res) {
        sp::audio::Sound::play("sfx/hurt.wav");
        pi->health = std::max(0, pi->health - amount);
        if (pi->health == 0)
            die();
    }
    return res;
}

PlayerDino::PlayerDino(sp::P<sp::Node> parent)
: Pawn(parent, {1.2, 1.75}, DamageTarget::Player)
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
        high_jump_request = controller.primary_action.get();

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

        if (controller.secondary_action.getDown()) {
            switch(ability)
            {
            case Ability::Bite:
                if (!special_anim) {
                    special_anim = true;
                    animationPlay("Bite");
                    sp::audio::Sound::play("sfx/chomp.wav");
                    auto p = getPosition2D();
                    if (animationGetFlags())
                        p.x += 1.0;
                    else
                        p.x -= 1.0;
                    getScene()->queryCollision({p - sp::Vector2d(0.75, 0.75), sp::Vector2d(1.5, 1.5)}, [this, p](sp::P<sp::Node> n) {
                        sp::P<Damageable> dmg = n;
                        if (dmg)
                            dmg->onDamage(1, DamageTarget::Enemy, getPosition2D());
                        return true;
                    });
                }
                break;
            case Ability::Swimming:
                break;
            case Ability::Dash:
                if (allow_dash) {
                    if (animationGetFlags())
                        setLinearVelocity(sp::Vector2d(20, 0));
                    else
                        setLinearVelocity(sp::Vector2d(-20, 0));
                    dash_timer.start(0.3);
                    animationPlay("Dash");
                    allow_dash = false;
                }
                break;
            case Ability::Fire:
                break;
            }
        }
        if (ability == Ability::Fire && controller.secondary_action.get() && on_floor && !in_water) {
            if (!flamethrower) {
                if (animationGetFlags())
                    flamethrower = new Flamethrower(this, sp::Vector2d(0.5, 0.1), 0, DamageTarget::Enemy);
                else
                    flamethrower = new Flamethrower(this, sp::Vector2d(-0.5, 0.1), 180, DamageTarget::Enemy);
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

bool PlayerDino::onDamage(int amount, DamageTarget target, sp::Vector2d source_position)
{
    auto res = Pawn::onDamage(amount, target, source_position);
    if (res && has_rider) {
        sp::audio::Sound::play("sfx/hurt.wav");
        pi->health = std::max(0, pi->health - amount);
        if (pi->health == 0) {
            die();
            if (pi->pawn) {
                pi->pawn->stepOffDino();
                pi->pawn->die();
            }
        }
    }
    return res;
}
