#pragma once

#include <sp2/scene/node.h>


class Coin : public sp::Node
{
public:
    Coin(sp::P<sp::Node> parent, sp::string key);

    void onCollision(sp::CollisionInfo& info) override;

    sp::string key;
};

class Key : public sp::Node
{
public:
    Key(sp::P<sp::Node> parent, sp::string key);

    void onCollision(sp::CollisionInfo& info) override;

    sp::string key;
};

class KeyBlock : public sp::Node
{
public:
    KeyBlock(sp::P<sp::Node> parent, sp::string key);

    void onCollision(sp::CollisionInfo& info) override;

    sp::string key;
};