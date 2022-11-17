#include "emote.h"

#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>


Emote::Emote(sp::P<sp::Node> parent, int index)
: sp::Node(parent)
{
    setPosition(sp::Vector2d(0, 1.5));

    render_data.type = sp::RenderData::Type::Normal;
    render_data.order = 99;
    render_data.texture = sp::texture_manager.get("sprites/emotes.png");
    render_data.shader = sp::Shader::get("internal:basic.shader");
    auto uv = sp::Vector2f(1.0f/5.0f * (index % 5), 1.0f/6.0f * (index / 5));
    render_data.mesh = sp::MeshData::createQuad({1, 1}, uv, uv + sp::Vector2f{1.0f/5.0f, 1.0f/6.0f});

    lifetime.start(1.0);
}

void Emote::onUpdate(float delta)
{
    if (lifetime.isExpired())
        delete this;
}
