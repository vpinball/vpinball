#import <Cocoa/Cocoa.h>

extern "C" char **g_argv;
extern "C" int g_argc;
extern "C" int WinMain(void*, void*, void*, int);

@interface AppDelegate : NSObject <NSApplicationDelegate>
- (void)makeWindowExit;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    if (g_argc == 1) {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        panel.message = @"Select a Visual Pinball Table File:";
        panel.allowsMultipleSelection = NO;
        panel.canChooseDirectories = NO;

        [panel beginWithCompletionHandler:^(NSInteger result) {
            if (result == NSModalResponseOK) {
                NSURL *theDoc = panel.URLs[0];
                NSString *filename = [NSString stringWithUTF8String:[theDoc fileSystemRepresentation]];

                if (filename && filename.length > 0 && [[filename.lowercaseString pathExtension] isEqualToString:@"vpx"]) {
                    char **new_argv = (char **)malloc(4 * sizeof(char *));
                    new_argv[0] = g_argv[0];
                    new_argv[1] = strdup("-DisableTrueFullscreen");
                    new_argv[2] = strdup("-play");
                    new_argv[3] = strdup([filename UTF8String]);

                    g_argc = 4;
                    g_argv = new_argv;
                }
            } else {
                NSAlert *alert = [[NSAlert alloc] init];
                [alert setMessageText:@"VPinballX_GL\n\nThis app must be run using command line arguments or double clicking a \".vpx\" file."];
                [alert addButtonWithTitle:@"OK"];
                [alert setAlertStyle:NSAlertStyleWarning];
                [alert runModal];
            }
            [self makeWindowExit];
        }];
    }
}

@end
