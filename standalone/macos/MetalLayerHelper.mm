// license:GPLv3+
// Isolated ObjC++ helper to avoid BOOL typedef conflicts between <objc/objc.h> and Wine headers.
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// Creates a CAMetalLayer directly on the NSWindow's content view.
// Must be called on the main thread so Core Animation registers the layer with the display
// compositor. Creating it on a background thread causes drawables to never be retired.
extern "C" void* VPX_CreateMetalLayer(void* nsWindow)
{
   NSWindow* window = (__bridge NSWindow*)nsWindow;
   NSView* view = window.contentView;
   view.wantsLayer = YES;
   CAMetalLayer* metalLayer = [CAMetalLayer layer];
   metalLayer.device = MTLCreateSystemDefaultDevice();
   metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
   metalLayer.framebufferOnly = YES;
   view.layer = metalLayer;
   metalLayer.frame = view.bounds;
   return (__bridge void*)metalLayer;
}
