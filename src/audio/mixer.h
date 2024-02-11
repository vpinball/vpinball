#pragma once

bool mixer_init(const HWND wnd);
void mixer_shutdown();

void mixer_get_volume();

void mixer_update();

void mixer_draw();
