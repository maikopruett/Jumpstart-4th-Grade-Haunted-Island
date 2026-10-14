#include "platform/print.h"

#import <AppKit/AppKit.h>

#include <cstdint>
#include <cstring>
#include <vector>

bool Print_DIB(const void* dib) {
    const uint8_t* p = (const uint8_t*)dib;
    int32_t w, h;
    memcpy(&w, p + 4, 4);
    memcpy(&h, p + 8, 4);
    if (w <= 0 || h == 0) return false;
    bool bottomUp = h > 0;
    if (h < 0) h = -h;
    const uint8_t* quads = p + 0x28;
    const uint8_t* bits = p + 0x28 + 0x400;
    int stride = (w + 3) & ~3;
    @autoreleasepool {
        NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nullptr
                                                                        pixelsWide:w
                                                                        pixelsHigh:h
                                                                     bitsPerSample:8
                                                                   samplesPerPixel:3
                                                                          hasAlpha:NO
                                                                          isPlanar:NO
                                                                    colorSpaceName:NSDeviceRGBColorSpace
                                                                       bytesPerRow:w * 3
                                                                      bitsPerPixel:24];
        uint8_t* out = [rep bitmapData];
        for (int y = 0; y < h; y++) {
            const uint8_t* src = bits + (bottomUp ? (h - 1 - y) : y) * stride;
            uint8_t* dst = out + y * w * 3;
            for (int x = 0; x < w; x++) {
                const uint8_t* q = quads + src[x] * 4;
                dst[x * 3] = q[2];
                dst[x * 3 + 1] = q[1];
                dst[x * 3 + 2] = q[0];
            }
        }
        NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(w, h)];
        [image addRepresentation:rep];
        NSImageView* view = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, w, h)];
        [view setImage:image];
        [view setImageScaling:NSImageScaleProportionallyUpOrDown];
        NSPrintInfo* info = [[NSPrintInfo sharedPrintInfo] copy];
        [info setHorizontalPagination:NSPrintingPaginationModeFit];
        [info setVerticalPagination:NSPrintingPaginationModeFit];
        [info setHorizontallyCentered:YES];
        [info setVerticallyCentered:YES];
        [info setOrientation:(w > h ? NSPaperOrientationLandscape : NSPaperOrientationPortrait)];
        NSPrintOperation* op = [NSPrintOperation printOperationWithView:view printInfo:info];
        [op setShowsPrintPanel:YES];
        [op setShowsProgressPanel:YES];
        return [op runOperation] ? true : false;
    }
}
