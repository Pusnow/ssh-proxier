#include <Cocoa/Cocoa.h>
#include <Security/Security.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <string.h>
#include "app.hpp"

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

static NSApplication* app = NULL;
static NSStatusBar* statusBar = NULL;
static NSStatusItem* statusItem = NULL;
static NSMenu* menu = NULL;
static NSMenuItem* header = NULL;
static std::vector<NSMenuItem*> menuItems;

void App::init_objcxx() {
    AppDelegate* delegate = [[AppDelegate alloc] init];
    app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    statusBar = [NSStatusBar systemStatusBar];
    statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
    [app activateIgnoringOtherApps:TRUE];

    menu = [[NSMenu alloc] init];
    [menu setAutoenablesItems:FALSE];

    header = [[NSMenuItem alloc] initWithTitle:@"Disconnected"
                                        action:@selector(menuCallback:)
                                 keyEquivalent:@""];
    [header setEnabled:FALSE];
    [header setState:0];

    [menu addItem:header];
    [menu addItem:[NSMenuItem separatorItem]];

    for (size_t i = 0; i < hosts.size(); ++i) {
        auto& host = hosts[i];
        NSMenuItem* menuItem =
            [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:host]
                                       action:@selector(menuCallback:)
                                keyEquivalent:@""];
        [menuItem setEnabled:TRUE];
        [menuItem setState:FALSE];
        [menuItem setRepresentedObject:[NSValue valueWithPointer:(void*)i]];
        menuItems.push_back(menuItem);
        [menu addItem:menuItem];
    }
    [statusItem setMenu:menu];
    update();
    AuthorizationRef authRef;
    AuthorizationFlags flags = 0;
    auto authRet = AuthorizationCreate(NULL, NULL, flags, &authRef);
    printf("authRet %d\n", authRet);
    ;

    CFStringRef a = (CFStringRef) @"sshproxier";
    SCPreferencesRef prefs = SCPreferencesCreate(NULL, a, NULL);
    NSDictionary* services = (NSDictionary*)SCPreferencesGetValue(prefs, kSCPrefNetworkServices);
    printf("prefs: %p\n", prefs);
    for (NSString* key in services) {
        id value = services[key];
        NSLog(@"Value: %@ for key: %@", value, key);
    }
    auto aa = SCPreferencesCopyKeyList(prefs);
    auto sz = CFArrayGetCount(aa);
    for (auto i = 0; i < sz; ++i) {
        CFStringRef entry = (CFStringRef)CFArrayGetValueAtIndex(aa, i);
        const char* entry_str = CFStringGetCStringPtr(entry, kCFStringEncodingMacRoman);
        printf("entry: %s\n", entry_str);
    }
}

int App::loop() {
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
    [until release];

    return 0;
}

int App::update() {
    statusItem.button.title = connected.has_value() ? @"〶" : @"〒";
    header.title = connected.has_value() ? @"Connected" : @"Disconnected";

    size_t connected_id = connected.value_or(-1);

    for (size_t i = 0; i < hosts.size(); ++i) {
        int checked = i == connected_id;
        [menuItems[i] setState:checked];
    }

    return 0;
}
