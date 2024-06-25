#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "../shaders.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <random>

using namespace std;

#define WIDTH 800
#define HEIGHT 800
#define EPSILON 20
#define MAX_ITER 10000
#define RADIUS 50

// Robot parameters
#define WHEEL_RADIUS 0.0975
#define WHEEL_DISTANCE 0.381

int count = 0;
bool animate = false;

class Node{
    private:
        int x;
        int y;
        float cost = 0;
        Node *parent;
    public:
        Node(int x, int y);
        void setParent(Node *parent);
        int getX();
        int getY();
        float getCost();
        void setX(int x);
        void setY(int y);
        void setCost(float cost);
        Node *getParent();
};

Node::Node(int x, int y){
    this->x = x;
    this->y = y;
    parent = NULL;
}

void Node::setParent(Node *parent){
    this->parent = parent;
}

int Node::getX(){
    return x;
}

int Node::getY(){
    return y;
}

float Node::getCost(){
    return cost;
}

void Node::setX(int x){
    this->x = x;
}

void Node::setY(int y){
    this->y = y;
}

void Node::setCost(float cost){
    this->cost = cost;
}

Node *Node::getParent(){
    return parent;
}

class Obstacle
{
    private:
        int x;
        int y;
        int w;
        int h;
    public:
        Obstacle(int x, int y, int w, int h);
        int getX();
        int getY();
        int getW();
        int getH();
};

