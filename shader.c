#include "shader.h"
#include <stdlib.h>
#include <stdio.h>
#include "glad/glad.h"

static bool loadShader(int itype, const char *shaderfile, unsigned int *pshader)
{
    int success;
    char infoLog[512];
    const char* strType[] = {
        "VERTEX", "FRAGMENT", "UNKNOW"
    };
    const char *pstrtype = strType[2];
    if(itype == GL_VERTEX_SHADER)
    {
        pstrtype = strType[0];
    }
    else if(itype == GL_FRAGMENT_SHADER)
    {
        pstrtype = strType[1];
    }

    FILE *f = fopen(shaderfile, "r");
    if(!f)
    {
        fprintf(stderr, "fopen %s failed\n", shaderfile);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long filelen = ftell(f);
    char *shadersource = (char*)malloc(filelen * sizeof(char) + 1);
    if(!shadersource)
    {
        fprintf(stderr, "malloc %ld failed\n", filelen*sizeof(char));
        fclose(f);
        return false;
    }
    fseek(f, 0, SEEK_SET);
    fread(shadersource, 1, filelen*sizeof(char), f);
    fclose(f);

    shadersource[filelen] = 0;
    const char *pstr = shadersource;
    unsigned int vertexShader = glCreateShader(itype);
    glShaderSource(vertexShader, 1, &pstr, NULL);
    glCompileShader(vertexShader);

    free(shadersource);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::%s::COMPILATION_FAILED:\n**start**\n%s\n**end**\n", pstrtype, infoLog);
        return false;
    }
    *pshader = vertexShader;
    return true;
}

bool CreateProgram(const char *vertex, const char *fragment, unsigned int *pprog)
{
    int success;
    char infoLog[512];

    unsigned int vertexShader;
    unsigned int fragmentShader;
    if(loadShader(GL_VERTEX_SHADER, vertex, &vertexShader)
       && loadShader(GL_FRAGMENT_SHADER, fragment, &fragmentShader))
    {
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
            return false;
        }
        *pprog = shaderProgram;
        return true;
    }
    return false;
}
