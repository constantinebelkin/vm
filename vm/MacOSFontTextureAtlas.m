#import "MacOSFontTextureAtlas.h"

#import <CoreGraphics/CGBitmapContext.h>
#import <CoreGraphics/CGImage.h>
#import <CoreText/CTFont.h>
#import <CoreText/CTLine.h>
#import <CoreText/CTStringAttributes.h>
#import <Metal/MTLDevice.h>
#import <Metal/MTLTexture.h>

#include <stdint.h>
#include <stdlib.h>

#define CHAR_WIDTH 16.0
#define CHAR_HEIGHT 32.0

#define TEXTURE_WIDTH CHAR_WIDTH*16.0
#define TEXTURE_HEIGHT CHAR_HEIGHT*16.0

#define TEXTURE_BUFFER_SIZE TEXTURE_WIDTH*TEXTURE_HEIGHT

@implementation MacOSFontTextureAtlas {
    id<MTLDevice> _device;
    _Nullable id<MTLTexture> _texture;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device {
    self = [super init];
    if (self != nil) {
        self->_device = device;
    }
    return self;
}

- (id<MTLTexture>)getTexture {
    if (self->_texture != nil) {
        return self->_texture;
    }

    @synchronized (self) {
        if (self->_texture != nil) {
            return self->_texture;
        }

        self->_texture = [self loadTexture];

        return self->_texture;
    }
}

- (id<MTLTexture>)loadTexture {
    uint8_t *textureBuffer = (uint8_t *)[[[NSData alloc] initWithContentsOfFile:@"texture_atlas.bin"] bytes];
    if (textureBuffer == nil) {
        textureBuffer = [self createTexture];
    }

    MTLTextureDescriptor *const textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                                                                                       width:TEXTURE_WIDTH
                                                                                                      height:TEXTURE_HEIGHT
                                                                                                   mipmapped:NO];
    [textureDescriptor setStorageMode:MTLStorageModeShared];

    const id<MTLTexture> texture = [self->_device newTextureWithDescriptor:textureDescriptor];
    [texture replaceRegion:MTLRegionMake2D(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT)
               mipmapLevel:0
                 withBytes:textureBuffer
               bytesPerRow:TEXTURE_WIDTH];

    return texture;
}

- (uint8_t *)createTexture {
    uint8_t *textureBuffer = calloc(TEXTURE_BUFFER_SIZE, sizeof(uint8_t));
    struct CGContext *bitmapContext = CGBitmapContextCreate(
        textureBuffer,
        TEXTURE_WIDTH,
        TEXTURE_HEIGHT,
        8,
        TEXTURE_WIDTH,
        NULL,
        (CGBitmapInfo)kCGImageAlphaOnly
    );

    CGContextScaleCTM(bitmapContext, 2.0, 2.0);

    const CTFontRef font = CTFontCreateWithName(CFSTR("Menlo"), 13.0, NULL);
    CGFloat descent = CTFontGetDescent(font);

    NSCharacterSet *const specialCharacters = [NSCharacterSet controlCharacterSet];
    uint16_t index;
    for (index = 0; index < 256; index++) {
        NSString *_Nullable character = [[NSString alloc] initWithBytes:&index
                                                                 length:1
                                                               encoding:NSWindowsCP1251StringEncoding];

        character = [character stringByTrimmingCharactersInSet:specialCharacters];

        if ([character length] == 0) {
            character = @" ";
        } else {
            NSLog(@"%@", character);
        }

        __auto_type *const attributedString = [[NSAttributedString alloc] initWithString:character attributes:@{
            (id)kCTFontAttributeName: (__bridge id)font,
            (id)kCTForegroundColorFromContextAttributeName: @YES
        }];

        const CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attributedString);
        CGContextSetGrayFillColor(bitmapContext, 1.0, 1.0);
        CGContextSetTextPosition(
            bitmapContext,
            ((index % 16) * 8),
            ((15 - (index / 16)) * 16) + descent
        );

        CTLineDraw(line, bitmapContext);
        CFRelease(line);
    }

    CFRelease(font);
    CGContextRelease(bitmapContext);

    __auto_type *const data = [[NSData alloc] initWithBytes:textureBuffer length:TEXTURE_BUFFER_SIZE];
    [data writeToFile:@"texture_atlas.bin" atomically:YES];

    return textureBuffer;
}

@end
