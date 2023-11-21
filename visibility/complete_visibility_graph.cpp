#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "../shaders.h"
#include <math.h>
#include <stdio.h>
#include <vector>

const int width = 800, height = 800;
float robotX = 0.0f, robotY = 0.0f, step = 0.005f;
const float sensorRadius = 3.0f;
int robotID;
int count = 0;

class Point
{
private:
    /* data */
public:
    float x, y;
    Point()
    {
        x = 0.0f;
        y = 0.0f;
    }
    Point(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
    ~Point()
    {
        //
    }
};

class CircleElement
{
private:
    /* data */
public:
    std::vector<float> vertices;
    std::vector<int> indexes;
    CircleElement(float radius, int numberOfPoints)
    {
        float theta, x, y;

        theta = 2.0f*M_PI/numberOfPoints;
        for(int step = 0; step < numberOfPoints; step++)
        {
            x = radius*cos(theta*step);
            y = radius*sin(theta*step);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }
        for(int i = 1;i < (numberOfPoints - 1);i++)
        {
            indexes.push_back(0);
            indexes.push_back(i);
            indexes.push_back(i + 1);
        }
    }
    CircleElement(Point center, float radius, int numberOfPoints)
    {
        float theta, x, y;

        theta = 2.0f*M_PI/numberOfPoints;
        for(int step = 0; step < numberOfPoints; step++)
        {
            x = center.x + radius*cos(theta*step);
            y = center.y + radius*sin(theta*step);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }
        for(int i = 1;i < (numberOfPoints - 1);i++)
        {
            indexes.push_back(0);
            indexes.push_back(i);
            indexes.push_back(i + 1);
        }
    }
    // Método para guardar los índices en un vector de enteros sin signo por medio de punteros
    void saveIndexes(unsigned int* indexes)
    {
        for(int i = 0;i < this->indexes.size();i++)
            indexes[i] = this->indexes[i];
    }
    // Método para guardar los vértices en un vector de flotantes por medio de punteros
    void saveVertices(float* vertices)
    {
        for(int i = 0;i < this->vertices.size();i++)
            vertices[i] = this->vertices[i];
    }
    ~CircleElement()
    {
        //
    }
};

class RectangleElement
{
private:
    // Método para actualizar los vértices
    void __update(vec2 topLeft, vec2 bottomRight)
    {
        vec2 topRight, bottomLeft;
        topRight[0] = bottomRight[0];
        topRight[1] = topLeft[1];
        bottomLeft[0] = topLeft[0];
        bottomLeft[1] = bottomRight[1];

        // Limpiar el vector de vértices
        vertices.clear();

        // Agregar los nuevos vértices
        vertices.push_back(topLeft[0]);
        vertices.push_back(topLeft[1]);
        vertices.push_back(0.0f);

        vertices.push_back(topRight[0]);
        vertices.push_back(topRight[1]);
        vertices.push_back(0.0f);

        vertices.push_back(bottomLeft[0]);
        vertices.push_back(bottomLeft[1]);
        vertices.push_back(0.0f);

        vertices.push_back(bottomRight[0]);
        vertices.push_back(bottomRight[1]);
        vertices.push_back(0.0f);

    }
public:
    std::vector<float> vertices;
    const std::vector<int> indexes = {0, 1, 3, 0, 2, 3};
    RectangleElement()
    {
        vec2 topLeft = {0.0f, 0.0f}, bottomRight = {0.1f, -0.1f};
        __update(topLeft, bottomRight);
    }
    RectangleElement(vec2 topLeft, vec2 bottomRight)
    {
        __update(topLeft, bottomRight);
    }
    // Método para guardar los índices en un vector de enteros sin signo por medio de punteros
    void saveIndexes(unsigned int* indexes)
    {
        for(int i = 0;i < this->indexes.size();i++)
            indexes[i] = this->indexes[i];
    }
    // Método para guardar los vértices en un vector de flotantes por medio de punteros
    void saveVertices(float* vertices)
    {
        for(int i = 0;i < this->vertices.size();i++)
            vertices[i] = this->vertices[i];
    }
    // Método para actualizar los vértices
    void update(vec2 topLeft, vec2 bottomRight)
    {
        __update(topLeft, bottomRight);
    }
    ~RectangleElement()
    {
        //
    }
};

// Función para detectar los obstáculos por medio del color, como si fuera una sensor con rango
std::vector<Point> detectObstacles(float x, float y);
//Función que calcula los vértices de un círculo
std::vector<float> buildCircle(float radius, int numberOfPoints);
// Función para leer el estado de las flechas del teclado
void pollKeys(GLFWwindow* window);
// Función para convertir un Point a coordenadas de OpenGL
Point convertToOpenGL(Point point);
// Función para convertir un Point a coordenadas de pantalla
Point convertToScreen(Point point);
// Función para eliminar las líneas que se encuentran dentro de los obstáculos
std::vector<float> removeLines(const std::vector<float> lines);


void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    vec3 color;
    if(button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
    {
        // printf("Mouse's left button pressed\n");
        glfwGetCursorPos(window, &x, &y);
        // Transformar coordenadas de tl-br a bl-tr
        y = height - y;
        printf("Mouse position: (%0.f, %0.f)\n", x, y);
        glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &color);
        printf("Color: (%0.3f, %0.3f, %0.3f)\n", color[0], color[1], color[2]);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int main(){
    int  success;
    char infoLog[512];

    float radius = 0.01f;
    int numberOfPoints = 16;
    CircleElement robot(radius, numberOfPoints);
    float robotVertices[robot.vertices.size()];
    unsigned int robotIndexes[robot.indexes.size()];
    robot.saveVertices(robotVertices);
    robot.saveIndexes(robotIndexes);

    // Definimos los vértices de los obstáculos
    vec2 topLeft[] = {{0.3f, 0.5f}, {-0.5f, 0.2f}, {0.6f, -0.2f}},
    bottomRight[] = {{0.5f, 0.0f}, {-0.2f, -0.1f}, {0.9f, -0.51f}};
    int numberOfObstacles = sizeof(topLeft)/sizeof(topLeft[0]);
    // Voy a hacer un arreglo de RectangleElement para guardar todos los obstáculos
    RectangleElement obstacles[numberOfObstacles];
    for(int i = 0;i < numberOfObstacles;i++)
        obstacles[i].update(topLeft[i], bottomRight[i]);

    const std::vector<float> moreObstacles = {
        -0.8f,  -0.32f, 0.0f,    // First polygon
        -0.4f,  -0.37f, 0.0f,
        -0.63f, -0.5f,  0.0f,
        -0.8f,  0.65f,  0.0f,   // Second polygon
        -0.84f, 0.76f,  0.0f,
        -0.68f, 0.65f,  0.0f,
        -0.9f,  0.57f,  0.0f
    };
    // Ahora voy a guardar todos los vértices y los índices en vectores de flotantes y enteros sin signo
    std::vector<float> obstacleVertices_vec, lineVertices_vec;
    std::vector<int> obstacleIndexes_vec;
    std::vector<Point> vertices;
    for(int i = 0;i < numberOfObstacles;i++)
    {
        obstacleVertices_vec.insert(obstacleVertices_vec.end(), obstacles[i].vertices.begin(), obstacles[i].vertices.end());
        // Condición para agregar los índices del primer obstáculo
        if(i == 0)
            obstacleIndexes_vec.insert(obstacleIndexes_vec.end(), obstacles[i].indexes.begin(), obstacles[i].indexes.end());
        else
        {
            // Antes de agregar los índices de los siguientes obstáculos, debemos sumar el número de vértices de los obstáculos anteriores
            for(int j = 0;j < obstacles[i].indexes.size();j++)
                obstacleIndexes_vec.push_back(obstacles[i].indexes[j] + obstacles[i - 1].vertices.size()*i/3);
        }
        // Agregar las coordenadas x e y de los vértices a un vector de Point
        for(int j = 0;j < obstacles[i].vertices.size();j += 3)
            vertices.push_back(Point(obstacles[i].vertices[j], obstacles[i].vertices[j + 1]));
    }
    // Ahora agregamos los vértices y los índices de los obstáculos nuevos
    obstacleVertices_vec.insert(obstacleVertices_vec.end(), moreObstacles.begin(), moreObstacles.end());
    for(int i = 0;i < 3;i++)
        obstacleIndexes_vec.push_back(i + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    // Agregamos los indices del polígono de 4 vértices. Deben dibujarse los vértices en el siguiente orden: 0, 1, 2, 0, 2, 3, pero le sumamos 3, debido 
    // a que ya se agregaron los índices del primer polígono (triángulo)
    obstacleIndexes_vec.push_back(3 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    obstacleIndexes_vec.push_back(4 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    obstacleIndexes_vec.push_back(5 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    obstacleIndexes_vec.push_back(3 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    obstacleIndexes_vec.push_back(5 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    obstacleIndexes_vec.push_back(6 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    // for(int i = 0;i < 2;i++)
    // {
    //     obstacleIndexes_vec.push_back(0 + obstacleVertices_vec.size()/3 - moreObstacles.size()/3);
    //     obstacleIndexes_vec.push_back(i + 1);
    //     obstacleIndexes_vec.push_back(i + 2);
    // }
    printf("Vertices in moreObstacles: %lu\n", moreObstacles.size()/3);
    for(int i = 0;i < moreObstacles.size();i += 3)
        vertices.push_back(Point(moreObstacles[i], moreObstacles[i + 1]));

    // Aquí unimos con líneas cada uno de los vértices de los obstáculos con los demás vértices de los obstáculos
    for(int i = 0;i < vertices.size();i++)
    {
        for(int j = i + 1;j < vertices.size();j++)
        {
            lineVertices_vec.push_back(vertices[i].x);
            lineVertices_vec.push_back(vertices[i].y);
            lineVertices_vec.push_back(0.0f);
            lineVertices_vec.push_back(vertices[j].x);
            lineVertices_vec.push_back(vertices[j].y);
            lineVertices_vec.push_back(0.0f);
        }
    }
 
    // Ahora voy a crear un círculo para cada vértice de los obstáculos
    int numberOfVertices = vertices.size();
    std::vector<float> verticesCircles_vec;
    std::vector<int> verticesIndexes_vec;
    for(int i = 0;i < numberOfVertices;i++)
    {
        CircleElement circle(vertices[i], 0.01f, 16);
        verticesCircles_vec.insert(verticesCircles_vec.end(), circle.vertices.begin(), circle.vertices.end());
        // Condición para agregar los índices del primer obstáculo
        if(i == 0)
            verticesIndexes_vec.insert(verticesIndexes_vec.end(), circle.indexes.begin(), circle.indexes.end());
        else
        {
            // Antes de agregar los índices de los siguientes obstáculos, debemos sumar el número de vértices de los obstáculos anteriores
            for(int j = 0;j < circle.indexes.size();j++)
                verticesIndexes_vec.push_back(circle.indexes[j] + circle.vertices.size()*i/3);
        }
    }
    
    // Ya tenemos los vértices y los índices de los obstáculos, ahora los guardamos en arreglos para poder usar las funciones de OpenGL
    float obstaclesVertices[obstacleVertices_vec.size()], verticesCircles[verticesCircles_vec.size()], lineVertices[lineVertices_vec.size()];
    unsigned int obstaclesIndexes[obstacleIndexes_vec.size()], verticesIndexes[verticesIndexes_vec.size()];
    for(int i = 0;i < obstacleVertices_vec.size();i++)
        obstaclesVertices[i] = obstacleVertices_vec[i];
    for(int i = 0;i < obstacleIndexes_vec.size();i++)
        obstaclesIndexes[i] = obstacleIndexes_vec[i];
    for(int i = 0;i < verticesCircles_vec.size();i++)
        verticesCircles[i] = verticesCircles_vec[i];
    for(int i = 0;i < verticesIndexes_vec.size();i++)
        verticesIndexes[i] = verticesIndexes_vec[i];

    for(int i = 0;i < lineVertices_vec.size();i++)
        lineVertices[i] = lineVertices_vec[i];

    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
        printf("Initialization failed\n");

    GLFWwindow* window = glfwCreateWindow(width, height, "Window name", NULL, NULL);
    if (!window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Build and compile our shader program
    Shader obstaclesShader("visibility/obstacles.vs", "visibility/obstacles.fs"), verticesShader("visibility/robot.vs", "visibility/robot.fs"),
    linesShader("visibility/draws.vs", "visibility/draws.fs");
    unsigned int VBO[3], VAO[3], EBO[3];

    // Vertex buffer object
    glGenVertexArrays(3, VAO); 
    glGenBuffers(3, VBO);
    glGenBuffers(2, EBO);  

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(obstaclesVertices), obstaclesVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(obstaclesIndexes), obstaclesIndexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCircles), verticesCircles, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(verticesIndexes), verticesIndexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    robotID = verticesShader.ID;

    float x, y;
    vec3 color, colorTemp;
    colorTemp[0] = -1.0f;
    float circleVertices[robot.vertices.size()];
    unsigned int circleIndexes[robot.indexes.size()];
    bool isFirstTime = true;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        obstaclesShader.use();
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, obstacleIndexes_vec.size(), GL_UNSIGNED_INT, 0);

        // Esto solo debe ejecutarse una vez
        if(isFirstTime)
        {
            lineVertices_vec = removeLines(lineVertices_vec);
            float lineVertices[lineVertices_vec.size()];
            for(int i = 0;i < lineVertices_vec.size();i++)
                lineVertices[i] = lineVertices_vec[i];
            glBindVertexArray(VAO[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
            isFirstTime = false;
        }

        linesShader.use();
        glBindVertexArray(VAO[2]);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, lineVertices_vec.size()/3);
        
        verticesShader.use();
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, verticesIndexes_vec.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(3, VBO);
    glfwTerminate();
    return 0;
}


// Función para detectar los obstáculos por medio del color, como si fuera una sensor con rango
std::vector<Point> detectObstacles(float x, float y)
{
    vec3 color;
    const int numberOfPoints = 70;
    float theta = 0, sensorX, sensorY, angleStep = 2.0f*M_PI/numberOfPoints, laserStep = 0.009f;
    int numberOfPointsInLine = int(sensorRadius/laserStep);
    std::vector<Point> points;

    for(int i = 0;i < numberOfPoints;i++)
    {
        Point startPoint(x, y), middlePoint, normalizedMiddlePoint, endPoint(x + sensorRadius*cos(theta), y + sensorRadius*sin(theta)), newPoint;
        float distance = sqrt(pow(endPoint.x - startPoint.x, 2) + pow(endPoint.y - startPoint.y, 2)), newRadius = laserStep*numberOfPointsInLine/2.0f;
        theta = angleStep*i;
        
        for(int i = 1;i <= numberOfPointsInLine;i++) // Esto es justo lo que no quería hacer. No es eficiente
        {
            startPoint.x = x + laserStep*cos(theta)*i;
            startPoint.y = y + laserStep*sin(theta)*i;
            newPoint = convertToScreen(startPoint);
            glReadPixels(newPoint.x, newPoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
            if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
            {
                break;
            }
        }
        glReadPixels(newPoint.x, newPoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
        if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
        {
            newPoint = convertToOpenGL(newPoint);
            // printf("Point: (%0.3f, %0.3f)\n", newPoint.x, newPoint.y);
            points.push_back(newPoint);
        }
    }
    return points;
}

//Función que calcula los vértices de un círculo
std::vector<float> buildCircle(float radius, int numberOfPoints)
{
    float theta, x, y;
    std::vector<float> vertices;
    std::vector<int> indexes_vect;

    theta = 2.0f*M_PI/numberOfPoints;
    for(int step = 0; step < numberOfPoints; step++){
        x = radius*cos(theta*step);
        y = radius*sin(theta*step);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
    }
    return vertices;
}

// Función para leer el estado de las flechas del teclado
void pollKeys(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        robotY += step;
        glUniform3f(glGetUniformLocation(robotID, "robotPosition"), robotX, robotY, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        robotY -= step;
        glUniform3f(glGetUniformLocation(robotID, "robotPosition"), robotX, robotY, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        robotX += step;
        glUniform3f(glGetUniformLocation(robotID, "robotPosition"), robotX, robotY, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        robotX -= step;
        glUniform3f(glGetUniformLocation(robotID, "robotPosition"), robotX, robotY, 0.0f);
    }
}
// Función para convertir un Point a coordenadas de OpenGL
Point convertToOpenGL(Point point)
{
    Point pointOpenGL;
    pointOpenGL.x = 2.0f*point.x/width - 1.0f;
    pointOpenGL.y = 2.0f*point.y/height - 1.0f;
    return pointOpenGL;
}

// Función para convertir un Point a coordenadas de pantalla
Point convertToScreen(Point point)
{
    Point pointScreen;
    pointScreen.x = (point.x + 1.0f)*width/2.0f;
    pointScreen.y = (point.y + 1.0f)*height/2.0f;
    return pointScreen;
}

// Función para eliminar las líneas que se encuentran dentro de los obstáculos
std::vector<float> removeLines(const std::vector<float> lines)
{
    std::vector<float> newLines = lines;
    vec3 color;
    Point startPoint, endPoint;
    int eraseCount = 0;
    float theta, distance;
    int numberOfPoints = 50;
    for(int i = 0;i < lines.size();i += 6)
    {
        startPoint.x = lines[i];
        startPoint.y = lines[i + 1];
        endPoint.x = lines[i + 3];
        endPoint.y = lines[i + 4];

        theta = atan2(endPoint.y - startPoint.y, endPoint.x - startPoint.x);
        distance = sqrt(pow(endPoint.x - startPoint.x, 2) + pow(endPoint.y - startPoint.y, 2));
        
        for(int j = 1;j < numberOfPoints;j++)
        {
            Point newPoint;
            newPoint.x = startPoint.x + j*distance*cos(theta)/numberOfPoints;
            newPoint.y = startPoint.y + j*distance*sin(theta)/numberOfPoints;
            newPoint = convertToScreen(newPoint);
            glReadPixels(newPoint.x, newPoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
            // Si el pixel en el punto medio es negro, eliminamos los puntos de inicio y fin de esta línea.
            // Pero si ese pixel está en la frontera del obstáculo, conservamos la línea.
            if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
            {
                glReadPixels(newPoint.x + 1, newPoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
                if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
                {
                    glReadPixels(newPoint.x - 1, newPoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
                    if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
                    {
                        glReadPixels(newPoint.x, newPoint.y + 1, 1, 1, GL_RGB, GL_FLOAT, &color);
                        if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
                        {
                            glReadPixels(newPoint.x, newPoint.y - 1, 1, 1, GL_RGB, GL_FLOAT, &color);
                            if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
                            {
                                newLines.erase(newLines.begin() + (i - eraseCount*6), newLines.begin() + (i - eraseCount*6 + 6));
                                eraseCount++;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return newLines;
}