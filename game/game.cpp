#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../shaders.h"
#include <vector>
#include <math.h>
#include <stdio.h>


const int width = 800, height = 800;
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
        // std::vector<float> vertices;
        // std::vector<int> indexes_vect;

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



std::vector<float> buildCircle(float radius, int numberOfPoints);
// float vertices[] = {
//      //positions        //colors
//     -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
//      0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
//      0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
// };

// float vertices[] = {
//     -0.8f, -0.8f, 0.0f,
//      0.0f, -0.8f, 0.0f,
//     -0.4f, -0.8f*(1.0f - sqrt(3.0f)/2.0f), 0.0f
// }; 

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

int main(){
    float radius = 0.01f;
    float theta, x, y;
    int numberOfPoints = 16;
    CircleElement circle(radius, numberOfPoints), sensor(0.1, numberOfPoints);
    int vertex_size = circle.vertices.size();
    // printf("%d\n", vertex_size);
    float vertices[vertex_size];
    for(int i = 0;i < vertex_size;i++)
        vertices[i] = circle.vertices[i];
  
    int n = circle.indexes.size();
    // printf("Indexes size: %d\n", n);
    printf("Vertex size: %d\n", n);
    // for(int i = 0;i < vertex_size;i++)
    //     printf("%f,\t", vertices[i]);
    // printf("\n");
    unsigned int indexes[n];
    for(int i = 0;i < n;i++)
        indexes[i] = circle.indexes[i];

    float vertices2[vertex_size];
    for(int i = 0;i < vertex_size;i++)
        vertices2[i] = sensor.vertices[i];  
    //-------------------------------------------

    // // Debug
    // for(int i = 0;i < n;i++)
    // {
    //     printf("%d, ", indexes_vect[i]);
    //     if((i + 1)%3 == 0)
    //         printf("\n");
    // }

    // printf("%d\n", n);

    // float vertices[] = {
    //      0.0f,  0.0f, 0.0f,
    //      1.0f,  0.0f, 0.0f,
    //      0.0f,  1.0f, 0.0f,
    //     -1.0f,  0.0f, 0.0f,
    //      0.0f, -1.0f, 0.0f
    // };
    // unsigned int indexes[] = {
    //     0, 1, 2,
    //     0, 2, 3,
    //     0, 3, 4,
    //     0, 4, 1
    // };

    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
        printf("Initialization failed\n");

    GLFWwindow* window = glfwCreateWindow(width, height, "Colors triangle", NULL, NULL);
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

    Shader myShader("game/triangle.vs", "game/triangle.fs");
    Shader myShader2("game/triangle.vs", "game/triangle2.fs");
    unsigned int VBO[2], VAO[2], EBO[2];

    // Vertex buffer object
    glGenVertexArrays(2, VAO); 
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);


    // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); // no need to unbind at all as we directly bind a different VAO the next few lines

    int robotPositionLocation = glGetUniformLocation(myShader.ID, "robotPosition");
    glUniform3f(robotPositionLocation, 0.0f, 0.0f, 0.0f);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while(!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // glDisable(GL_BLEND);
        myShader.use();
        float timeValue = glfwGetTime();
        // float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        int offsetLocation = glGetUniformLocation(myShader.ID, "offset");
        myShader.setFloat("offset", timeValue/10);
        myShader.setVec3("robotPosition", timeValue/10, 0.0f, 0.0f);
        
        glBindVertexArray(VAO[0]);
        // glDrawArrays(GL_TRIANGLES, 0, 12);
        glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, 0);
        

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        myShader2.use();
        myShader2.setVec3("robotPosition", timeValue/10, 0.0f, 0.0f);
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, 0);


        // NOTA: Tengo que corregir esto, no funciona
        vec3 color;
        float x;
        // Transformar x del rango [-1, 1] al rango [0, width]
        x = (timeValue/10 + 1.0f)/2.0f*width;
        float sensorRadius = 0.01f;
        float xSensor = x + sensorRadius*width + 1;
        glReadPixels(xSensor, 0.0f, 1, 1, GL_RGB, GL_FLOAT, &color);
        // printf("Color: %f, %f, %f\n", color[0], color[1], color[2]);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
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