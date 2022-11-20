#include "world.h"

#include <sp2/io/resourceProvider.h>
#include <sp2/math/rect.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/tilemap.h>
#include <sp2/scene/camera.h>

#include <nlohmann/json.hpp>
#include <unordered_set>

#include "player.h"
#include "pickup.h"
#include "enemy/olaf.h"
#include "enemy/mort.h"
#include "enemy/nico.h"
#include "enemy/doux.h"
#include "enemy/bat.h"


World world;
Map map;

class MapInfo
{
public:
    sp::string name;
    sp::Rect2i rect;
};

std::vector<MapInfo> maps;


void World::init()
{
    auto json = nlohmann::json::parse(sp::io::ResourceProvider::get("world/world.world")->readAll(), nullptr, false);
    for(auto& json_map : json["maps"]) {
        auto name = static_cast<std::string>(json_map["fileName"]);
        //auto width = static_cast<int>(json_map["width"]) / 18;
        //auto height = static_cast<int>(json_map["height"]) / 18;
        int x = std::floor(static_cast<float>(json_map["x"]) / 18.0f);
        int y = std::floor(static_cast<float>(json_map["y"]) / 18.0f);
        auto map_json = nlohmann::json::parse(sp::io::ResourceProvider::get("world/" + name)->readAll(), nullptr, false);
        auto width = static_cast<int>(map_json["width"]);
        auto height = static_cast<int>(map_json["height"]);
        
        y = - height - y;
        LOG(Debug, x, y, width, height, name);
        maps.push_back({name, {{x, y}, {width, height}}});
    }
}

void World::loadMap(sp::P<sp::Node> root, sp::string map_name)
{
    for(auto& info : maps) {
        if (info.name == map_name) {
            map.rect = info.rect;
        }
    }
    for(auto obj : root->getChildren()) {
        if (obj == root->getScene()->getCamera()) continue;
        if (obj == pi->pawn) continue;
        if (pi->pawn && obj == pi->pawn->dino) continue;
        obj.destroy();
    }

    if (!pi->dino && !pi->abilities.empty() && map.rect.contains(sp::Vector2i(pi->dino_location))) {
        pi->dino = new PlayerDino(root);
        pi->dino->setPosition(pi->dino_location);
    }
    auto tileset_json = nlohmann::json::parse(sp::io::ResourceProvider::get("world/tileset.json")->readAll(), nullptr, false);
    std::unordered_set<int> solid_tiles, water_tiles, burn_tiles, platform_tiles;
    for(auto& tile : tileset_json["tiles"]) {
        int id = static_cast<int>(tile["id"]);
        for(auto& prop : tile["properties"]) {
            auto name = static_cast<std::string>(prop["name"]);
            if (name == "solid") solid_tiles.insert(id);
            if (name == "water") water_tiles.insert(id);
            if (name == "platform") platform_tiles.insert(id);
            if (name == "burnable") burn_tiles.insert(id);
        }
    }

    auto json = nlohmann::json::parse(sp::io::ResourceProvider::get("world/" + map_name)->readAll(), nullptr, false);
    map.tiles.clear();
    map.tiles.resize(map.rect.size.x * map.rect.size.y);
    int tilemap_order = -10;
    for(auto& layer : json["layers"]) {
        if (static_cast<std::string>(layer["type"]) == "tilelayer") {
            auto tilemap = new sp::Tilemap(root, "tiles.png", 1, 1, 20, 9);
            tilemap->setPosition(sp::Vector2d(map.rect.position));
            tilemap->setTilemapSpacingMargin(0.01f, 0.0f);
            tilemap->render_data.order = tilemap_order;
            map.nodes.add(tilemap);
            map.main_tilemap = tilemap;
            tilemap_order++;
            auto size = sp::Vector2i(static_cast<int>(layer["width"]), static_cast<int>(layer["height"]));
            for(auto p : sp::Rect2i({0, 0}, size)) {
                int idx = static_cast<int>(layer["data"][p.x + (size.y - 1 - p.y) * size.x]) - 1;
                if (idx >= 0) {
                    if (water_tiles.find(idx) != water_tiles.end())
                        map.tiles[p.x + map.rect.size.x * p.y].water = true;
                    if (burn_tiles.find(idx) != burn_tiles.end())
                        map.tiles[p.x + map.rect.size.x * p.y].burnable = true;
                    if (solid_tiles.find(idx) != solid_tiles.end())
                        tilemap->setTile(p, idx, sp::Tilemap::Collision::Solid);
                    else if (platform_tiles.find(idx) != platform_tiles.end())
                        tilemap->setTile(p, idx, sp::Tilemap::Collision::Platform);
                    else
                        tilemap->setTile(p, idx);
                }
            }
            tilemap->onFixedUpdate(); // Force an update
        }
        if (static_cast<std::string>(layer["type"]) == "objectgroup") {
            for(auto& obj : layer["objects"]) {
                float x = obj["x"];
                float y = obj["y"];
                float width = obj["width"];
                float height = obj["height"];
                std::string name = obj["name"];
                std::string type = obj["type"];
                std::string key = map_name + ":" + name + ":" + std::to_string(static_cast<int>(obj["id"]));
                sp::Vector2d position{x / 18.0f + width / 36.0f, float(map.rect.size.y) - y / 18.0f - height / 36.0f};
                position += sp::Vector2d(map.rect.position);

                if (pi->flags.find(key) != pi->flags.end())
                    continue;

                if (name == "start") {
                    if (!pi->pawn) {
                        pi->pawn = new PlayerPawn(root);
                        pi->pawn->setPosition(position);
                    }
                } else if (name == "playerdino") {
                    if (!pi->dino && !pi->abilities.empty()) {
                        pi->dino = new PlayerDino(root);
                        pi->dino->setPosition(position);
                    }
                } else if (name == "olaf") {
                    (new Olaf(root, key))->setPosition(position);
                } else if (name == "mort") {
                    (new Mort(root, key))->setPosition(position);
                } else if (name == "nico") {
                    (new Nico(root, key))->setPosition(position);
                } else if (name == "doux") {
                    (new Doux(root, key))->setPosition(position);
                } else if (name == "bat") {
                    (new Bat(root))->setPosition(position);
                } else if (name == "apple") {
                    (new Apple(root))->setPosition(position);
                } else if (name == "coin") {
                    (new Coin(root, key))->setPosition(position);
                } else if (name == "key") {
                    (new Key(root, key))->setPosition(position);
                } else if (name == "keyblock") {
                    (new KeyBlock(root, key))->setPosition(position);
                } else {
                    LOG(Warning, "Unknown level object type:", name);
                }
            }
        }
    }
}

bool World::loadMapAt(sp::P<sp::Node> root, sp::Vector2d position)
{
    for(auto& info : maps) {
        if (info.rect.contains(sp::Vector2i(std::floor(position.x), std::floor(position.y)))) {
            loadMap(root, info.name);
            return true;
        }
    }
    return false;
}

Map::TileFlags& Map::tileAt(sp::Vector2d position)
{
    auto posi = sp::Vector2i(std::floor(position.x), std::floor(position.y)) - rect.position;
    posi.x = std::clamp(posi.x, 0, rect.size.x - 1);
    posi.y = std::clamp(posi.y, 0, rect.size.y - 1);
    return tiles[posi.x + posi.y * rect.size.x];
}
