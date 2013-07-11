#include <QtGlobal>

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#elif __ANDROID__
    #include <GLES/gl.h>
    #include <GLES/glext.h>
#else
    #if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        #include <QtOpenGL>
    #else
        #include <GL/glew.h>
        #include <GL/gl.h>
    #endif
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ggl.h>
#include <glog.h>
#include <gimage.h>
#include <QDebug>

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{

}

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGL()
{
    glewInit();


    ggl_AfterCreatingContext(context());

    ggl_SetCurrentContext(context());


    g_id blendState = ggl_BlendStateCreate();

    ggl_BlendStateSetColorSourceBlend(blendState, GGL_SOURCE_ALPHA);
    ggl_BlendStateSetColorDestinationBlend(blendState, GGL_INVERSE_SOURCE_ALPHA);
    //ggl_BlendStateSetAlphaSourceBlend(blendState, GGL_ONE);
    //ggl_BlendStateSetAlphaDestinationBlend(blendState, GGL_ZERO);
    //ggl_BlendStateSetColorBlendFunction(blendState, GGL_ADD);
    //ggl_BlendStateSetAlphaBlendFunction(blendState, GGL_ADD);

    ggl_SetBlendState(blendState);

    int width, height, comp;
    int result = gimage_parsePng("sky_world.png", &width, &height, &comp);
    qDebug() << result << width << height << comp;

    void *pixels = malloc(width * height * comp);
    gimage_loadPng("sky_world.png", pixels);

    g_id texture = ggl_TextureCreate2D(width, height, false, GGL_RGBA);
    ggl_TextureSetData(texture, pixels);

    free(pixels);

    ggl_SetTexture(0, texture);
    ggl_SetTexture(1, texture);

    g_id effect = ggl_EffectCreate(
        "#version 120\n"
        "\n"
        "attribute vec4 POSITION0;\n"
        "attribute vec2 TEXCOORD0;\n"
        "attribute vec4 COLOR0;\n"
        "\n"
        "uniform float scale;"
        "\n"
        "varying vec4 color;"
        "varying vec2 texCoord;"
        "\n"
        "void main()\n"
        "{\n"
        "    color = COLOR0;\n"
        "    texCoord = TEXCOORD0;\n"
        "    gl_Position = POSITION0 * vec4(scale, scale, 1, 1);\n"
        "}\n",

        "#version 120\n"
        "\n"
        "uniform sampler2D tex1, tex2;"
        "\n"
        "varying vec4 color;"
        "varying vec2 texCoord;"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = color * texture2D(tex1, texCoord * 0.1) * texture2D(tex2, texCoord * 0.2);\n"
        "}\n");

    ggl_EffectSetParameter1f(effect, "scale", 1.5f);
    ggl_EffectSetParameter1i(effect, "tex1", 0);
    ggl_EffectSetParameter1i(effect, "tex2", 1);



    glog_e("%d", effect);
    ggl_SetEffect(effect);
}


void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
}

#pragma pack(1)
struct VertexPositionColor
{
    float x, y, z;
    float u, v;
    unsigned char r, g, b, a;
};

void GLWidget::paintGL()
{
    glClearColor(0.9, 0.6, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    VertexPositionColor v[] = {
        { 0.0f,  0.5f, 0.0f, 0.0f, 0.0f,   0, 255,   0, 0},
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 200, 255, 200, 150},
        { 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 200, 200, 255, 255},
    };

    ggl_VertexDeclaration vertexDeclaration;
    vertexDeclaration.vertexElementCount = 3;

    vertexDeclaration.vertexElements[0].offset = 0;
    vertexDeclaration.vertexElements[0].vertexElementFormat = GGL_FLOAT_3;
    vertexDeclaration.vertexElements[0].vertexElementUsage = GGL_POSITION;
    vertexDeclaration.vertexElements[0].usageIndex = 0;

    vertexDeclaration.vertexElements[1].offset = 12;
    vertexDeclaration.vertexElements[1].vertexElementFormat = GGL_FLOAT_2;
    vertexDeclaration.vertexElements[1].vertexElementUsage = GGL_TEXCOORD;
    vertexDeclaration.vertexElements[1].usageIndex = 0;

    vertexDeclaration.vertexElements[2].offset = 20;
    vertexDeclaration.vertexElements[2].vertexElementFormat = GGL_UNSIGNED_BYTE_4;
    vertexDeclaration.vertexElements[2].vertexElementUsage = GGL_COLOR;
    vertexDeclaration.vertexElements[2].usageIndex = 0;

    vertexDeclaration.vertexStride = 24;

    ggl_DrawUserPrimitives(GGL_TRIANGLE_LIST, v, 0, 3, &vertexDeclaration);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setCentralWidget(new GLWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
