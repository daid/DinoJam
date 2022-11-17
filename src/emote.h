#pragma once

#include <sp2/scene/node.h>
#include <sp2/timer.h>


class Emote : public sp::Node
{
public:
    Emote(sp::P<sp::Node> parent, int index);

    void onUpdate(float delta) override;

    sp::Timer lifetime;
};