Obstacle::Obstacle(int x, int y, int w, int h)
{
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

int Obstacle::getX()
{
    return x;
}

int Obstacle::getY()
{
    return y;
}

int Obstacle::getW()
{
    return w;
}

int Obstacle::getH()
{
    return h;
}

class RRT{
    private:
        Node *start;
        Node *goal;
        vector<Node *> nodes;
        vector<Obstacle *> obstacles;
        bool isCollision(Node *node);
        bool isGoal(Node *node);
        float dist(Node *start, Node *goal);
        vector<Node *> chooseParent(Node *nearest, Node *newNode);
        vector<Node *> reWire(Node *newNode);
        // Función para verificar si hay camino libre entre dos nodos
        bool isFree(Node *node);
        bool goalFound = false;
        float theta = 0;
        float move = 0;
        vector<float> controls;
    public:
        RRT(Node *start, Node *goal);
        vector<float> generateObstacles();
        vector<float> drawRRT();
        vector<float> drawPath();
        void drawStartGoal();
        void draw();
        void run();
};

// Declare functions
// Error callback function
void error_callback(int error, const char* description);
// Key callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
// Frame buffer size callback function
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// Función para leer el estado de las flechas del teclado
void pollKeys(GLFWwindow* window);
// Función para convertir las coordenadas a coordenadas de OpenGL
float convertToOpenGL(int coordinate, bool isX);
// Función para convertir las coordenadas de OpenGL a coordenadas de la ventana
int convertToWindow(float coordinate, bool isX);
// Función para convertir las coordenadas de la ventana a coordenadas top-left
vector<float> convertToTopLeft(int x, int y);
//Función que calcula los vértices de un círculo
vector<float> buildCircle(float centerX, float centerY, float radius, int numberOfPoints);
// Función que dibuja el robot
vector<float> drawRobot(float x, float y);
// Función que calcula las acciones de control del robot
vector<float> calculateControls(vector<float>  path);
// Función que suaviza los controles
vector<float> smoothControls(vector<float> controls);
// Función que muestrea un control
vector<float> sampleControl();
// Función que muestrea un punto en el espacio de configuración
vector<float> samplePoint();


RRT::RRT(Node *start, Node *goal){
    this->start = start;
    this->goal = goal;
    nodes.push_back(start);
}

//generate random obstacles of different sizes and shapes: can be circular or rectangular
vector<float> RRT::generateObstacles()
{
    vector<float> obstacleVertices_vec;
    vec2 topLeft[] = {{-1.0f, 1.0f}, {-1.0f, 1.0f}, {0.98f, 1.0f}, {-1.0f, -0.98f}, {-0.8f, 1.0f}, {-0.6f, 0.38f}, {-0.5f, 0.3f}, {-0.4, -0.4f}, {0.1f, 0.25f},
                    {0.1f, 0.2f}, {-0.8f, -0.8f}, {0.9f, -0.5f}, {0.5f, 0.0f}, {0.4f, 1.0f}, {0.415f, 0.74f}},
    bottomRight[] = {{-0.98f, -1.0f}, {1.0f, 0.98f}, {1.0f, -1.0f}, {1.0f, -1.0f}, {-0.7f, 0.8f}, {-0.3f, 0.3f}, {-0.42, 0.1f}, {0.1f, -0.5f}, {0.25f, 0.2f},
                    {0.15f, 0.05f}, {-0.74f, -1.0f}, {1.0f, -0.56f}, {0.542f, -0.896f}, {0.415f, 0.7f}, {0.7f, 0.7f}};
    int numberOfObstacles = sizeof(topLeft)/sizeof(topLeft[0]);
    for(int i = 0; i < numberOfObstacles; i++)
    {
        int w = int((bottomRight[i][0] - topLeft[i][0])*WIDTH/2), h = int((topLeft[i][1] - bottomRight[i][1])*HEIGHT/2),
        x = convertToWindow(topLeft[i][0], true), y = convertToWindow(topLeft[i][1], false);

        obstacleVertices_vec.push_back(topLeft[i][0]);
        obstacleVertices_vec.push_back(topLeft[i][1]);
        obstacleVertices_vec.push_back(0.0f);

        obstacleVertices_vec.push_back(topLeft[i][0]);
        obstacleVertices_vec.push_back(bottomRight[i][1]);
        obstacleVertices_vec.push_back(0.0f);

        obstacleVertices_vec.push_back(bottomRight[i][0]);
        obstacleVertices_vec.push_back(bottomRight[i][1]);
        obstacleVertices_vec.push_back(0.0f);

        obstacleVertices_vec.push_back(bottomRight[i][0]);
        obstacleVertices_vec.push_back(topLeft[i][1]);
        obstacleVertices_vec.push_back(0.0f);

        Obstacle *obstacle = new Obstacle(x, y, w, h);
        // Node *obstacle = new Node(x, y);
        obstacles.push_back(obstacle);
    }

    return obstacleVertices_vec;
}

vector<float> RRT::drawRRT()
{
    vector<float> lineVertices_vec;
    for (int i = 0; i < nodes.size(); i++){
        Node *node = nodes[i];
        if(node->getParent() != NULL){
            lineVertices_vec.push_back(convertToOpenGL(node->getX(), true));
            lineVertices_vec.push_back(convertToOpenGL(node->getY(), false));
            lineVertices_vec.push_back(0.0f);
            lineVertices_vec.push_back(convertToOpenGL(node->getParent()->getX(), true));
            lineVertices_vec.push_back(convertToOpenGL(node->getParent()->getY(), false));
            lineVertices_vec.push_back(0.0f);
        }
    }
    if(goalFound)
    {
        // Connect last node to goal
        lineVertices_vec.push_back(convertToOpenGL(nodes[nodes.size() - 1]->getX(), true));
        lineVertices_vec.push_back(convertToOpenGL(nodes[nodes.size() - 1]->getY(), false));
        lineVertices_vec.push_back(0.0f);
        lineVertices_vec.push_back(convertToOpenGL(goal->getX(), true));
        lineVertices_vec.push_back(convertToOpenGL(goal->getY(), false));
        lineVertices_vec.push_back(0.0f);
    }
    return lineVertices_vec;
}

vector<float> RRT::drawPath()
{
    vector<float> lineVertices_vec;
    Node *node = nodes[nodes.size() - 1];
    // Connect last node to goal
    lineVertices_vec.push_back(convertToOpenGL(goal->getX(), true));
    lineVertices_vec.push_back(convertToOpenGL(goal->getY(), false));
    lineVertices_vec.push_back(0.0f);
    lineVertices_vec.push_back(convertToOpenGL(node->getX(), true));
    lineVertices_vec.push_back(convertToOpenGL(node->getY(), false));
    lineVertices_vec.push_back(0.0f);
    while(node->getParent() != NULL){
        lineVertices_vec.push_back(convertToOpenGL(node->getX(), true));
        lineVertices_vec.push_back(convertToOpenGL(node->getY(), false));
        lineVertices_vec.push_back(0.0f);
        lineVertices_vec.push_back(convertToOpenGL(node->getParent()->getX(), true));
        lineVertices_vec.push_back(convertToOpenGL(node->getParent()->getY(), false));
        lineVertices_vec.push_back(0.0f);
        node = node->getParent();
    }
    vector<float> aux(lineVertices_vec.size());
    copy(lineVertices_vec.begin(), lineVertices_vec.end(), aux.begin());
    // Reverse path
    for(int i = 0; i < lineVertices_vec.size()/3; i++)
    {
        lineVertices_vec[3*i] = aux[3*(lineVertices_vec.size()/3 - i - 1)];
        lineVertices_vec[3*i + 1] = aux[3*(lineVertices_vec.size()/3 - i - 1) + 1];
        lineVertices_vec[3*i + 2] = aux[3*(lineVertices_vec.size()/3 - i - 1) + 2];
    }
    return lineVertices_vec;
}

void RRT::drawStartGoal(){
    // glBegin(GL_POINTS);
    //     glVertex2f(start->getX(), start->getY());
    //     glVertex2f(goal->getX(), goal->getY());
    // glEnd();
}

void RRT::draw(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(1);
    // glColor3f(0, 0, 0);
    drawRRT();
    glLineWidth(4);
    // glColor3f(1, 0, 0);
    drawPath();
}

void RRT::run(){
    srand(time(NULL));
    int iteration = 0;
    while(iteration < MAX_ITER){
        vector<float> point = samplePoint();
        int x = point[0], y = point[1];
        Node *node = new Node(x, y);
        int minIndex = 0;
        float minDist = dist(node, nodes[0]);
        for(int i = 1; i < nodes.size(); i++){
            float d = dist(node, nodes[i]);
            if(d < minDist){
                minDist = d;
                minIndex = i;
            }
        }
        Node *nearest = nodes[minIndex];
        float theta = atan2(node->getY() - nearest->getY(), node->getX() - nearest->getX());
        int newX = nearest->getX() + EPSILON * cos(theta);
        int newY = nearest->getY() + EPSILON * sin(theta);
        Node *newNode = new Node(newX, newY);
        // newNode->setParent(nearest);
        vector<Node *> aux = chooseParent(nearest, newNode);
        newNode = aux[0];
        nearest = aux[1];
        if(!isCollision(newNode) && isFree(newNode))
        {
            nodes.push_back(newNode);
            nodes = reWire(newNode);
            if(isGoal(newNode))
            {
                cout << "Goal found!" << endl;
                goalFound = true;
                return;
            }
        }
        iteration++;
    }
    cout << "Goal not found!" << endl;
}

bool RRT::isCollision(Node *node){
    for (int i = 0; i < obstacles.size(); i++)
    {
        int minDistToObstacle = 8;
        Obstacle *obstacle = obstacles[i];
        if(node->getX() >= obstacle->getX() -minDistToObstacle && node->getX() <= obstacle->getX() + obstacle->getW() + minDistToObstacle &&
        node->getY() >= obstacle->getY() - minDistToObstacle && node->getY() <= obstacle->getY() + obstacle->getH() + minDistToObstacle)
        {
            return true;
        }
        // if(dist(node, obstacle) < 10){
        //     return true;
        // }
    }
    return false;
}

bool RRT::isGoal(Node *node){
    return dist(node, goal) < 20;
}

float RRT::dist(Node *start, Node *goal){
    return sqrt(pow(start->getX() - goal->getX(), 2) + pow(start->getY() - goal->getY(), 2));
}

vector<Node *> RRT::chooseParent(Node *nearest, Node *newNode)
{
    vector<Node *> output;
	for(Node *node : nodes)
    {
        if(dist(node, newNode) < RADIUS && node->getCost() + dist(node, newNode) < nearest->getCost() + dist(nearest, newNode))
            nearest = node;
    }
    newNode->setCost(nearest->getCost() + dist(nearest, newNode));
    newNode->setParent(nearest);
    
    output.push_back(newNode);
    output.push_back(nearest);
	return output;
}

vector<Node *> RRT::reWire(Node *newNode)
{
    for(int i = 0; i < nodes.size(); i++)
    {
        Node *node = nodes[i];
        if(node != newNode->getParent() && dist(node, newNode) < RADIUS && newNode->getCost() + dist(node, newNode) < node->getCost() && isFree(newNode))
            {
                node->setParent(newNode);
                node->setCost(newNode->getCost() + dist(node, newNode));
                nodes[i] = node;
            }               
    }
    return nodes;
}

// Función para verificar si hay camino libre entre dos nodos
bool RRT::isFree(Node *node)
{
    float x1 = node->getX(), y1 = node->getY(), x2 = node->getParent()->getX(), y2 = node->getParent()->getY();
    float theta = atan2(y2 - y1, x2 - x1), distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    int numberOfPoints = 30, minDistToObstacle = 10;
    vec3 color;

    for(int i = 1; i <= numberOfPoints; i++)
    {
        float newX = x1 + i*distance*cos(theta)/numberOfPoints, newY = y1 + i*distance*sin(theta)/numberOfPoints;
        for(int j = 0; j < obstacles.size(); j++)
        {
            Obstacle *obstacle = obstacles[j];
            if(newX >= obstacle->getX() - minDistToObstacle && newX <= obstacle->getX() + obstacle->getW() + minDistToObstacle &&
            newY >= obstacle->getY() - minDistToObstacle && newY <= obstacle->getY() + obstacle->getH() + minDistToObstacle)
                return false;
        } 
    }

    theta = atan2(y1 - y2, x1 - x2);
    for(int i = 1; i <= numberOfPoints; i++)
    {
        float newX = x1 + i*distance*cos(theta)/numberOfPoints, newY = y1 + i*distance*sin(theta)/numberOfPoints;
        for(int j = 0; j < obstacles.size(); j++)
        {
            Obstacle *obstacle = obstacles[j];
            if(newX >= obstacle->getX() - minDistToObstacle && newX <= obstacle->getX() + obstacle->getW() + minDistToObstacle &&
            newY >= obstacle->getY() - minDistToObstacle && newY <= obstacle->getY() + obstacle->getH() + minDistToObstacle)
                return false;
        } 
    }
    return true;
}



int main(){
    // Initialize GLFW
    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
        printf("Initialization failed\n");
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "RRT star", NULL, NULL);
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

    // Run RRT
    Node *start = new Node(18, 418);
    // Node *goal = new Node(WIDTH - 20, HEIGHT - 20);
    Node *goal = new Node(WIDTH - 150, 600);
    // Node *goal = new Node(WIDTH/2, HEIGHT/2);
    RRT *rrt = new RRT(start, goal);
    vector<float> obstacleVertices_vec = rrt->generateObstacles();
    rrt->run();

    // Get vertices from RRT
    vector<float> lineVertices_vec = rrt->drawRRT(), pathVertices_vec = rrt->drawPath(),
    goalVertices_vec = buildCircle(convertToOpenGL(goal->getX(), true), convertToOpenGL(goal->getY(), false), 0.01f, 16),
    startVertices_vec = buildCircle(convertToOpenGL(start->getX(), true), convertToOpenGL(start->getY(), false), 0.01f, 16),
    // robotVertices_vec = drawRobot(convertToOpenGL(start->getX(), true), convertToOpenGL(start->getY(), false));
    // robotVertices_vec = drawRobot(convertToOpenGL(start->getX(), true), convertToOpenGL(start->getY(), false));
    robotVertices_vec = drawRobot(0.0f, 0.0f);
    goalVertices_vec.insert(goalVertices_vec.end(), startVertices_vec.begin(), startVertices_vec.end());

    float lineVertices[lineVertices_vec.size()], pathVertices[pathVertices_vec.size()], goalVertices[goalVertices_vec.size()],
    robotVertices[robotVertices_vec.size()],
    obstacleVertices[obstacleVertices_vec.size()];

    copy(lineVertices_vec.begin(), lineVertices_vec.end(), lineVertices);
    copy(pathVertices_vec.begin(), pathVertices_vec.end(), pathVertices);
    copy(goalVertices_vec.begin(), goalVertices_vec.end(), goalVertices);
    copy(robotVertices_vec.begin(), robotVertices_vec.end(), robotVertices);
    copy(obstacleVertices_vec.begin(), obstacleVertices_vec.end(), obstacleVertices);

    vector<float> controls = calculateControls(pathVertices_vec);
    vector<float> smoothedControls = smoothControls(controls);

    // Build and compile our shader program
    Shader goalShader("rrt/goal.vs", "rrt/goal.fs"), pathShader("rrt/path.vs", "rrt/path.fs"),
    linesShader("rrt/lines.vs", "rrt/lines.fs"), robotShader("rrt/robot.vs", "rrt/robot.fs");
    unsigned int VBO[5], VAO[5], EBO[2];

    // Vertex buffer object
    glGenVertexArrays(5, VAO); 
    glGenBuffers(5, VBO);
    glGenBuffers(2, EBO);

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    // Goal
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(goalVertices), goalVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Path
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pathVertices), pathVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Lines
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Obstacles
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(obstacleVertices), obstacleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Robot
    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(robotVertices), robotVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Translate robot to start position
    robotShader.use();
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::rotate(trans, glm::radians(0.0f), glm::vec3(0.0, 0.0, 1.0));
    // trans = glm::translate(trans, glm::vec3(convertToOpenGL(goal->getX(), true), convertToOpenGL(goal->getY(), false), 0.0f));
    trans = glm::translate(trans, glm::vec3(convertToOpenGL(start->getX(), true), convertToOpenGL(start->getY(), false), 0.0f));
    unsigned int transformLoc = glGetUniformLocation(robotShader.ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    long int timer = 0, i = 0;
    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        float timeValue = glfwGetTime();

        glDisable(GL_BLEND);
        pathShader.use();
        glBindVertexArray(VAO[1]);
        glLineWidth(4.0f);
        glDrawArrays(GL_LINES, 0, pathVertices_vec.size()/3);

        linesShader.use();
        glBindVertexArray(VAO[2]);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, lineVertices_vec.size()/3);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        pathShader.use();
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_QUADS, 0, obstacleVertices_vec.size()/3);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, goalVertices_vec.size()/6);
        glDrawArrays(GL_TRIANGLE_FAN, goalVertices_vec.size()/6, goalVertices_vec.size()/6);

        goalShader.use();
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, goalVertices_vec.size()/6);

        robotShader.use();
        glBindVertexArray(VAO[4]);
        glDrawArrays(GL_QUADS, 0, robotVertices_vec.size()/3);
        pollKeys(window);

        if(count > 0)
            {
                if(i == smoothedControls.size()/2)
                    count = 0;
                if(timeValue > 0.04f)
                {
                    // trans = glm::rotate(trans, controls[2*i], glm::vec3(0.0, 0.0, 1.0));
                    // trans = glm::translate(trans, glm::vec3(controls[2*i + 1], 0.0f, 0.0f)); 
                    trans = glm::rotate(trans, smoothedControls[2*i], glm::vec3(0.0, 0.0, 1.0));
                    trans = glm::translate(trans, glm::vec3(smoothedControls[2*i + 1], 0.0f, 0.0f)); 
                    unsigned int transformLoc = glGetUniformLocation(robotShader.ID, "transform");
                    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
                    i++;
                    glfwSetTime(0.0f);
                }
            }

        // if(animate)
        // {
        //     if(i >= lineVertices_vec.size()/6)
        //         i = lineVertices_vec.size()/6;
        //     // glDrawArrays(GL_LINES, 0, 2*i - 2);
        //     linesShader.use();
        //     glBindVertexArray(VAO[2]);
        //     glLineWidth(1.0f);
        //     glDrawArrays(GL_LINES, 0, 2*i - 2);
        //     timer++;
        //     if(timer >= 2)
        //     {
        //         i++;
        //         timer = 0;
        //     }
        // }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(4, VAO);
    glDeleteBuffers(4, VBO);    
    glfwTerminate();
    return 0;
}

