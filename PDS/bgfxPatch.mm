#import <Foundation/Foundation.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Cocoa/Cocoa.h>

extern "C" {
void *cbSetupMetalLayer(void *wnd);
}

void *cbSetupMetalLayer(void *wnd) {
  NSWindow *window = (__bridge NSWindow*)wnd;
  NSView *contentView = [window contentView];
  [contentView setWantsLayer:YES];
  CAMetalLayer *res = [CAMetalLayer layer];
  [contentView setLayer:res];
  return (__bridge void*)res;
}
