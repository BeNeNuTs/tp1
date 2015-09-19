#include "terrain.h"

#include <QOpenGLBuffer>
#include <QVector3D>

static const char *vertexShaderSource =
        "attribute highp vec4 posAttr;\n"
        "attribute lowp vec4 colAttr;\n"
        "varying lowp vec4 col;\n"
        "uniform highp mat4 matrix;\n"
        "void main() {\n"
        "   col = colAttr;\n"
        "   gl_Position = matrix * posAttr;\n"
        "}\n";

static const char *fragmentShaderSource =
        "varying lowp vec4 col;\n"
        "void main() {\n"
        "   gl_FragColor = col;\n"
        "}\n";

Terrain::Terrain(QString heightmap) : m_program(0), m_frame(0)
{
    openImage(heightmap);

    createTerrain();
}

GLuint Terrain::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void Terrain::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
}

void Terrain::openImage(QString str){
    heightmap.load(str);

    hauteur = new int[heightmap.width() * heightmap.height()];

    int index = 0;
    for(int i = 0  ; i < heightmap.height() ; i++){
        for(int j = 0 ; j < heightmap.width() ; j++){
            QRgb pixel = heightmap.pixel(i,j);
            hauteur[index++] = qRed(pixel);
        }
    }
}

void Terrain::createTerrain(){
    vertices = new GLfloat*[heightmap.height()];

    float gap = 0.5f;
    float posX = -(gap * heightmap.width()/2.f);
    float posY = -20.f;
    float posZ = gap * heightmap.height()/2.f;

    int indexHeightmap = 0;

    for(int line = 0 ; line < heightmap.height() - 1 ; line++){
        int index = 0;

        vertices[line] = new GLfloat[heightmap.width()*2*nbCoord];
        for(int i = 0 ; i < heightmap.width() ; i++){
            for(int j = 0 ; j < 2 ; j++){
                vertices[line][index++] = posX + gap * i;
                vertices[line][index++] = posY + hauteur[indexHeightmap++] / 100.f;
                vertices[line][index++] = posZ - gap * j;
            }
        }

        posZ -= gap;
    }
}

void Terrain::displayTerrain(){


    for(int line = 0 ; line < heightmap.height() - 1 ; line++){
        glVertexAttribPointer(m_posAttr, nbCoord, GL_FLOAT, GL_FALSE, 0, vertices[line]);

        glEnableVertexAttribArray(0);

        glDrawArrays(GL_LINE_LOOP, 0, heightmap.width()*2);

        glDisableVertexAttribArray(0);
    }
}

void Terrain::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 16.0f/9.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -100);
    matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

    displayTerrain();

    m_program->release();

    ++m_frame;
}

/* VBO doesn't work
 * ////////////////////////////////////////////////////////////////
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf(QOpenGLBuffer::IndexBuffer);

    arrayBuf.create();
    indexBuf.create();

    ////////////////////////////////////////////////////////////////

    QVector3D vertices[] = {
        QVector3D(0.0f, 0.707f,  1.0f),
        QVector3D(-0.5f, -0.5f,  1.0f),
        QVector3D(0.5f,  -0.5f,  1.0f)
    };

    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, 4 * sizeof(QVector3D));

    GLushort indices[] = {
        0, 1, 2
    };

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 3 * sizeof(GLushort));

    ////////////////////////////////////////////////////////////////

    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = m_program->attributeLocation("a_position");
    m_program->enableAttributeArray(vertexLocation);
    m_program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);

    ////////////////////////////////////////////////////////////////
 * */


