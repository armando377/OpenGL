#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaders.h"
#include <stdio.h>

float vertices[] = {
     //positions        //colors
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.1f,
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.1f,
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.1f
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int main(){
    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
        printf("Initialization failed\n");

    GLFWwindow* window = glfwCreateWindow(800, 600, "Colors triangle", NULL, NULL);
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

    Shader myShader("triangle.vs", "triangle.fs");
    unsigned int VBO, VAO;

    // Vertex buffer object
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO); 

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); // no need to unbind at all as we directly bind a different VAO the next few lines

    while(!glfwWindowShouldClose(window)){
        glClearColor(0.055f, 0.647f, 0.91f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        myShader.use();
        float timeValue = glfwGetTime();
        // float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        int offsetLocation = glGetUniformLocation(myShader.ID, "offset");
        // myShader.setFloat("offset", timeValue/10);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}