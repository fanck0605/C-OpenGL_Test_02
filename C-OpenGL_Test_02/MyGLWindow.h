#pragma once
#include<vector>
#include<random>
#include<chrono>
#include<qdebug.h>
#include<qopenglfunctions_4_5_core.h>
#include<qopenglwidget.h>
#include"SimpleTextureBox.h"
#include"Simple3DBox.h"
#include"Camera.h"
#include"Model.h"

class MyGLWindow : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    MyGLWindow(QWidget* parent = nullptr);
    ~MyGLWindow();

    void initializeGL()override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mouseMoveEvent(QMouseEvent* event)override;
    void keyPressEvent(QKeyEvent* event)override;
    void keyReleaseEvent(QKeyEvent* event)override;
private:
    Camera mainCamera{ 800.0f,800.0f };
    std::chrono::steady_clock::time_point lastTimePoint;
    std::chrono::steady_clock::time_point programBeginPoint;
    //code here
    struct TriangleStripBox
    {
        unsigned int vao, vbo, ebo;
        constexpr static std::array<float, 24> vertices
        {
            -1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            -1.0,  1.0, -1.0,
            -1.0,  1.0,  1.0,
             1.0, -1.0, -1.0,
             1.0, -1.0,  1.0,
             1.0,  1.0, -1.0,
             1.0,  1.0,  1.0,
        };

        constexpr static std::array<unsigned int, 17> indices
        {
            0, 1, 2, 3, 6, 7, 4, 5,
            0xFFFF,
            2, 6, 0, 4, 1, 5, 3, 7
        };
    };

    TriangleStripBox tsBox;
    QOpenGLShaderProgram singleColor;

    struct AntiAliasing
    {
        unsigned int fbo;
        unsigned int depthTex, colorTex;
    };

    AntiAliasing antiAliasing;

    struct AAOutput
    {
        unsigned int vao, vbo, fbo;
        unsigned int colorTex;

        constexpr static std::array<float, 20> vertices
        {
            -1,-1,0,0,0,
            1,-1,0,1,0,
            -1,1,0,0,1,
            1,1,0,1,1
        };
    };

    AAOutput outFrame;

    QOpenGLShaderProgram outShader;

    struct BlinnPhong
    {
        unsigned int vao, vbo;
        std::unique_ptr<QOpenGLTexture> tex;
        QOpenGLShaderProgram shader;

        constexpr static std::array<float, 32> vertices
        {
            //position             //normal           //texCoords
           -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
            10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   10.0f, 10.0f,
           -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
            10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   10.0f,  0.0f
        };
    };

    BlinnPhong bp;
};
