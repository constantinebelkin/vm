#include <metal_stdlib>

#include "Shared.h"

using namespace metal;

struct VertexFramebufferMap {
    float4 screen_position [[position]];
    float2 texture_position;
    uint8_t char_code;
};

vertex VertexFramebufferMap vertex_framebuffer(
    uint vertex_id [[vertex_id]],
    uint instance_id [[instance_id]],
    constant uint8_t *framebuffer [[buffer(0)]]
) {
    // Считаем позицию символа в сетке 80x25
    float x_grid = instance_id % 80;
    float y_grid = instance_id / 80;

    // Координаты одного квадрата (0..1)
    float2 quadCoords[] = { {0,0}, {1,0}, {0,1}, {1,1} };
    float2 localPos = quadCoords[vertex_id];

    // Размеры ячейки: 1/80 по горизонтали, 1/25 по вертикали (от ширины экрана)
    float x = ((x_grid + localPos.x) / 80.0) * 2.0 - 1.0;
    float y = ((y_grid + localPos.y) / 25.0) * -2.0 + 1.0;

    return VertexFramebufferMap {
        .screen_position = float4(x, y, 0.0, 1.0),
        .texture_position = localPos,
        .char_code = framebuffer[instance_id]
    };
}

fragment float4 fragment_framebuffer(
    VertexFramebufferMap vertex_frmebuffer_map [[stage_in]],
    texture2d<float> texture [[texture(0)]]
) {
    float gridCount = 16.0;

    // Получаем колонку и строку символа
    uint charCol = vertex_frmebuffer_map.char_code % 16;
    uint charRow = vertex_frmebuffer_map.char_code / 16;

    // Считаем UV-координату левого верхнего угла нужной буквы в атласе
    float2 originUV = float2(float(charCol) / gridCount, float(charRow) / gridCount);

    // Считаем смещение внутри этой буквы (размер буквы в UV = 1.0/16.0)
    float2 offsetUV = vertex_frmebuffer_map.texture_position / gridCount;

    // Итоговая координата
    float2 finalUV = originUV + offsetUV;

    // Читаем маску. Используем nearest, чтобы исключить замыливание
    sampler s(filter::nearest);
    float mask = texture.sample(s, finalUV).r;

    return mask < 0.05
        ? float4(0.0, 0.0, 0.0, 1.0)
        : float4(1.0, 1.0, 1.0, 1.0);
}
