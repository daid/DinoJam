#include "fire.h"
#include "world.h"

#include <sp2/random.h>
#include <sp2/scene/scene.h>


TileFire::TileFire(sp::P<sp::Node> parent, sp::Vector2d position)
: sp::ParticleEmitter(parent, "tilefire.particle")
{
    setPosition(position);
    lifetime.start(10.0);
    spreadstart.start(3.0);
}

void TileFire::onUpdate(float delta)
{
    sp::ParticleEmitter::onUpdate(delta);
    
    if (spreadstart.isExpired()) {
        tryStartFireAt(getParent(), getPosition2D() + sp::Vector2d( 1.0, 0.0));
        tryStartFireAt(getParent(), getPosition2D() + sp::Vector2d(-1.0, 0.0));
        tryStartFireAt(getParent(), getPosition2D() + sp::Vector2d( 0.0,-1.0));
        tryStartFireAt(getParent(), getPosition2D() + sp::Vector2d( 0.0, 1.0));
    }
    if (lifetime.isExpired()) {
        stopSpawn();
        auto_destroy = true;
        map.main_tilemap->setTile(sp::Vector2i(getPosition2D()) - map.rect.position, -1);
    }
}

bool TileFire::tryStartFireAt(sp::P<sp::Node> root, sp::Vector2d position)
{
    auto& tile = map.tileAt(position);
    if (!tile.burnable || tile.water)
        return false;
    tile.burnable = false;
    auto posi = sp::Vector2i(position);
    new TileFire(root, sp::Vector2d(posi) + sp::Vector2d(0.5, 0.5));
    return true;
}

Flamethrower::Flamethrower(sp::P<sp::Node> parent, sp::Vector2d position, double direction)
: sp::ParticleEmitter(parent, "flamethrower.particle")
{
    setPosition(position);
    setRotation(direction);

    fire_start_timer.repeat(0.5);
}

void Flamethrower::onFixedUpdate()
{
    if (fire_start_timer.isExpired() && !auto_destroy) {
        auto p = getGlobalPosition2D();
        auto v = sp::Vector2d(1.0, 0.0).rotate(getRotation2D());
        for(int n=0; n<5; n++) {
            p += v;
            TileFire::tryStartFireAt(getScene()->getRoot(), p);
        }
    }
}

Fireball::Fireball(sp::P<sp::Node> parent, sp::Vector2d position, sp::Vector2d velocity)
: sp::ParticleEmitter(parent, 16, sp::ParticleEmitter::Origin::Global), velocity(velocity)
{
    setPosition(position);
}

void Fireball::onFixedUpdate()
{
    TileFire::tryStartFireAt(getParent(), getPosition2D());

    for(int n=0; n<5; n++) {
        sp::ParticleEmitter::Parameters p;
        p.color = sp::HsvColor(sp::random(0, 360), 100, 100);
        p.velocity = sp::Vector3f(sp::random(-3, 3), sp::random(-3, 3), 0.0f);
        p.size = 0.1;
        p.lifetime = 0.1;
        emit(p);
    }
    
    setPosition(getPosition2D() + velocity);
    if (!lifetime--)
        delete this;
}
