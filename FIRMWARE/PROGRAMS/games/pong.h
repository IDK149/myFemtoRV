#pragma once
#include "../include/hub75.h"
#include "../pong_assets.h"

void pong_set_mode(int multiplayer);
void pong_init(void);
int pong_input(int key);
void pong_update(void);
void pong_render(void);
