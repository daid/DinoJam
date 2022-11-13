#include "mainScene.h"
#include "ingameMenu.h"
#include "main.h"

#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/mesh/obj.h>
#include <sp2/random.h>
#include <sp2/engine.h>
#include <sp2/scene/camera.h>
#include <sp2/scene/tilemap.h>
#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/collision/2d/box.h>
#include <sp2/io/cameraCapture.h>
#include <sp2/graphics/spriteAnimation.h>

#include <nlohmann/json.hpp>
#include <sp2/stringutil/convert.h>
#include <unordered_set>

#include "player.h"
#include "pawn.h"
#include "world.h"

class Camera : public sp::Camera
{
public:
    Camera(sp::P<sp::Node> parent)
    : sp::Camera(parent)
    {
        setOrtographic({10, 10});
    }

    void onUpdate(float delta) override
    {
        if (!target) return;
        sp::Vector2d v = sp::Vector2d(getProjectionMatrix().inverse() * sp::Vector2f(1, 1));

        auto target_position = target->getPosition2D();
        if (map.rect.size.x > v.x)
            target_position.x = std::clamp(target_position.x, map.rect.position.x + v.x, map.rect.position.x + map.rect.size.x - v.x);
        else
            target_position.x = map.rect.position.x + map.rect.size.x / 2;
        if (map.rect.size.y > v.y)
            target_position.y = std::clamp(target_position.y, map.rect.position.y + v.y, map.rect.position.y + map.rect.size.y - v.y);
        else
            target_position.y = map.rect.position.y + map.rect.size.y / 2;
        auto pos = getPosition2D();
        if ((pos - target_position).length() > 6)
            setPosition(target_position);
        else
            setPosition(getPosition2D() * 0.85 + target_position * 0.15);
    }

    sp::P<sp::Node> target;
};


class Bat : public sp::Node
{
public:
    Bat(sp::P<sp::Node> parent)
    : sp::Node(parent)
    {
        setAnimation(sp::SpriteAnimation::load("sprites/bat.txt"));
        animationPlay("Idle");
        auto shape = sp::collision::Box2D(1.0, 1.0);
        shape.type = sp::collision::Shape::Type::Kinematic;
        setCollisionShape(shape);
    }

    void onCollision(sp::CollisionInfo& info) override
    {
        sp::P<Damageable> obj = info.other;
        if (obj)
            obj->onDamage(1, DamageTarget::Player, getPosition2D());
    }
};


Scene::Scene()
: sp::Scene("MAIN")
{
    sp::Scene::get("INGAME_MENU")->enable();

    pi = std::make_unique<PlayerInfo>();
    pi->abilities.push_back(PlayerDino::Ability::Bite);
    pi->abilities.push_back(PlayerDino::Ability::Swimming);
    pi->abilities.push_back(PlayerDino::Ability::Fire);

    auto camera = new Camera(getRoot());
    setDefaultCamera(camera);

    world.loadMap(getRoot(), "start.json");

    pi->pawn = new PlayerPawn(getRoot());
    pi->pawn->setPosition(sp::Vector2d(12, 10));
    camera->target = pi->pawn;
    pi->dino = new PlayerDino(getRoot());
    pi->dino->setPosition(sp::Vector2d(16, 10));

    auto n = new Bat(getRoot());
    n->setPosition(sp::Vector2d(15, 10));
}

Scene::~Scene()
{
    sp::Scene::get("INGAME_MENU")->disable();
}

void Scene::onUpdate(float delta)
{
    if (pi->pawn) {
        auto position = pi->pawn->getPosition2D();
        if (pi->pawn->dino) position = pi->pawn->dino->getPosition2D();
        if (!map.rect.contains(sp::Vector2i(position))) {
            if (world.loadMapAt(getRoot(), position)) {
                getCamera()->onUpdate(0); // force the camera to update to prevent 1 frame glitch
            } else {
                LOG(Debug, "Cannot load map at:", position);
            }
        }
    }
}
