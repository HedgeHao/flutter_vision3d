#ifndef _DEF_SHADER_
#define _DEF_SHADER_
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

#include <iostream>

class Shader
{
public:
    unsigned int vertextFixColor;
    unsigned int vertextWithColor;
    unsigned int textureShader;

    Shader()
    {
        createShaderProgram(&vertextFixColor, vertextFixColorVertexSource, vertextFixColorFragmentSource);
        createShaderProgram(&vertextWithColor, vertextWithColorVertexSource, vertextWithColorFragmentSource);
        createShaderProgram(&textureShader, textureVertexSource, textureFragmentSource);
    }

private:
    const char *vertextWithColorVertexSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 proj;\n"
        "out vec4 vertexColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "    vertexColor = vec4(aColor, 1.0);\n"
        "}\n";

    const char *vertextWithColorFragmentSource =
        "#version 330 core\n"
        "in vec4 vertexColor;\n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "    FragColor = vertexColor;\n"
        "}\n";

    const char *vertextFixColorVertexSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 proj;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\n";

    const char *vertextFixColorFragmentSource =
        "#version 330 core\n"
        "uniform vec3 color;\n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "    FragColor = vec4(color, 1);\n"
        "}\n";

    const char *textureVertexSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 proj;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "}\n";

    const char *textureFragmentSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D ourTexture;\n"
        "void main(){\n"
        "    FragColor = texture(ourTexture, TexCoord);\n"
        "}\n";

    static void createShaderProgram(unsigned int *program, const char *vertextSource, const char *fragmentSource)
    {
        int success;
        char infoLog[512];

        unsigned int vertextShader;
        vertextShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertextShader, 1, &vertextSource, NULL);
        glCompileShader(vertextShader);

        glGetShaderiv(vertextShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertextShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        };

        unsigned int fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        };

        *program = glCreateProgram();
        glAttachShader(*program, vertextShader);
        glAttachShader(*program, fragmentShader);
        glLinkProgram(*program);

        glDeleteShader(vertextShader);
        glDeleteShader(fragmentShader);
    }
};
#endif