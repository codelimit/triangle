#include "eng.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <vector>

#define GL_CHECK()                                                             \
  {                                                                            \
    int error = glGetError();                                                  \
    if (error != GL_NO_ERROR) {                                                \
      switch (error) {                                                         \
      case GL_INVALID_ENUM:                                                    \
        std::cerr << GL_INVALID_ENUM << ": GL_INVALID_ENUM." << std::endl;     \
        break;                                                                 \
      case GL_INVALID_VALUE:                                                   \
        std::cerr << GL_INVALID_VALUE << ": GL_INVALID_VALUE" << std::endl;    \
        break;                                                                 \
      case GL_INVALID_OPERATION:                                               \
        std::cerr << GL_INVALID_OPERATION << ": GL_INVALID_OPERATION"          \
                  << std::endl;                                                \
        break;                                                                 \
      case GL_STACK_OVERFLOW:                                                  \
        std::cerr << GL_STACK_OVERFLOW << ": GL_STACK_OVERFLOW" << std::endl;  \
        break;                                                                 \
      case GL_STACK_UNDERFLOW:                                                 \
        std::cerr << GL_STACK_UNDERFLOW << ": GL_STACK_UNDERFLOW"              \
                  << std::endl;                                                \
        break;                                                                 \
      case GL_OUT_OF_MEMORY:                                                   \
        std::cerr << GL_OUT_OF_MEMORY << ": GL_OUT_OF_MEMORY" << std::endl;    \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

std::istream &operator>>(std::istream &is, vertex &v) {
  is >> v.x;
  is >> v.y;
  return is;
}

std::istream &operator>>(std::istream &is, triangle &t) {
  is >> t.v[0];
  is >> t.v[1];
  is >> t.v[2];
  return is;
}

bool initAll() {

  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);

  if (SDL_COMPILEDVERSION !=
      SDL_VERSIONNUM(linked.major, linked.minor, linked.patch)) {
    std::cerr << "SDL2 compiled and linked versions mismatch. Errors or "
                 "crashes may be occurred: "
              << SDL_GetError() << std::endl;
  }
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

GLuint loadShader(const char *shaderSrc, GLenum type) {
  GLuint shader;
  GLint compiled;
  shader = glCreateShader(type);
  GL_CHECK();
  if (shader == 0)
    return 0;
  glShaderSource(shader, 1, &shaderSrc, NULL);
  GL_CHECK();
  glCompileShader(shader);
  GL_CHECK();
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  GL_CHECK();
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    GL_CHECK();
    if (infoLen > 1) {
      char infoLog[infoLen] = {0};
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      GL_CHECK();
      std::string log = infoLog;
      std::cerr << "Error compiling shader: " << log;
    }
    glDeleteShader(shader);
    GL_CHECK();
    return 0;
  }
  return shader;
}

Uint32 initWindow(int height, int width) {
  SDL_Window *window;
  Uint32 id;
  window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, height, width,
                            SDL_WINDOW_OPENGL);
  if (window == nullptr) {
    std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
    return 0;
  }
  id = SDL_GetWindowID(window);
  //---------------------------------------------------------------------------/
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (gl_context == nullptr) {
    std::cerr << "Could not create GL_context for specified window: "
              << SDL_GetError() << std::endl;
    return 0;
  }
  //---------------------------------------------------------------------------/
  int gl_major{};
  int gl_minor{};
  if ((SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major)) != 0) {
    std::cerr << "Could not get GL attribute: " << SDL_GetError() << std::endl;
  }
  if ((SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor) != 0)) {
    std::cerr << "Could not get GL attribute: " << SDL_GetError() << std::endl;
  }
  std::cout << "OpenGL current version: " << gl_major << '.' << gl_minor
            << std::endl;
  if (gl_major <= 2 && gl_minor < 1) {
    std::cerr << "OpenGL current version: " << gl_major << '.' << gl_minor
              << std::endl
              << "need at least version: 2.1" << std::endl;
    return 0;
  }
  //---------------------------------------------------------------------------/
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "GLEW initialization failed. " << glewGetErrorString(err)
              << std::endl;
    return 0;
  }

  //--------------------------------------------------------------------------/

  GLchar vShaderStr[] = "attribute vec4 vPosition; \n"
                        "void main() \n"
                        "{ \n"
                        " gl_Position = vPosition; \n"
                        "} \n";
  GLchar fShaderStr[] = "precision mediump float; \n"
                        "void main() \n"
                        "{ \n"
                        " gl_FragColor = vec4(0.1, 0.014, 0.7, 1.0); \n"
                        "} \n";
  GLint linked;
  GLuint vertexShader = loadShader(vShaderStr, GL_VERTEX_SHADER);
  GL_CHECK();
  if (vertexShader == 0) {
    return 0;
  }
  GLuint fragmentShader = loadShader(fShaderStr, GL_FRAGMENT_SHADER);
  GL_CHECK();
  if (fragmentShader == 0) {
    return 0;
  }
  GLuint programObject = glCreateProgram();
  GL_CHECK();
  if (programObject == 0) {
    std::cerr << "Failed to create GL program" << std::endl;
    return 0;
  }
  glAttachShader(programObject, vertexShader);
  GL_CHECK();
  glAttachShader(programObject, fragmentShader);
  GL_CHECK();
  glBindAttribLocation(programObject, 0, "vPosition");
  GL_CHECK();
  glLinkProgram(programObject);
  GL_CHECK();
  glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
  GL_CHECK();
  if (!linked) {
    GLint infoLen = 0;
    glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
    GL_CHECK();
    if (infoLen > 1) {
      char infoLog[infoLen] = {};
      glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
      GL_CHECK();
      std::string log = infoLog;
      std::cout << "Error linking program: " << log << std::endl;
    }
    glDeleteProgram(programObject);
    GL_CHECK();
    return 0;
  }
  glUseProgram(programObject);
  GL_CHECK();
  /*
  // forked version. uncomment to use
   GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
   GL_CHECK();
   std::string vertex_shader_src = R"(
    attribute vec2 a_position;
    void main()
    {
        gl_Position = vec4(a_position, 0.0, 1.0);
    }
    )";
   const char *source = vertex_shader_src.data();
   glShaderSource(vert_shader, 1, &source, nullptr);
   GL_CHECK();

   glCompileShader(vert_shader);
   GL_CHECK();

   GLint compiled_status = 0;
   glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &compiled_status);
   GL_CHECK();
   if (compiled_status == 0) {
     GLint info_len = 0;
     glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &info_len);
     GL_CHECK();
     std::vector<char> info_chars(info_len);
     glGetShaderInfoLog(vert_shader, info_len, NULL, info_chars.data());
     GL_CHECK();
     glDeleteShader(vert_shader);
     GL_CHECK();

     std::string shader_type_name = "vertex";
     std::cerr << "Error compiling shader(vertex)\n"
               << vertex_shader_src << "\n"
               << info_chars.data();
     return 0;
   }

   // create fragment shader

   GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
   GL_CHECK();
   std::string fragment_shader_src = R"(
    void main()
    {
        gl_FragColor = vec4(0.0, 0.0, 0.7, 1.0);
    }
    )";
   source = fragment_shader_src.data();
   glShaderSource(fragment_shader, 1, &source, nullptr);
   GL_CHECK();

   glCompileShader(fragment_shader);
   GL_CHECK();

   compiled_status = 0;
   glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled_status);
   GL_CHECK();
   if (compiled_status == 0) {
     GLint info_len = 0;
     glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_len);
     GL_CHECK();
     std::vector<char> info_chars(info_len);
     glGetShaderInfoLog(fragment_shader, info_len, NULL, info_chars.data());
     GL_CHECK();
     glDeleteShader(fragment_shader);
     GL_CHECK();

     std::cerr << "Error compiling shader(fragment)\n"
               << vertex_shader_src << "\n"
               << info_chars.data();
     return 0;
   }

   // now create program and attach vertex and fragment shaders

   GLuint program_id_ = glCreateProgram();
   GL_CHECK();
   if (0 == program_id_) {
     std::cerr << "failed to create gl program";
     return 0;
   }

   glAttachShader(program_id_, vert_shader);
   GL_CHECK();
   glAttachShader(program_id_, fragment_shader);
   GL_CHECK();

   // bind attribute location
   glBindAttribLocation(program_id_, 0, "a_position");
   GL_CHECK();
   // link program after binding attribute locations
   glLinkProgram(program_id_);
   GL_CHECK();
   // Check the link status
   GLint linked_status = 0;
   glGetProgramiv(program_id_, GL_LINK_STATUS, &linked_status);
   GL_CHECK();
   if (linked_status == 0) {
     GLint infoLen = 0;
     glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &infoLen);
     GL_CHECK();
     std::vector<char> infoLog(infoLen);
     glGetProgramInfoLog(program_id_, infoLen, NULL, infoLog.data());
     GL_CHECK();
     std::cerr << "Error linking program:\n" << infoLog.data();
     glDeleteProgram(program_id_);
     GL_CHECK();
     return 0;
   }

   // turn on rendering with just created shader program
   glUseProgram(program_id_);
   GL_CHECK();
*/
  return id;
}

