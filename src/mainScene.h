#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/timer.h>


class Scene : public sp::Scene
{
public:
    Scene();
    ~Scene();

    virtual void onUpdate(float delta) override;

    void msg(const sp::string& msg, std::function<void()> func);

    sp::P<sp::gui::Widget> hud;
    std::function<void()> msg_done_func;
    sp::Timer death_timer;
};

#endif//MAIN_SCENE_H
