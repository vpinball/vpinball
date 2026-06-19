// license:GPLv3+

// Entry point for the cross-platform doctest based unit tests.
// These tests link the engine but do not boot the player/renderer, so they run headless on CI.

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char** argv)
{
   doctest::Context context;
   context.applyCommandLine(argc, argv);
   return context.run();
}