// Error callback function
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// Función para leer el estado de las flechas del teclado
void pollKeys(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        count++;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        animate = true;
}

// Función para convertir las coordenadas a coordenadas de OpenGL
float convertToOpenGL(int coordinate, bool isX)
{
    if(isX)
        return 1.0f*coordinate/WIDTH*2.0f - 1.0f;
    else
        return 1.0f*(HEIGHT - coordinate)/HEIGHT*2.0f - 1.0f;
}

// Función para convertir las coordenadas de OpenGL a coordenadas de la ventana
int convertToWindow(float coordinate, bool isX)
{
    if(isX)
        return (coordinate + 1.0f)*WIDTH/2.0f;
    else
        return HEIGHT - (coordinate + 1.0f)*HEIGHT/2.0f;
}

// Función para convertir las coordenadas de la ventana a coordenadas top-left
vector<float> convertToTopLeft(int x, int y)
{
    vector<float> point;
    point.push_back(x);
    point.push_back(HEIGHT - y);
    return point;
}

//Función que calcula los vértices de un círculo
vector<float> buildCircle(float centerX, float centerY, float radius, int numberOfPoints)
{
    vector<float> vertices;
    float theta, x, y;

    theta = 2.0f*M_PI/numberOfPoints;
    for(int step = 0; step < numberOfPoints; step++)
    {
        x = centerX + radius*cos(theta*step);
        y = centerY + radius*sin(theta*step);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
    }
    return vertices;
}

