#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <math.h>

static GLuint shaderProgram;
static GLint timeLocation;

static GLfloat time = 0.0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glUseProgram(shaderProgram);
    glUniform1f(timeLocation, time);

    glBegin(GL_QUADS);
        glVertex2f(-0.5, -0.5);
        glVertex2f(0.5, -0.5);
        glVertex2f(0.5, 0.5);
        glVertex2f(-0.5, 0.5);
    glEnd();

    glutSwapBuffers();
}

void idle() {
    time += 0.01;
    glutPostRedisplay();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);

    shaderProgram = glCreateProgram();

    const char* vertexShaderSource =
        "#version 120\n"
        "void main() {\n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 120\n"
        "uniform float time;\n"
        "void main() {\n"
        "    float red = abs(sin(time));\n"
        "    float green = abs(cos(time));\n"
        "    gl_FragColor = vec4(red, green, 0.0, 1.0);\n"
        "}\n";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glAttachShader(shaderProgram, vertexShader);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    timeLocation = glGetUniformLocation(shaderProgram, "time");
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Shader Animation");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
    return 0;
}
