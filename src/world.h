#pragma once

#include <sp2/string.h>
#include <sp2/math/rect.h>
#include <sp2/scene/node.h>
#include <sp2/scene/tilemap.h>


class World
{
public:
    void init();

    void loadMap(sp::P<sp::Node> root, sp::string name);
    bool loadMapAt(sp::P<sp::Node> root, sp::Vector2d position);
};

class Map
{
public:
    class TileFlags {
    public:
        bool water = false;
        bool burnable = false;
    };
    sp::Rect2i rect;
    std::vector<TileFlags> tiles;

    TileFlags& tileAt(sp::Vector2d position);

    sp::P<sp::Tilemap> main_tilemap;
    sp::PList<sp::Node> nodes;
};

extern World world;
extern Map map;