// Función que dibuja el robot
vector<float> drawRobot(float x, float y)
{
    vector<float> vertices;
    float width = 0.01f, height = 0.02f, wheelDiameter = 18.5f/38.0f*height;
    vec2 topLeft = {x - width/2, y + height/2}, bottomRight = {x + width/2, y - height/2};

    // First quad
    vertices.push_back(topLeft[0]);
    vertices.push_back(topLeft[1]);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0]);
    vertices.push_back(bottomRight[1]);
    vertices.push_back(0.0f);

    vertices.push_back(bottomRight[0]);
    vertices.push_back(bottomRight[1]);
    vertices.push_back(0.0f);

    vertices.push_back(bottomRight[0]);
    vertices.push_back(topLeft[1]);
    vertices.push_back(0.0f);

    // Second quad
    vertices.push_back(topLeft[0] - wheelDiameter);
    vertices.push_back(bottomRight[1]);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0] - wheelDiameter);
    vertices.push_back(bottomRight[1] - 0.01f);
    vertices.push_back(0.0f);
    
    vertices.push_back(bottomRight[0] + wheelDiameter);
    vertices.push_back(bottomRight[1] - 0.01f);
    vertices.push_back(0.0f);

    vertices.push_back(bottomRight[0] + wheelDiameter);
    vertices.push_back(bottomRight[1]);
    vertices.push_back(0.0f);

    // Third quad
    vertices.push_back(bottomRight[0] + wheelDiameter);
    vertices.push_back(topLeft[1]);
    vertices.push_back(0.0f);

    vertices.push_back(bottomRight[0] + wheelDiameter);
    vertices.push_back(topLeft[1] + 0.01f);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0] - wheelDiameter);
    vertices.push_back(topLeft[1] + 0.01f);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0] - wheelDiameter);
    vertices.push_back(topLeft[1]);
    vertices.push_back(0.0f);

    // Fourth quad (crazy wheel)
    vertices.push_back(topLeft[0] - width);
    vertices.push_back(y + 0.003f);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0] - width);
    vertices.push_back(y - 0.003f);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0]);
    vertices.push_back(y - 0.003f);
    vertices.push_back(0.0f);

    vertices.push_back(topLeft[0]);
    vertices.push_back(y + 0.003f);
    vertices.push_back(0.0f);
    return vertices;
}

