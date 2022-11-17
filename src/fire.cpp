#include "fire.h"
#include "world.h"
#include "pawn.h"

#include <sp2/random.h>
#include <sp2/scene/scene.h>
#include <sp2/audio/sound.h>
#include <sp2/audio/audioSource.h>


FireEffect::FireEffect(sp::P<sp::Node> parent, sp::Vector2d position)
: sp::ParticleEmitter(parent, "tilefire.particle")
{
    sp::audio::Sound::play("sfx/fire.wav");
    setPosition(position);
    lifetime.start(1.5);
}

void FireEffect::onUpdate(float delta)
{
    sp::ParticleEmitter::onUpdate(delta);
    
    if (lifetime.isExpired()) {
        stopSpawn();
        auto_destroy = true;
        map.main_tilemap->setTile(sp::Vector2i(getPosition2D()) - map.rect.position, -1);
    }
}


TileFire::TileFire(sp::P<sp::Node> parent, sp::Vector2d position)
: sp::ParticleEmitter(parent, "tilefire.particle")
{
    sp::audio::Sound::play("sfx/fire.wav");
    setPosition(position);
    lifetime.start(4.0);
    spreadstart.start(1.5);
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

class FireNoise : public sp::audio::AudioSource
{
public:
    virtual void onMixSamples(float* stream, int sample_count) override
    {
        for(int n=0; n<sample_count; n++) {
            stream[n] += sample;
            if (delay) {
                delay--;
            } else {
                sample = sp::random(-volume, volume);
                delay = 100;
            }
        }
    }
    int active_count = 0;

    float volume = 0.25;
    float sample = 0.0f;
    int delay = 0;
};

static FireNoise* noise;
Flamethrower::Flamethrower(sp::P<sp::Node> parent, sp::Vector2d position, double direction, DamageTarget damage_target)
: sp::ParticleEmitter(parent, "flamethrower.particle"), damage_target(damage_target)
{
    setPosition(position);
    setRotation(direction);

    fire_start_timer.repeat(0.35);
    if (!noise)
        noise = new FireNoise();
    if (noise->active_count == 0)
        noise->start();
    noise->active_count += 1;
}

Flamethrower::~Flamethrower()
{
    if (has_noise) {
        noise->active_count -= 1;
        if (noise->active_count == 0)
            noise->stop();
    }
}

void Flamethrower::onFixedUpdate()
{
    if (auto_destroy && has_noise) {
        noise->active_count -= 1;
        if (noise->active_count == 0)
            noise->stop();
        has_noise = false;
    }
    if (fire_start_timer.isExpired() && !auto_destroy) {
        auto p = getGlobalPosition2D();
        auto v = sp::Vector2d(1.0, 0.0).rotate(getRotation2D());
        for(int n=0; n<5; n++) {
            p += v;
            TileFire::tryStartFireAt(getScene()->getRoot(), p);
        }

        for(float y=-0.2;y<=0.2; y+=0.2) {
            getScene()->queryCollisionAny(sp::Ray2d(getGlobalPosition2D(), getGlobalPoint2D({5, y})), [this](sp::P<sp::Node> n, sp::Vector2d hit_location, sp::Vector2d hit_normal) {
                sp::P<Damageable> dmg = n;
                if (dmg) {
                    if (dmg->onDamage(1, damage_target, getGlobalPosition2D())) {
                        new FireEffect(getScene()->getRoot(), hit_location);
                    }
                }
                return true;
            });
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