void renderTriangle(const triangle &t) {
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), &t.v[0]);
  GL_CHECK();
  glEnableVertexAttribArray(0);
  GL_CHECK();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  GL_CHECK();
  return;
}

void swapBuffers(Uint32 id) {
  SDL_Window *window = SDL_GetWindowFromID(id);
  SDL_GL_SwapWindow(window);
  glClearColor(0.15, 0.f, 0.f, 0.0f);
  GL_CHECK();
  glClear(GL_COLOR_BUFFER_BIT);
  GL_CHECK();
}

std::string getEvent() {

  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.key.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        return "QUIT";
        break;
      case SDLK_RETURN:
        return "START";
        break;
      case SDLK_BACKSPACE:
        return "SELECT";
        break;
      case SDLK_UP:
        return "UP";
        break;
      case SDLK_DOWN:
        return "DOWN";
        break;
      case SDLK_LEFT:
        return "LEFT";
        break;
      case SDLK_RIGHT:
        return "RIGHT";
        break;
      case SDLK_SPACE:
        return "FIRE1";
        break;
      case SDLK_LCTRL:
        return "FIRE2";
        break;
      }
    }
  }
  return "";
}

void shutDown(Uint32 id) {
  std::cout << "System is shutting down" << std::endl;
  SDL_Delay(2000);
  SDL_Window *window = SDL_GetWindowFromID(id);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