// Función que calcula las acciones de control del robot
vector<float> calculateControls(vector<float>  path)
{
    vector<float> controls;
    float x1, y1, x2, y2, theta, move, turn, lastTheta = 0;
    for(int i = 0; i < path.size()/6 - 1; i++)
    {
        x1 = path[6*i];
        y1 = path[6*i + 1];
        x2 = path[6*i + 6];
        y2 = path[6*i + 7];
        theta = atan2(y2 - y1, x2 - x1);
        move = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
        turn = theta - lastTheta;
        lastTheta = theta;
        controls.push_back(turn);
        controls.push_back(move);
    }
    return controls;
}

// Función que suaviza los controles
vector<float> smoothControls(vector<float> controls)
{
    vector<float> smoothedControls;
    float move, turn;
    for(int i = 0; i < controls.size()/2; i++)
    {
        turn = controls[2*i];
        move = controls[2*i + 1];
        smoothedControls.push_back(turn/2);
        smoothedControls.push_back(0.0f);

        smoothedControls.push_back(turn/2);
        smoothedControls.push_back(move/2);

        smoothedControls.push_back(0.0f);
        smoothedControls.push_back(move/2);
    }
    return smoothedControls;
}

// Función que muestrea un punto en el espacio de configuración
vector<float> samplePoint()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0f, WIDTH), dis2(0.0f, HEIGHT), dis3(-M_PI, M_PI);
    float x = dis(gen), y = dis2(gen), theta = dis3(gen);
    vector<float> point;

    point.push_back(x);
    point.push_back(y);
    point.push_back(theta);
    return point;
}

// Función que muestrea un control
vector<float> sampleControl()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0f, 1.0f);
    float ur, ul, r = WHEEL_RADIUS, l = WHEEL_DISTANCE, x = 0.0f, y = 0.0f, theta = 0.0f, dt = 0.1f;
    int numberOfSamples = 10;
    vector<float> path;
    for(int i = 0; i < numberOfSamples; i++)
    {
        ur = dis(gen);
        ul = dis(gen);
        x += r/2*(ur + ul)*cos(theta)*dt;
        y += r/2*(ur + ul)*sin(theta)*dt;
        theta += r/l*(ur - ul)*dt;
        path.push_back(x);
        path.push_back(y);
        path.push_back(theta);
    }
    return path;
}