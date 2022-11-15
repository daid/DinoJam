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
        if (!pi->pawn) return;
        sp::Vector2d v = sp::Vector2d(getProjectionMatrix().inverse() * sp::Vector2f(1, 1));

        auto target_position = pi->pawn->getPosition2D();
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
};


class Bat : public Damageable
{
public:
    Bat(sp::P<sp::Node> parent)
    : Damageable(parent)
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

    bool onDamage(int amount, DamageTarget target, sp::Vector2d source_position) override
    {
        if (target == DamageTarget::Player) return false;
        delete this;
        return true;
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
    pi->abilities.push_back(PlayerDino::Ability::Dash);

    auto camera = new Camera(getRoot());
    setDefaultCamera(camera);

    world.loadMap(getRoot(), "start.json");

    auto n = new Bat(getRoot());
    n->setPosition(sp::Vector2d(15, 4));

    hud = sp::gui::Loader::load("gui/hud.gui", "HUD");
}

Scene::~Scene()
{
    sp::Scene::get("INGAME_MENU")->disable();
    hud.destroy();
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

    auto hpbar = hud->getWidgetWithID("HPBAR");
    while(hpbar->getChildren().size() < pi->health_max / 2)
        sp::gui::Loader::load("gui/hud.gui", "HP", hpbar);
    int cur_hp = 0;
    for(auto hp : hpbar->getChildren()) {
        if (cur_hp + 1 < pi->health)
            sp::P<sp::gui::Widget>(hp)->setAttribute("texture", "gui/heart.png");
        else if (cur_hp < pi->health)
            sp::P<sp::gui::Widget>(hp)->setAttribute("texture", "gui/heart2.png");
        else
            sp::P<sp::gui::Widget>(hp)->setAttribute("texture", "gui/heart3.png");
        cur_hp += 2;
    }

    if (pi->health <= 0 && !death_timer.isRunning()) {
        death_timer.start(3);
    }
    if (death_timer.isExpired()) {
        msg("You died.", [this]() {
            pi->health = pi->health_max;
            pi->pawn.destroy();
            pi->dino.destroy();
            world.loadMap(getRoot(), "start.json");
            death_timer.stop();
        });
    }

    if (hud->getWidgetWithID("MSGBOX")->isVisible() && controller.primary_action.getDown()) {
        auto func = msg_done_func;
        hud->getWidgetWithID("MSGBOX")->hide();
        func();
        sp::Engine::getInstance()->setPause(false);
    }
}

void Scene::msg(const sp::string& msg, std::function<void()> func)
{
    sp::Engine::getInstance()->setPause(true);
    hud->getWidgetWithID("MSGBOX")->show();
    hud->getWidgetWithID("MSG")->setAttribute("caption", msg);
    msg_done_func = func;
}
