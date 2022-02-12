#include <Cocoa/Cocoa.h>
#include <string.h>
#include "app.hpp"

#if !__has_feature(objc_arc)
#error "ARC is off"
#endif

@interface AppDelegate : NSObject <NSApplicationDelegate>
- (IBAction)menuCallback:(id)sender;
@end
@implementation AppDelegate {
}
- (IBAction)menuCallback:(id)sender {
    size_t menu_id = (size_t)[[sender representedObject] pointerValue];
    auto& inner = App::get_instance();
    inner.menu_handler(menu_id);
}
@end

static NSApplication* app;
static NSStatusBar* statusBar;
static NSStatusItem* statusItem;

void App::init_tray() {
    AppDelegate* delegate = [[AppDelegate alloc] init];
    app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    statusBar = [NSStatusBar systemStatusBar];
    statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
    [app activateIgnoringOtherApps:TRUE];
}

int App::loop() {
    @autoreleasepool {
        if (int ret = update()) {
            return ret;
        }

        NSDate* until = [NSDate dateWithTimeIntervalSinceNow:30];
        NSEvent* event =
            [app nextEventMatchingMask:ULONG_MAX
                             untilDate:until
                                inMode:[NSString stringWithUTF8String:"kCFRunLoopDefaultMode"]
                               dequeue:TRUE];
        if (event) {
            [app sendEvent:event];
        }
    }
    return 0;
}

int App::update() {
    statusItem.button.title = connected.has_value() ? @"〶" : @"〒";

    size_t connected_id = connected.value_or(-1);

    NSMenu* menu = [[NSMenu alloc] init];
    [menu setAutoenablesItems:FALSE];
    NSMenuItem* header =
        [[NSMenuItem alloc] initWithTitle:connected.has_value() ? @"Connected" : @"Disconnected"
                                   action:@selector(menuCallback:)
                            keyEquivalent:@""];
    [header setEnabled:FALSE];
    [header setState:0];
    [menu addItem:header];
    [menu addItem:[NSMenuItem separatorItem]];

    for (size_t i = 0; i < hosts.size(); ++i) {
        auto& host = hosts[i];
        int checked = i == connected_id;
        NSMenuItem* menuItem =
            [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:host]
                                       action:@selector(menuCallback:)
                                keyEquivalent:@""];
        [menuItem setEnabled:TRUE];
        [menuItem setState:checked];
        [menuItem setRepresentedObject:[NSValue valueWithPointer:(void*)i]];
        [menu addItem:menuItem];
    }
    [statusItem setMenu:menu];
    return 0;
}
