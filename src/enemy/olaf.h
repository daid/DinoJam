#pragma once

#include "../pawn.h"


class Olaf : public Pawn
{
public:
    Olaf(sp::P<sp::Node> parent);

    void onFixedUpdate() override;

private:
    sp::Timer next_action_timer;
};