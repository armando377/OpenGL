#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"
#include <math.h>
#include <stdio.h>


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

const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char *fragmentShader1Source =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.0f, 1.0f, 0.078f, 1.0f);\n"
"}\n\0";

const char *fragmentShader2Source =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);\n"
"}\n\0";

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
    int  success;
    char infoLog[512];

    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
        printf("Initialization failed\n");
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(880, 495, "Window name", NULL, NULL);
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
    // glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // int width, height;
    // glfwGetFramebufferSize(window, &width, &height);
    // glViewport(0, 0, width, height);
    glfwSetKeyCallback(window, key_callback);

    unsigned int VBO[2], VAO[2], EBO[2], vertexShader, fragmentShader1, fragmentShader2, shaderProgram1, shaderProgram2;
    //Create and compile vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    //Create and compile fragment shader
    fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
    fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader1, 1, &fragmentShader1Source, NULL);
    glCompileShader(fragmentShader1);
    glGetShaderiv(fragmentShader1, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader1, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }
    glShaderSource(fragmentShader2, 1, &fragmentShader2Source, NULL);
    glCompileShader(fragmentShader2);
    glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    shaderProgram1 = glCreateProgram();
    shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram1, vertexShader);
    glAttachShader(shaderProgram1, fragmentShader1);
    glLinkProgram(shaderProgram1);
    glGetProgramiv(shaderProgram1, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram1, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    glAttachShader(shaderProgram2, vertexShader);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);
    glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader1);

    // Vertex buffer object
    glGenVertexArrays(2, VAO); 
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);
    // glBindVertexArray(VAO);
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
    // glBindBuffer(GL_ARRAY_BUFFER, 0); 
    // glBindVertexArray(0); // no need to unbind at all as we directly bind a different VAO the next few lines

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL is default
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.055f, 0.647f, 0.91f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram1);
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        // glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shaderProgram2);
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram1);
    glfwTerminate();
    return 0;
}