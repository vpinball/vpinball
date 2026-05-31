// license:GPLv3+

#include "core/stdafx.h"

#if defined(__linux__) && !defined(__ANDROID__)

#include "WaylandActivation.h"

#include <SDL3/SDL.h>

// wine/objbase.h (pulled in via stdafx.h) defines `interface` as `struct` for
// COM-compat. That macro pollutes the global namespace and breaks the Wayland
// headers, which use `interface` as a parameter name. Drop it before pulling
// the Wayland headers in.
#ifdef interface
#undef interface
#endif

#include <wayland-client.h>
#include "wayland-protocols/xdg-activation-v1-client-protocol.h"

#include <string>

namespace {

std::string s_startup_token;
struct xdg_activation_v1* s_activation_manager = nullptr;
bool s_registry_probed = false;

void registry_global(void* /*data*/, struct wl_registry* registry,
                     uint32_t name, const char* interface, uint32_t /*version*/)
{
   if (SDL_strcmp(interface, xdg_activation_v1_interface.name) == 0) {
      s_activation_manager = static_cast<struct xdg_activation_v1*>(
         wl_registry_bind(registry, name, &xdg_activation_v1_interface, 1));
   }
}

void registry_global_remove(void*, struct wl_registry*, uint32_t) {}

const struct wl_registry_listener registry_listener = {
   registry_global, registry_global_remove,
};

struct xdg_activation_v1* probe_manager(struct wl_display* display)
{
   if (s_registry_probed) return s_activation_manager;
   s_registry_probed = true;
   struct wl_registry* registry = wl_display_get_registry(display);
   if (!registry) return nullptr;
   wl_registry_add_listener(registry, &registry_listener, nullptr);
   wl_display_roundtrip(display);
   wl_registry_destroy(registry);
   if (!s_activation_manager) {
      PLOGI << "Compositor does not expose xdg_activation_v1 — Playfield will use default focus policy";
   }
   return s_activation_manager;
}

} // namespace

namespace WaylandActivation {

void SaveStartupToken(const char* token)
{
   if (token && *token) {
      s_startup_token = token;
      PLOGI << "Stashed XDG_ACTIVATION_TOKEN (len " << s_startup_token.size() << ") for Playfield activation";
   }
}

bool HasStartupToken()
{
   return !s_startup_token.empty();
}

void ActivatePlayfield(struct wl_display* display, struct wl_surface* surface)
{
   if (s_startup_token.empty() || !display || !surface) return;
   const char* drv = SDL_GetCurrentVideoDriver();
   if (!drv || SDL_strcmp(drv, "wayland") != 0) return;

   struct xdg_activation_v1* mgr = probe_manager(display);
   if (!mgr) {
      s_startup_token.clear();
      return;
   }

   xdg_activation_v1_activate(mgr, s_startup_token.c_str(), surface);
   wl_display_flush(display);
   PLOGI << "Applied XDG_ACTIVATION_TOKEN to Playfield surface (len " << s_startup_token.size() << ')';

   s_startup_token.clear();
   SDL_unsetenv_unsafe("XDG_ACTIVATION_TOKEN");
}

} // namespace WaylandActivation

#endif
