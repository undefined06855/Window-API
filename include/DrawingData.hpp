#pragma once

struct DrawingData {
    DrawingData();
    ~DrawingData();

    DrawingData(const DrawingData&) = delete;
    DrawingData& operator=(const DrawingData&) = delete;

    bool setup;
    GLuint vao, vbo, ebo, shader;
    GLint textureLocation, winSizeLocation;
};
