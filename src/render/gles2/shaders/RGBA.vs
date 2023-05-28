//
//  VideoSource.vs
//  CMGL
//
//  Created by 熊朝伟 on 2020/1/10.
//  Copyright © 2020 熊朝伟. All rights reserved.
//
// 行首和行位的特殊标记用于告诉clang编译器，此文件内容为字符串，此语法为C++11标准。

R"SHADER(

attribute vec2 position;

uniform mat4 globalMatrix;
uniform mat4 localMatrix;
uniform mat4 clipMatrix;
uniform vec2 size;

varying vec2 clipCoord;
varying vec2 coordinate;

void main() {
    coordinate = position;
    clipCoord = (clipMatrix * vec4(position, 0.0, 1.0)).xy;
    gl_Position = globalMatrix * localMatrix * vec4(position * size, 0.0, 1.0);
}

//)SHADER"
