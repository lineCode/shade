#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <SDL2/SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

namespace window {
  struct parameters {
    uint32_t width = 0;
    uint32_t height = 0;
    bool     fullscreen = false;
  };

  static SDL_Window *
  build
  (const parameters &parameters)
  {
    SDL_Window *result = SDL_CreateWindow(
      "Shade",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      parameters.width,
      parameters.height,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (NULL == result)
    {
      printf("Could not initialise SDL: %s\n", SDL_GetError());
      return NULL;
    }

    if (parameters.fullscreen)
      SDL_SetWindowFullscreen(result, SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext glContext = SDL_GL_CreateContext(result);
    if (NULL == glContext)
    {
      printf("Could not initialise OpenGL: %s\n", SDL_GetError());
      return NULL;
    }

    if(SDL_GL_SetSwapInterval(1) < 0)
    {
      printf("Could not set VSync: %s\n", SDL_GetError());
      return NULL;
    }

    return result;
  }
}

namespace gl {
  static void
  init
  (void)
  {
    GLuint vertexArray = 0;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  namespace geometry {
    static GLuint
    build
    (void)
    {
      static const float vertexPositions[] = {
        -1, 1, -1,-1,  1, 1,
         1,-1,  1, 1, -1,-1,
      };

      GLuint result = 0;
      glGenBuffers(1, &result);
      glBindBuffer(GL_ARRAY_BUFFER, result);
      glBufferData(GL_ARRAY_BUFFER,
        sizeof(vertexPositions),
        vertexPositions,
        GL_STATIC_DRAW
      );

      return result;
    }
  }

  namespace program {
    namespace shader {
      static GLuint
      build
      (const char *shaderSource, const GLenum shaderType)
      {
        GLuint result = glCreateShader(shaderType);
        glShaderSource(result, 1, &shaderSource, NULL);
        glCompileShader(result);

        GLint ret = GL_FALSE;
        glGetShaderiv(result, GL_COMPILE_STATUS, &ret);

        if (GL_FALSE == ret)
        {
          GLint len = 0; glGetShaderiv(result, GL_INFO_LOG_LENGTH, &len);
          char errorMessage[len+1];
          glGetShaderInfoLog(result, len, NULL, errorMessage);
          printf("Error: Couldn't build shader:\n %s", errorMessage);
        }

        return result;
      }
    }

    static GLuint
    link
    (GLuint vertexShader, GLuint fragmentShader)
    {
      GLuint result = glCreateProgram();
      glAttachShader(result, vertexShader); glAttachShader(result, fragmentShader);
      glLinkProgram(result);

      glDetachShader(result, vertexShader); glDetachShader(result, fragmentShader);
      glDeleteShader(vertexShader);         glDeleteShader(fragmentShader);

      GLint ret = GL_FALSE;
      glGetProgramiv(result, GL_LINK_STATUS, &ret);

      if (GL_FALSE == ret)
      {
        GLint len = 0; glGetProgramiv(result, GL_INFO_LOG_LENGTH, &len);
        char errorMessage[len+1];
        glGetProgramInfoLog(result, len, NULL, errorMessage);
        printf("Error: Unable to link shaders:\n %s", errorMessage);
      }

      return result;
    }

    GLuint
    build
    (void)
    {
      static const std::string fragmentShaderSource(
        #include "build/fragment.glsl.h"
      );

      static const std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 vPosition;

        out vec2 fPosition;

        void main()
        {
          gl_Position.xy = vPosition;
          gl_Position.zw = vec2(0, 1);
          fPosition      = (vPosition + vec2(1,1))/2;
        }
      )";

      return program::link(
        shader::build(vertexShaderSource.data(), GL_VERTEX_SHADER), 
        shader::build(fragmentShaderSource.data(), GL_FRAGMENT_SHADER)
      );
    }
  }
}

int
main
(int argc, char **argv)
{
  SDL_Window *window = window::build((window::parameters){
    .width = 1024,
    .height = 768,
    .fullscreen = false,
  });

  if (NULL == window)
    return 1;

  gl::init();
  GLuint program = gl::program::build();
  GLuint geometry = gl::geometry::build();

  bool running = true;
  while(running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = false;
    }

    if (!running)
      continue;

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, 1024, 768);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, geometry);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    SDL_GL_SwapWindow(window);
  }
}
