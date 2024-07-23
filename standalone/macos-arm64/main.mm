#import <Cocoa/Cocoa.h>

extern "C" char** g_argv;
extern "C" int g_argc;
extern "C" int WinMain(void*, void*, void*, int);

@interface VPXAppDelegate : NSObject <NSApplicationDelegate>

- (void)makeWindowExit;

@end

@implementation VPXAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
  if (g_argc == 1) {
    NSOpenPanel* panel = [NSOpenPanel openPanel];

    panel.message = @"Select a Visual Pinball Table (.vpx) File:";
    panel.allowsMultipleSelection = NO;
    panel.canChooseDirectories = NO;

    [panel beginWithCompletionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK) {

          NSURL* fileURL = panel.URLs[0]; 
          NSString* vpxTableArchiveFile =
              [NSString stringWithUTF8String:[fileURL fileSystemRepresentation]];

          if (vpxTableArchiveFile && vpxTableArchiveFile.length > 0 &&
              [[vpxTableArchiveFile.lowercaseString pathExtension]
                  isEqualToString:@"vpx"]) {
            char** new_argv = (char**)malloc(3 * sizeof(char*));

            new_argv[0] = g_argv[0];
            new_argv[1] = strdup("-play");
            new_argv[2] = strdup([vpxTableArchiveFile UTF8String]);

            g_argc = 3;
            g_argv = new_argv;
          }
        } else {
          NSAlert* alert = [[NSAlert alloc] init];
          [alert setMessageText:
                     @"VPinballX_GL\n\nYou must choose a VPX table to start, run using the command line "
                     @" to set arguments, or\ndouble click a \".vpx\" file."];
          [alert addButtonWithTitle:@"OK"];
          [alert setAlertStyle:NSAlertStyleWarning];
          [panel close];
          [alert runModal];
        }
        [self makeWindowExit];
    }];
  } else if (g_argc > 1) {
    [self makeWindowExit];
  }
}

- (void)makeWindowExit
{
  int status = WinMain(NULL, NULL, NULL, 0);
  exit(status);
}

- (BOOL)application:(NSApplication*)sender openFile:(NSString*)filename
{
  if (g_argc == 1
      && (filename && filename.length > 0 &&
          [[filename.lowercaseString pathExtension] isEqualToString:@"vpx"])) {
    char** new_argv = (char**)malloc(3 * sizeof(char*));

    new_argv[0] = g_argv[0];
    new_argv[1] = strdup("-play");
    new_argv[2] = strdup([filename UTF8String]);

    g_argc = 3;
    g_argv = new_argv;
  }

  [self makeWindowExit];

  return YES;
}

@end

int main(int argc, const char* argv[])
{
  @autoreleasepool {
    g_argc = argc;
    g_argv = (char**)argv;

    NSApplication* vpxApp = [NSApplication sharedApplication];
    VPXAppDelegate* delegate = [[VPXAppDelegate alloc] init];
    [vpxApp setDelegate:delegate];
    [vpxApp run];
  }

  return 0;
}
