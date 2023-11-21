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
    vec2 topLeft[] = {{0.3f, 0.5f}, {-0.5f, 0.2f}, {-1.0f, 1.0f}, {-1.0f, 1.0f}, {0.98f, 1.0f}, {-1.0f, -0.98f}},
    bottomRight[] = {{0.5f, 0.0f}, {-0.2f, -0.1f}, {-0.98f, -1.0f}, {1.0f, 0.98f}, {1.0f, -1.0f}, {1.0f, -1.0f}};

    int numberOfObstacles = sizeof(topLeft)/sizeof(topLeft[0]);
    // Voy a hacer un arreglo de RectangleElement para guardar todos los obstáculos
    RectangleElement obstacles[numberOfObstacles];
    for(int i = 0;i < numberOfObstacles;i++)
        obstacles[i].update(topLeft[i], bottomRight[i]);
    // Ahora voy a guardar todos los vértices y los índices en vectores de flotantes y enteros sin signo
    std::vector<float> obstacleVertices_vec;
    std::vector<int> obstacleIndexes_vec;
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
    }
    // Ya tenemos los vértices y los índices de los obstáculos, ahora los guardamos en arreglos para poder usar las funciones de OpenGL
    float obstaclesVertices[obstacleVertices_vec.size()];
    unsigned int obstaclesIndexes[obstacleIndexes_vec.size()];
    for(int i = 0;i < obstacleVertices_vec.size();i++)
        obstaclesVertices[i] = obstacleVertices_vec[i];
    for(int i = 0;i < obstacleIndexes_vec.size();i++)
        obstaclesIndexes[i] = obstacleIndexes_vec[i];

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
    Shader myShader1("visibility/obstacles.vs", "visibility/obstacles.fs"), robotShader("visibility/robot.vs", "visibility/robot.fs"),
    drawShader("visibility/draws.vs", "visibility/draws.fs");
    unsigned int VBO[3], VAO[3], EBO[3], vertexShader, fragmentShader1, fragmentShader2, fragmentShader3, shaderProgram1, shaderProgram2, shaderProgram3;

    // Vertex buffer object
    glGenVertexArrays(3, VAO); 
    glGenBuffers(3, VBO);
    glGenBuffers(3, EBO);  

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(obstaclesVertices), obstaclesVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(obstaclesIndexes), obstaclesIndexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(robotVertices), robotVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(robotIndexes), robotIndexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(robotVertices), robotVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(robotIndexes), robotIndexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    robotID = robotShader.ID;

    float x, y;
    vec3 color, colorTemp;
    colorTemp[0] = -1.0f;
    float circleVertices[robot.vertices.size()];
    unsigned int circleIndexes[robot.indexes.size()];
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        myShader1.use();
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, obstacleIndexes_vec.size(), GL_UNSIGNED_INT, 0);

        // robotShader.use();
        // glBindVertexArray(VAO[1]);
        // glDrawElements(GL_TRIANGLES, robot.indexes.size(), GL_UNSIGNED_INT, 0);
        // pollKeys(window);

        std::vector<Point> detectedObstacles = detectObstacles(robotX, robotY);
        int i = 0, numberOfVertices = detectedObstacles.size()*3, indexesSize = 3*(detectedObstacles.size() - 2);
        unsigned int polygonIndexes[indexesSize];
        float polygonVertices[numberOfVertices];
        // Inserto las coordenadas del robot en el vector de puntos
        detectedObstacles.insert(detectedObstacles.begin(), Point(robotX, robotY));
        // For para guardar todos los puntos en un arreglo de vértices
        for(Point point : detectedObstacles)
        {
            polygonVertices[i] = point.x;
            polygonVertices[i + 1] = point.y;
            polygonVertices[i + 2] = 0.0f;
            i += 3;
        }
        for(int i = 1;i < detectedObstacles.size();i++)
        {
            polygonIndexes[3*(i - 1)] = 0;
            polygonIndexes[1 + 3*(i - 1)] = i;
            polygonIndexes[2 + 3*(i - 1)] = i + 1;
            if(i == (detectedObstacles.size() - 3))
                polygonIndexes[2 + 3*(i - 1)] = 1;
        }
        glBindVertexArray(VAO[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(polygonVertices), polygonVertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(polygonIndexes), polygonIndexes, GL_STATIC_DRAW);
        drawShader.use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES, indexesSize, GL_UNSIGNED_INT, 0);
        
        
        robotShader.use();
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, robot.indexes.size(), GL_UNSIGNED_INT, 0);
        pollKeys(window);

        // for(Point point : detectedObstacles)
        // {
        //     CircleElement circle(point, 0.008f, 16);
        //     circle.saveVertices(circleVertices);
        //     glBindVertexArray(VAO[2]);
        //     glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
        //     // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleIndexes), circleIndexes, GL_STATIC_DRAW);
        //     drawShader.use();
        //     glDrawElements(GL_TRIANGLES, circle.indexes.size(), GL_UNSIGNED_INT, 0);
        // }
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
                // printf("break\n");
                break;
            }
        }
        // // Me gustaría cooregir el error de que a veces no detecta el obstáculo
        // for(int i = 0;i < numberOfPointsInLine;i++)
        // {
        //     normalizedMiddlePoint.x = (startPoint.x + endPoint.x)/2.0f;
        //     normalizedMiddlePoint.y = (startPoint.y + endPoint.y)/2.0f;
        //     // Transformar las coordenadas a coordenadas de pantalla
        //     middlePoint = convertToScreen(normalizedMiddlePoint);
        //     glReadPixels(middlePoint.x, middlePoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
        //     if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
        //     {
        //         endPoint = normalizedMiddlePoint;
        //     }
        //     else if(color[0] == 1.0f && color[1] == 1.0f && color[2] == 1.0f)
        //     {
        //         startPoint = normalizedMiddlePoint;
        //     }
        //     distance = sqrt(pow(endPoint.x - startPoint.x, 2) + pow(endPoint.y - startPoint.y, 2));
        //     if(distance <= 2.0*laserStep)
        //     {
        //         break;
        //     }
        // }
        // glReadPixels(middlePoint.x, middlePoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
        // // printf("Color: (%0.3f, %0.3f, %0.3f)\n", color[0], color[1], color[2]);
        // if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
        // {
        // Point nearestPoint(normalizedMiddlePoint.x, normalizedMiddlePoint.y);
        // points.push_back(nearestPoint);
        // }
        glReadPixels(newPoint.x, newPoint.y, 1, 1, GL_RGB, GL_FLOAT, &color);
        if(color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f)
        {
            newPoint = convertToOpenGL(newPoint);
            // printf("Point: (%0.3f, %0.3f)\n", newPoint.x, newPoint.y);
            points.push_back(newPoint);
        }
    }
    // std::vector<float> points_vec;
    // for(int i = 0;i < points.size();i++)
    // {
    //     points_vec.push_back(points[i].x);
    //     points_vec.push_back(points[i].y);
    //     points_vec.push_back(0.0f);
    // }
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