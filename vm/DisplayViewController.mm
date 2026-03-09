#import "DisplayViewController.h"

#import "Display.hpp"
#import "DisplayView.h"
#import "DisplayObserverAdapter.h"
#import "MacOSFontTextureAtlas.h"
#import "Shared.h"

@interface DisplayViewController () {
@private
    id<MTLDevice> _gpu;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _renderPipelineState;
    id<MTLTexture> _texture;
    id<MTLBuffer> _renderBuffer;
}
@end

@interface DisplayViewController (DisplayObserver) <DisplayObserver>
@end

@implementation DisplayViewController (DisplayObserver)

- (void)displayDidChangeBufferState:(struct Display *const)display {
    uint8_t *const framebuffer = display->get_framebuffer();
    self->_renderBuffer = [self->_gpu newBufferWithBytes:framebuffer length:2000 options:MTLResourceStorageModeShared];
}

@end

@interface DisplayViewController (MTKViewDelegate) <MTKViewDelegate>
@end

@implementation DisplayViewController (MTKViewDelegate)

- (void)drawInMTKView:(MTKView *)view {
    const id<MTLCommandBuffer> commandBuffer = [self->_commandQueue commandBuffer];
    MTLRenderPassDescriptor *const renderFrameDescriptor = [view currentRenderPassDescriptor];
    if (renderFrameDescriptor != nil) {
        const id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderFrameDescriptor];
        if (self->_renderPipelineState != nil && self->_renderBuffer != nil && self->_texture != nil) {
            [renderCommandEncoder setRenderPipelineState:self->_renderPipelineState];
            [renderCommandEncoder setVertexBuffer:self->_renderBuffer offset:0 atIndex:0];
            [renderCommandEncoder setFragmentTexture:self->_texture atIndex:0];
//            [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
//                                     vertexStart:0
//                                     vertexCount:[self->_renderBuffer length]];
            [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                                     vertexStart:0
                                     vertexCount:4
                                   instanceCount:2000];
        }
        [renderCommandEncoder endEncoding];

        [commandBuffer presentDrawable:[view currentDrawable]];
    }
    [commandBuffer commit];
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size { /* Do nothing */ }

@end

@implementation DisplayViewController {
@private
    struct Display *_display;
    struct DisplayObserverAdapter *_Nullable _observerAdapter;

    MacOSFontTextureAtlas *_textureAtlas;
}

- (instancetype)initWithDisplay:(struct Display *const)display {
    self = [super init];
    if (self) {
        self->_display = display;
    }
    return self;
}

- (void)dealloc {
    if (self->_observerAdapter != nullptr) {
        self->_display->remove_observer(self->_observerAdapter);
        delete self->_observerAdapter;
    }

    [super dealloc];
}

- (DisplayView *)displayView {
    return (DisplayView *)[self view];
}

- (void)loadView {
    [self setView:[[DisplayView alloc] init]];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
    self->_gpu = gpu;
    self->_commandQueue = [gpu newCommandQueue];

    self->_textureAtlas = [[MacOSFontTextureAtlas alloc] initWithDevice:gpu];

    DisplayView *const displayView = [self displayView];
    [displayView setDevice:gpu];
    [displayView setDelegate:self];
    [displayView setClearColor:MTLClearColorMake(0.0, 0.0, 0.0, 1.0)];

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

//    self->_renderBuffer = [gpu newBufferWithBytes:renderMap
//                                           length:sizeof(struct RenderMap)
//                                          options:MTLResourceStorageModeShared];

    self->_texture = [self->_textureAtlas getTexture];

    const id<MTLLibrary> shadersLibrary = [gpu newDefaultLibrary];
//    const id<MTLFunction> vertexShader = [shadersLibrary newFunctionWithName:@"vertex_shader"];
//    const id<MTLFunction> fragmentShader = [shadersLibrary newFunctionWithName:@"fragment_shader"];
    const id<MTLFunction> vertexShader = [shadersLibrary newFunctionWithName:@"vertex_framebuffer"];
    const id<MTLFunction> fragmentShader = [shadersLibrary newFunctionWithName:@"fragment_framebuffer"];

    auto *const renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    [renderPipelineDescriptor setVertexFunction:vertexShader];
    [renderPipelineDescriptor setFragmentFunction:fragmentShader];
    renderPipelineDescriptor.colorAttachments[0].pixelFormat = self.displayView.colorPixelFormat;

    NSError *error;
    self->_renderPipelineState = [gpu newRenderPipelineStateWithDescriptor:renderPipelineDescriptor error:&error];
    if (error != nil) {
        NSLog(@"%@", [error localizedDescription]);
    }

    self->_observerAdapter = new struct DisplayObserverAdapter(self);
    self->_display->add_observer(self->_observerAdapter);
}

- (void)viewDidAppear {
    [super viewDidAppear];

    DisplayView *const displayView = [self displayView];
    [[displayView window] makeFirstResponder:displayView];
}

- (void)viewDidDisappear {
    [super viewDidDisappear];

    [[self displayView] resignFirstResponder];
}

- (void)keyDown:(NSEvent *)event {
    [self->displayInputDelegate displayDidCatchInputWithEvent:event];
}

@end
