#import "DisplayViewController.h"

#import <stdint.h>

#import "VirtualDisplay.hpp"
#import "DisplayView.h"
#import "MacOSFontTextureAtlas.h"
#import "Shared.h"

@interface DisplayViewController () {
@private
    id<MTLDevice> _gpu;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _renderPipelineState;
    id<MTLTexture> _texture;
    id<MTLBuffer> _renderBuffer;

    uint8_t _frameVersion;
}
@end

@interface DisplayViewController (MTKViewDelegate) <MTKViewDelegate>
@end

@implementation DisplayViewController (MTKViewDelegate)

- (void)drawInMTKView:(MTKView *)view {
//    if (self->_renderBuffer == nil) {
//        return;
//    }

//    const __auto_type buffer = (uint8_t *)self->_renderBuffer.contents;
//    const uint8_t frameVersion = buffer[0];
//    if (frameVersion == self->_frameVersion) {
//        return;
//    }

    const id<MTLCommandBuffer> commandBuffer = [self->_commandQueue commandBuffer];
    MTLRenderPassDescriptor *const renderFrameDescriptor = view.currentRenderPassDescriptor;
    if (renderFrameDescriptor != nil) {
        const id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderFrameDescriptor];
        if (self->_renderPipelineState != nil && self->_renderBuffer != nil && self->_texture != nil) {
            [renderCommandEncoder setRenderPipelineState:self->_renderPipelineState];
            [renderCommandEncoder setVertexBuffer:self->_renderBuffer offset:4 atIndex:0];
            [renderCommandEncoder setFragmentTexture:self->_texture atIndex:0];
            [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                                     vertexStart:0
                                     vertexCount:4
                                   instanceCount:2000];
        }
        [renderCommandEncoder endEncoding];

        [commandBuffer presentDrawable:view.currentDrawable];
    }
    [commandBuffer commit];

//    self->_frameVersion = frameVersion;
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size { /* Do nothing */ }

@end

@implementation DisplayViewController {
@private
    struct VirtualDisplay *_display;

    MacOSFontTextureAtlas *_textureAtlas;
}

- (instancetype)initWithDisplay:(struct VirtualDisplay *const)display {
    self = [super init];
    if (self) {
        self->_display = display;
    }
    return self;
}

- (DisplayView *)displayView {
    return (DisplayView *)self.view;
}

- (void)loadView {
    self.view = [[DisplayView alloc] init];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
    self->_gpu = gpu;
    self->_commandQueue = [gpu newCommandQueue];

    self->_textureAtlas = [[MacOSFontTextureAtlas alloc] initWithDevice:gpu];

    DisplayView *const displayView = [self displayView];
    displayView.device = gpu;
    displayView.delegate = self;
    displayView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);

    auto *const renderMap = (struct RenderMap *)malloc(sizeof(struct RenderMap));
    simd_float4 display_vertices[4] = {
        simd_make_float4(-1.0, 1.0, 0.0, 1.0),
        simd_make_float4(1.0, 1.0, 0.0, 1.0),
        simd_make_float4(-1.0, -1.0, 0.0, 1.0),
        simd_make_float4(1.0, -1.0, 0.0, 1.0)
    };
    memcpy(renderMap->display_vertices, display_vertices, sizeof(renderMap->display_vertices));

    simd_float2 texture_vertices[4] = {
        simd_make_float2(0.0, 0.0),
        simd_make_float2(1.0, 0.0),
        simd_make_float2(0.0, 1.0),
        simd_make_float2(1.0, 1.0)
    };
    memcpy(renderMap->texture_vertices, texture_vertices, sizeof(renderMap->texture_vertices));

    const VirtualDisplay::FrameBuffer framebuffer = self->_display->framebuffer();
    self->_renderBuffer = [gpu newBufferWithBytesNoCopy:static_cast<void*>(framebuffer.data)
                                                 length:framebuffer.size
                                                options:MTLResourceStorageModeShared
                                            deallocator:nil];

    self->_texture = [self->_textureAtlas getTexture];

    const id<MTLLibrary> shadersLibrary = [gpu newDefaultLibrary];
    const id<MTLFunction> vertexShader = [shadersLibrary newFunctionWithName:@"vertex_framebuffer"];
    const id<MTLFunction> fragmentShader = [shadersLibrary newFunctionWithName:@"fragment_framebuffer"];

    auto *const renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    renderPipelineDescriptor.vertexFunction = vertexShader;
    renderPipelineDescriptor.fragmentFunction = fragmentShader;
    renderPipelineDescriptor.colorAttachments[0].pixelFormat = self.displayView.colorPixelFormat;

    NSError *error;
    self->_renderPipelineState = [gpu newRenderPipelineStateWithDescriptor:renderPipelineDescriptor error:&error];
    if (error != nil) {
        NSLog(@"%@", error.localizedDescription);
    }
}

@end
