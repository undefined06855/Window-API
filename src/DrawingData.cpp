#include "DrawingData.hpp"

float quad[] = {
    -1.f, -1.f,
    1.f, -1.f,
    1.f, 1.f,
    -1.f, 1.f
};

int indices[] = { 0, 1, 2, 2, 3, 0 };

const char* vertexShader = R"(
#version 150

in vec2 position;

void main() {
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
)";

const char* fragmentShader = R"(
#version 150

uniform sampler2D tex;
uniform vec2 winSize;

out vec4 fragColor;

void main() {
    vec2 uv = gl_FragCoord.xy / winSize;
    uv.y = 1.f - uv.y;
    fragColor = texture(tex, uv);
    // fragColor = gl_FragCoord;
}
)";

DrawingData::DrawingData() {
    this->setup = false;

    glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->ebo);

    // setup vao with quad data
    GLint prevVao;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
    glBindVertexArray(this->vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(prevVao);

    // shader shit
    auto vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShader, nullptr);
    glCompileShader(vertex);

    auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShader, nullptr);
    glCompileShader(fragment);

    this->shader = glCreateProgram();
    glAttachShader(this->shader, vertex);
    glAttachShader(this->shader, fragment);
    glBindAttribLocation(this->shader, 0, "position");
    glLinkProgram(this->shader);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    this->textureLocation = glGetUniformLocation(this->shader, "tex");
    this->winSizeLocation = glGetUniformLocation(this->shader, "winSize");

    this->setup = true;
}

DrawingData::~DrawingData() {
    if (!this->setup) return;

    glDeleteShader(this->shader);
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
}
