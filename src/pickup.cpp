#include "pickup.h"
#include "player.h"

#include <sp2/graphics/spriteAnimation.h>
#include <sp2/collision/2d/box.h>
#include <sp2/audio/sound.h>


Coin::Coin(sp::P<sp::Node> parent, sp::string key)
: sp::Node(parent), key(key)
{
    setAnimation(sp::SpriteAnimation::load("sprites/coin.txt"));
    animationPlay("Idle");
    sp::collision::Box2D shape(1, 1);
    shape.type = sp::collision::Shape::Type::Sensor;
    setCollisionShape(shape);
}

void Coin::onCollision(sp::CollisionInfo& info)
{
    if (info.other == pi->pawn || info.other == pi->dino) {
        sp::audio::Sound::play("sfx/pickupCoin.wav");
        pi->coins += 1;
        pi->flags.emplace(key);
        delete this;
    }
}

Key::Key(sp::P<sp::Node> parent, sp::string key)
: sp::Node(parent), key(key)
{
    setAnimation(sp::SpriteAnimation::load("sprites/key.txt"));
    animationPlay("Idle");
    sp::collision::Box2D shape(1, 1);
    shape.type = sp::collision::Shape::Type::Sensor;
    setCollisionShape(shape);
}

void Key::onCollision(sp::CollisionInfo& info)
{
    if (info.other == pi->pawn || info.other == pi->dino) {
        sp::audio::Sound::play("sfx/pickupCoin.wav");
        pi->keys += 1;
        pi->flags.emplace(key);
        delete this;
    }
}

KeyBlock::KeyBlock(sp::P<sp::Node> parent, sp::string key)
: sp::Node(parent), key(key)
{
    setAnimation(sp::SpriteAnimation::load("sprites/key.txt"));
    animationPlay("Block");
    sp::collision::Box2D shape(2, 2);
    shape.type = sp::collision::Shape::Type::Kinematic;
    setCollisionShape(shape);
}

void KeyBlock::onCollision(sp::CollisionInfo& info)
{
    if (info.other == pi->pawn || info.other == pi->dino) {
        if (pi->keys > 0) {
            pi->keys += 0;
            pi->flags.emplace(key);
            delete this;
        }
    }
}
