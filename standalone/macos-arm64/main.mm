#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

extern "C" char **g_argv;
extern "C" int g_argc;
extern "C" int WinMain(void*, void*, void*, int);

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
   if (g_argc == 1) {
      NSAlert* alert = [[NSAlert alloc] init];
      [alert setMessageText:@"VPinballX_GL\n\nThis app must be run using command line arguments or double clicking a \".vpx\" file."];
      [alert addButtonWithTitle:@"OK"];
      [alert setAlertStyle:NSAlertStyleWarning];
      [alert runModal];
   }

   int status = WinMain(NULL, NULL, NULL, 0);
   exit(status);
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
   if (g_argc == 1 && (filename && filename.length > 0 && [[filename.lowercaseString pathExtension] isEqualToString:@"vpx"])) {
      char **new_argv = (char**)malloc(3 * sizeof(char *));

      new_argv[0] = g_argv[0];
      new_argv[1] = strdup("-play");
      new_argv[2] = strdup([filename UTF8String]);

      g_argc = 3;
      g_argv = new_argv;
    }

    return YES;
}

@end

int main(int argc, const char * argv[])
{
   @autoreleasepool {
      g_argc = argc;
      g_argv = (char**)argv;

      [NSApplication sharedApplication];
      AppDelegate *delegate = [[AppDelegate alloc] init];
      [NSApp setDelegate:delegate];
      [NSApp run];
   }
   return 0;
}