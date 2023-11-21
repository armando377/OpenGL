#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "shaders.h"
#include <math.h>
#include <stdio.h>
#include <vector>

const int width = 880, height = 495;
float robotX = 0.0f, robotY = 0.0f, step = 0.005f;
int robotID;
std::vector<float> buildCircle(float radius, int numberOfPoints);
// Función para leer el estado de las flechas del teclado
void pollKeys(GLFWwindow* window);

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
        for(int step = 0; step < numberOfPoints; step++){
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
    ~CircleElement()
    {
        //
    }
};


float vertices[] = {
    -0.8f, -0.8f, 0.0f,
     0.0f, -0.8f, 0.0f,
    -0.4f, -0.8f*(1.0f - sqrt(3.0f)/2.0f), 0.0f,
     0.8f, -0.8f, 0.0f,
     0.4f, -0.8f*(1.0f - sqrt(3.0f)/2.0f), 0.0f,
     0.0f, -0.8f*(1.0f - sqrt(3.0f)), 0.0f
}; 

unsigned int indexes[] = {
    0, 1, 2,    //first triangle
    1, 3, 4,    //second triangle
    2, 4, 5
};


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
    CircleElement circle(radius, numberOfPoints);
    int vertex_size = circle.vertices.size();
    float robotVertices[vertex_size];
    for(int i = 0;i < vertex_size;i++)
        robotVertices[i] = circle.vertices[i];
    unsigned int robotIndexes[circle.indexes.size()];
    for(int i = 0;i < circle.indexes.size();i++)
        robotIndexes[i] = circle.indexes[i];

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
    Shader myShader1("triangle.vs", "triforce1.fs"), myShader2("triangle.vs", "triforce2.fs"), robotShader("game/triangle.vs", "robot.fs");
    unsigned int VBO[3], VAO[3], EBO[3], vertexShader, fragmentShader1, fragmentShader2, fragmentShader3, shaderProgram1, shaderProgram2, shaderProgram3;

    // Vertex buffer object
    glGenVertexArrays(3, VAO); 
    glGenBuffers(3, VBO);
    glGenBuffers(3, EBO);
    unsigned int indexes2[] = {1, 4, 2};    

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes2), indexes2, GL_STATIC_DRAW);

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
    colorTemp[0] = 0.7531326149f;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.055f, 0.647f, 0.91f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        myShader1.use();
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        // glDrawArrays(GL_TRIANGLES, 0, 3);

        myShader2.use();
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        robotShader.use();
        glBindVertexArray(VAO[2]);
        glDrawElements(GL_TRIANGLES, circle.indexes.size(), GL_UNSIGNED_INT, 0);
        pollKeys(window);

        // Aquí leemos el color de los pixeles en la posición del robot
        // Pero primero debemos transformar las coordenadas del robot a coordenadas de pantalla
        x = (robotX + 1.0f)*width/2.0f + radius*width;
        y = (robotY + 1.0f)*height/2.0f + radius*height;
        glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &color);
        if(color[0] != colorTemp[0] && color[1] != colorTemp[1] && color[2] != colorTemp[2])
        {
            printf("Color: (%0.3f, %0.3f, %0.3f)\n", color[0], color[1], color[2]);
            colorTemp[0] = color[0];
            colorTemp[1] = color[1];
            colorTemp[2] = color[2];
        }
        // printf("Color: (%0.3f, %0.3f, %0.3f)\n", color[0], color[1], color[2]);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(3, VBO);
    glfwTerminate();
    return 0;
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