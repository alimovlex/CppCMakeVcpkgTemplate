/*  GLEW+GLFW spinning triangle example
*  Compile with:
*      g++ spin_triangle.cpp -o spin_triangle -lglfw -lGLEW -lGL
*  (On Windows link against opengl32.lib, glew32.lib, glfw3.lib, etc.)
*
*  Requires:
*      GLEW, GLFW, OpenGL 3.3+ (core profile)
*/

#include <iostream>
#include <stdexcept>
#include <vector>
#include <chrono>          // for timing

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

/* --------------------------------------------------------------------- */
/* Helper: shader compilation & program linking                          */
/* --------------------------------------------------------------------- */
GLuint CompileShader(const char *source, GLenum type)
{
 GLuint shader = glCreateShader(type);
 glShaderSource(shader, 1, &source, nullptr);
 glCompileShader(shader);

 GLint ok;
 glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
 if (!ok)
 {
   char log[512];
   glGetShaderInfoLog(shader, 512, nullptr, log);
   throw std::runtime_error(std::string("Shader compilation failed: ") + log);
 }
 return shader;
}

GLuint CreateShaderProgram(const char *vertSrc, const char *fragSrc)
{
 GLuint vs = CompileShader(vertSrc, GL_VERTEX_SHADER);
 GLuint fs = CompileShader(fragSrc, GL_FRAGMENT_SHADER);

 GLuint program = glCreateProgram();
 glAttachShader(program, vs);
 glAttachShader(program, fs);
 glLinkProgram(program);

 GLint ok;
 glGetProgramiv(program, GL_LINK_STATUS, &ok);
 if (!ok)
 {
   char log[512];
   glGetProgramInfoLog(program, 512, nullptr, log);
   throw std::runtime_error(std::string("Program linking failed: ") + log);
 }

 glDeleteShader(vs);
 glDeleteShader(fs);

 return program;
}

/* --------------------------------------------------------------------- */
/* Vertex data for a triangle (position + colour)                        */
/* --------------------------------------------------------------------- */
static const GLfloat vertices[] = {
   //  Position     //  Colour
   0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // Top (red)
   -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // Left (green)
   0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // Right (blue)
};

static const char *vertexShaderSrc = R"(
   #version 330 core
   layout (location = 0) in vec3 inPos;
   layout (location = 1) in vec3 inCol;

   uniform mat4 model;          // Rotation matrix

   out vec3 fragCol;

   void main()
   {
       gl_Position = model * vec4(inPos, 1.0);
       fragCol = inCol;
   }
)";

static const char *fragmentShaderSrc = R"(
   #version 330 core
   in vec3 fragCol;
   out vec4 outColor;

   void main()
   {
       outColor = vec4(fragCol, 1.0);
   }
)";

/* --------------------------------------------------------------------- */
/* Helper to build a rotation matrix (column-major)                     */
/* --------------------------------------------------------------------- */
void buildRotationMatrix(float angle, float *matrix)
{
 // 4x4 rotation around Z‑axis
 float c = cosf(angle);
 float s = sinf(angle);

 // column‑major order
 matrix[0]  =  c;  matrix[4] =  s;  matrix[8]  = 0.0f; matrix[12] = 0.0f;
 matrix[1]  = -s;  matrix[5] =  c;  matrix[9]  = 0.0f; matrix[13] = 0.0f;
 matrix[2]  = 0.0f;matrix[6] = 0.0f;matrix[10] = 1.0f; matrix[14] = 0.0f;
 matrix[3]  = 0.0f;matrix[7] = 0.0f;matrix[11] = 0.0f; matrix[15] = 1.0f;
}

/* --------------------------------------------------------------------- */
/* Main entry point                                                      */
/* --------------------------------------------------------------------- */
int main()
{
 /* -------- 1. Init GLFW ------------------------------------------- */
 if (!glfwInit())
 {
   std::cerr << "Error: could not initialise GLFW\n";
   return EXIT_FAILURE;
 }

 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
 glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
 glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 // macOS: glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

 GLFWwindow* window = glfwCreateWindow(800, 600,
                                       "GLEW Spinning Triangle", nullptr, nullptr);
 if (!window)
 {
   std::cerr << "Error: could not create a window\n";
   glfwTerminate();
   return EXIT_FAILURE;
 }

 glfwMakeContextCurrent(window);

 /* -------- 2. Init GLEW ------------------------------------------- */
 glewExperimental = GL_TRUE;
 GLenum err = glewInit();
 if (GLEW_OK != err)
 {
   std::cerr << "Error: could not initialise GLEW: "
             << glewGetErrorString(err) << "\n";
   glfwDestroyWindow(window);
   glfwTerminate();
   return EXIT_FAILURE;
 }

 /* -------- 3. Create shader program -------------------------------- */
 GLuint shaderProg = 0;
 try
 {
   shaderProg = CreateShaderProgram(vertexShaderSrc, fragmentShaderSrc);
 }
 catch (const std::exception &e)
 {
   std::cerr << e.what() << "\n";
   glfwDestroyWindow(window);
   glfwTerminate();
   return EXIT_FAILURE;
 }

 /* -------- 4. Setup VAO + VBO ------------------------------------- */
 GLuint VAO, VBO;
 glGenVertexArrays(1, &VAO);
 glGenBuffers(1, &VBO);

 glBindVertexArray(VAO);

 glBindBuffer(GL_ARRAY_BUFFER, VBO);
 glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

 // Position attribute
 glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                       6 * sizeof(float), (void*)0);
 glEnableVertexAttribArray(0);

 // Colour attribute
 glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                       6 * sizeof(float), (void*)(3 * sizeof(float)));
 glEnableVertexAttribArray(1);

 glBindVertexArray(0);

 /* -------- 5. Get uniform location --------------------------------- */
 GLint uniModel = glGetUniformLocation(shaderProg, "model");
 if (uniModel == -1)
 {
   std::cerr << "Warning: 'model' uniform not found in shader\n";
 }

 /* -------- 6. Timing variables ------------------------------------- */
 auto start = std::chrono::high_resolution_clock::now();

 /* -------- 7. Render loop ------------------------------------------ */
 while (!glfwWindowShouldClose(window))
 {
   /* Poll events */
   glfwPollEvents();

   /* Clear screen */
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   /* ---- Compute rotation ---- */
   auto now = std::chrono::high_resolution_clock::now();
   std::chrono::duration<float> elapsed = now - start;
   float angle = elapsed.count();          // radians: 1 rad/s

   float modelMat[16];
   buildRotationMatrix(angle, modelMat);

   /* ---- Render triangle ---- */
   glUseProgram(shaderProg);
   glUniformMatrix4fv(uniModel, 1, GL_FALSE, modelMat);
   glBindVertexArray(VAO);
   glDrawArrays(GL_TRIANGLES, 0, 3);
   glBindVertexArray(0);

   /* Swap buffers */
   glfwSwapBuffers(window);
 }

 /* -------- 8. Cleanup --------------------------------------------- */
 glDeleteVertexArrays(1, &VAO);
 glDeleteBuffers(1, &VBO);
 glDeleteProgram(shaderProg);

 glfwDestroyWindow(window);
 glfwTerminate();

 return EXIT_SUCCESS;
}
