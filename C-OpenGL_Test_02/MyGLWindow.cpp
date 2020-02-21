#include "MyGLWindow.h"

#include<QKeyEvent>
#include<cmath>

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using glm::mat4;
using glm::vec3;
using glm::translate;
using glm::scale;
using glm::value_ptr;
using glm::rotate;
using glm::radians;
using glm::normalize;
using glm::perspective;
using glm::lookAt;

MyGLWindow::MyGLWindow(QWidget* parent)
    : QOpenGLWidget(parent)
{
    resize(800, 800);
    //setWindowFlags(Qt::FramelessWindowHint);
    setCursor(Qt::BlankCursor);
    setMouseTracking(true);
}

MyGLWindow::~MyGLWindow()
{
}

void MyGLWindow::initializeGL()
{
    programBeginPoint = lastTimePoint = std::chrono::steady_clock::now();
    std::default_random_engine dre(std::chrono::system_clock::now().time_since_epoch().count());

    QOpenGLFunctions_4_5_Core::initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    testModel.loadModel("./models/nanosuit/nanosuit.obj");
    testModel.init();
    modelShader.create();
    modelShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/model.vert");
    modelShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/model.frag");
    modelShader.link();

    //light box
    lightBoxShader.create();
    lightBoxShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/boxShader.vert");
    lightBoxShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/lightBox.frag");
    lightBoxShader.link();
    lightBox.init();
    lightBox.resetScaleMat(scale(mat4(1.0f), vec3{ 0.2f }));
    std::normal_distribution<float> lightPosDistribution{ 0.0f, 2.0f };
    for (int i = 0; i < 4; ++i)
    {
        pointLightColor.push_back({ 1.0f,1.0f,1.0f });
    }
    lightPos.push_back(vec3(0.0f, -1.7f, 0.4f));
    lightPos.push_back(vec3(-0.7f, -0.8f, 0.0f));
    lightPos.push_back(vec3(0.0f, 0.4f, 0.7f));
    lightPos.push_back(vec3(-0.4f, 1.7f, 0.0f));

    singleColorShader.create();
    singleColorShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/singleColor.vert");
    singleColorShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/singleColor.frag");
    singleColorShader.link();

    vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    glGenVertexArrays(1, &vegetationVAO);
    glGenBuffers(1, &vegetationVBO);
    glGenBuffers(1, &vegetationEBO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glBufferData(GL_ARRAY_BUFFER, vegetationVertices.size() * sizeof(float), vegetationVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vegetationEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vegetationIndices.size() * sizeof(unsigned int), vegetationIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(vegetationVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vegetationEBO);
    glBindVertexArray(0);

    vegetationShader.create();
    vegetationShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/textureSquare.vert");
    vegetationShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/textureSquare.frag");
    vegetationShader.link();

    vegetationTex = new QOpenGLTexture(QImage("./images/grass.png").mirrored());
    vegetationTex->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void MyGLWindow::paintGL()
{
    boxCamera.caculateCamera();
    auto currentTime = std::chrono::steady_clock::now();
    auto passedDuration = duration_cast<duration<float>>(currentTime - lastTimePoint).count();
    auto timeFromBeginPoint = duration_cast<duration<float>>(currentTime - programBeginPoint).count();
    lastTimePoint = currentTime;

    //light box
    vec3 lightColor{ 1.0f, 1.0f, 1.0f };
    lightColor.r = 0.5f + 0.5f * sin(timeFromBeginPoint);
    lightColor.g = 0.5f + 0.5f * cos(timeFromBeginPoint);
    lightColor.b = 1.0f - lightColor.r;
    vec3 diffuseColor = lightColor * vec3{ 0.7f };
    vec3 ambientColor = diffuseColor * vec3{ 0.3f };

    lightBoxShader.bind();
    lightBox.bind();
    glUniform3fv(lightBoxShader.uniformLocation("lightColor"), 1, value_ptr(lightColor));
    for (int i = 0; i < lightPos.size(); ++i)
    {
        pointLightColor[i].r = 0.5f + 0.5f * sin(0.5f + timeFromBeginPoint + 0.5f * i);
        pointLightColor[i].g = 0.5f + 0.5f * cos(0.5f + timeFromBeginPoint + 0.5f * i);
        pointLightColor[i].b = 1.0f - pointLightColor[i].r;
        glUniform3fv(lightBoxShader.uniformLocation("lightColor"), 1, value_ptr(pointLightColor[i]));

        mat4 lightModel = translate(mat4{ 1.0f }, lightPos[i]) * lightBox.scaleMat;
        glUniformMatrix4fv(lightBoxShader.uniformLocation("MVP"), 1, GL_FALSE, value_ptr(boxCamera.viewProjectionMat() * lightModel));
        lightBox.draw();
    }

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    //model
    modelShader.bind();
    vec3 position;
    position.x = 5.0f * sin(timeFromBeginPoint);
    position.z = 5.0f * cos(timeFromBeginPoint);
    mat4 translateMat = translate(mat4{ 1.0f }, vec3(0.0f, -1.75f, 0.0f));
    mat4 scaleMat = scale(mat4{ 1.0f }, vec3(0.2f, 0.2f, 0.2f));
    mat4 modelMat = translateMat * scaleMat;

    setLightVariableForShader(ambientColor, diffuseColor);
    glUniform1f(modelShader.uniformLocation("material.shininess"), 32.0f);
    glUniformMatrix4fv(modelShader.uniformLocation("MVP"), 1, GL_FALSE, value_ptr(boxCamera.viewProjectionMat() * modelMat));
    glUniformMatrix4fv(modelShader.uniformLocation("modelMat"), 1, GL_FALSE, value_ptr(modelMat));
    glUniform3fv(modelShader.uniformLocation("viewPos"), 1, value_ptr(boxCamera.position));
    testModel.draw(&modelShader);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    singleColorShader.bind();
    scaleMat = scale(scaleMat, vec3(1.01f, 1.01f, 1.01f));
    modelMat = translateMat * scaleMat;
    glUniformMatrix4fv(singleColorShader.uniformLocation("MVP"), 1, GL_FALSE, value_ptr(boxCamera.viewProjectionMat() * modelMat));
    glUniform3fv(singleColorShader.uniformLocation("color"), 1, value_ptr(pointLightColor[0]));
    testModel.draw(&singleColorShader);
    glEnable(GL_DEPTH_TEST);
    glStencilMask(0xFF); //if stencil mask set to 0x00, glClean(GL_STENCIL_BUFFER_BIT) will not work.
    glClear(GL_STENCIL_BUFFER_BIT);

    vegetationShader.bind();
    glUniformMatrix4fv(vegetationShader.uniformLocation("MVP"), 1, GL_FALSE, value_ptr(boxCamera.viewProjectionMat()));
    glActiveTexture(GL_TEXTURE0);
    vegetationTex->bind(GL_TEXTURE_2D);
    glUniform1i(vegetationShader.uniformLocation("tex"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glBindVertexArray(vegetationVAO);
    std::sort(vegetation.begin(), vegetation.end(), [this](auto a,auto b)
        {
            float distanceA = glm::length(a - boxCamera.position);
            float distanceB = glm::length(b - boxCamera.position);
            return distanceA < distanceB;
        });
    for (auto& i : vegetation)
    {
        mat4 transFromVegetation = translate(mat4{ 1.0f }, i);
        mat4 MVP = boxCamera.viewProjectionMat() * transFromVegetation;
        glUniformMatrix4fv(vegetationShader.uniformLocation("MVP"), 1, GL_FALSE, value_ptr(MVP));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glDisable(GL_BLEND);

    update();
}

void MyGLWindow::resizeGL(int w, int h)
{
    boxCamera.resizeCamera(w, h);
}

void MyGLWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (!rect().contains(event->pos()))
        return;
    if (firstMouse)
    {
        QCursor myCursor = cursor();
        myCursor.setPos(mapToGlobal({ width() / 2, height() / 2 }));
        setCursor(myCursor);
        firstMouse = false;
        return;
    }
    if (secondMouse)
    {
        secondMouse = false;
        return;
    }
    float xAxisMove = event->x() - width() / 2;
    float yAxisMove = event->y() - height() / 2;
    boxCamera.processMouseMovement(xAxisMove, yAxisMove);
    QCursor myCursor = cursor();
    myCursor.setPos(mapToGlobal({ width() / 2, height() / 2 }));
    setCursor(myCursor);
}

void MyGLWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    if (event->key() == Qt::Key_W)
        boxCamera.setKeyW(true);
    if (event->key() == Qt::Key_S)
        boxCamera.setKeyS(true);
    if (event->key() == Qt::Key_A)
        boxCamera.setKeyA(true);
    if (event->key() == Qt::Key_D)
        boxCamera.setKeyD(true);
}

void MyGLWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_W)
        boxCamera.setKeyW(false);
    if (event->key() == Qt::Key_S)
        boxCamera.setKeyS(false);
    if (event->key() == Qt::Key_A)
        boxCamera.setKeyA(false);
    if (event->key() == Qt::Key_D)
        boxCamera.setKeyD(false);
}

void MyGLWindow::setLightVariableForShader(vec3 ambientColor, vec3 diffuseColor)
{
    vec3 spotLightColor{ 1.0f, 1.0f, 1.0f };
    vec3 spotDiffuseColor = spotLightColor * vec3{ 0.7f };
    vec3 spotAmbientColor = spotDiffuseColor * vec3{ 0.3f };

    glUniform3f(modelShader.uniformLocation("dirlight.direction"), -0.2f, -1.0f, -0.3f);
    glUniform3fv(modelShader.uniformLocation("dirlight.ambient"), 1, value_ptr(ambientColor));
    glUniform3fv(modelShader.uniformLocation("dirlight.diffuse"), 1, value_ptr(diffuseColor));
    glUniform3f(modelShader.uniformLocation("dirlight.specular"), 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 4; ++i)
    {
        vec3 pointLightDiffuseColor = pointLightColor[i] * vec3{ 0.7f };
        vec3 pointLightAmbientColor = pointLightDiffuseColor * vec3{ 0.1f };

        QString lightName = QString::fromStdString("pointlights[" + std::to_string(i) + "].");
        glUniform3fv(modelShader.uniformLocation(lightName + "position"), 1, value_ptr(lightPos[i]));
        glUniform1f(modelShader.uniformLocation(lightName + "constant"), 1.0f);
        glUniform1f(modelShader.uniformLocation(lightName + "linear"), 0.09f);
        glUniform1f(modelShader.uniformLocation(lightName + "quadratic"), 0.032f);
        glUniform3fv(modelShader.uniformLocation(lightName + "ambient"), 1, value_ptr(pointLightAmbientColor));
        glUniform3fv(modelShader.uniformLocation(lightName + "diffuse"), 1, value_ptr(pointLightDiffuseColor));
        glUniform3f(modelShader.uniformLocation(lightName + "specular"), 1.0f, 1.0f, 1.0f);
    }

    glUniform3fv(modelShader.uniformLocation("spotlight.position"), 1, value_ptr(boxCamera.position));
    glUniform3fv(modelShader.uniformLocation("spotlight.direction"), 1, value_ptr(boxCamera.front));
    glUniform1f(modelShader.uniformLocation("spotlight.cutOff"), cosf(radians(12.5f)));
    glUniform1f(modelShader.uniformLocation("spotlight.outerCutOff"), cosf(radians(17.5f)));
    glUniform3fv(modelShader.uniformLocation("spotlight.ambient"), 1, value_ptr(spotAmbientColor));
    glUniform3fv(modelShader.uniformLocation("spotlight.diffuse"), 1, value_ptr(spotDiffuseColor));
    glUniform3f(modelShader.uniformLocation("spotlight.specular"), 1.0f, 1.0f, 1.0f);
    glUniform1f(modelShader.uniformLocation("spotlight.constant"), 1.0f);
    glUniform1f(modelShader.uniformLocation("spotlight.linear"), 0.09f);
    glUniform1f(modelShader.uniformLocation("spotlight.quadratic"), 0.032f);
}
