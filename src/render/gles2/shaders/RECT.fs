﻿//
//  VideoSource.fs
//  CMGL
//
//  Created by 熊朝伟 on 2020/1/10.
//  Copyright © 2020 熊朝伟. All rights reserved.
//
// 行首和行位的特殊标记用于告诉clang编译器，此文件内容为字符串，此语法为C++11标准。

R"SHADER(
precision highp float;

varying vec2 clipCoord;
varying vec2 coordinate;

uniform float alpha;
uniform mat4 colorConversion;
uniform vec4 color;

void main() {
    if (clipCoord.x < 0.0 || clipCoord.x > 1.0 || clipCoord.y < 0.0 || clipCoord.y > 1.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        gl_FragColor = clamp(colorConversion * color * alpha, 0.0, 1.0);
    }
}

//)SHADER"
