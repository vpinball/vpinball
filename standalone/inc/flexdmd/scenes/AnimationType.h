#pragma once

typedef enum {
   AnimationType_FadeIn = 0, // Fade from black to scene
   AnimationType_FadeOut = 1, // Fade from scene to black
   AnimationType_ZoomIn = 2, // zoom from a centered small dmd to full size
   AnimationType_ZoomOut = 3, // zoom from a full sized dmd to an oversize one
   AnimationType_ScrollOffLeft = 4,
   AnimationType_ScrollOffRight = 5,
   AnimationType_ScrollOnLeft = 6,
   AnimationType_ScrollOnRight = 7,
   AnimationType_ScrollOffUp = 8,
   AnimationType_ScrollOffDown = 9,
   AnimationType_ScrollOnUp = 10,
   AnimationType_ScrollOnDown = 11,
   AnimationType_FillFadeIn = 12, // fade from black to white (the scene won't be seen)
   AnimationType_FillFadeOut = 13, // fade from white to black (the scene won't be seen)
   AnimationType_None = 14
} AnimationType;