// license:GPLv3+

#pragma once

#if defined(__linux__) && !defined(__ANDROID__)

struct wl_display;
struct wl_surface;

namespace WaylandActivation {

void SaveStartupToken(const char* token);
bool HasStartupToken();
void ActivatePlayfield(struct wl_display* display, struct wl_surface* surface);

} // namespace WaylandActivation

#endif
