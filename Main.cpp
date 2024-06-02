#include "SDL.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <string>

// Globals (prefixed with a g) - globals as we're learning
int gScreenHeight = 480;
int gScreenWidth = 640;
SDL_Window* gGraphicApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
bool gQuit = false; // if true, quit

// In OpenGL GLuint are used as sort of identifiers for some objects because this is a C based API
// VAO:
GLuint gVertexArrayObject = 0;
// VBO:
GLuint gVertexBufferObject = 0;
// Program Object (for our shaders) - another way to say that's our graphics pipeline, something that has a handle to a pipeline that we compile that has the vertex shader and fragment shader
GLuint gGraphicsPipelineShaderProgram = 0;

// Vertex Shader
// Vertex shader executes once per vertex, and will be in charge of the final position of the vertex
const std::string gVertexShaderSource =
"#version 410 core\n"
"in vec4 position;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
"}\n";

// Fragment Shader
// The fragment shader executes once per fragment (i.e. roughly for every pixel that will be rasterized), and in part determines the final color that will be sent to the screen.
const std::string gFragmentShaderSource =
"#version 410 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
"}\n";


// Where the actual compilation happens
GLuint CompileShader(GLuint Type, const std::string& Source)
{
   // We want to create a shader object based on which enum we have:
   GLuint shaderObject;

   if (Type == GL_VERTEX_SHADER)
   {
      // Create a vertex shader
      shaderObject = glCreateShader(GL_VERTEX_SHADER); // these checking steps aren't necessary, it could be only these creation lines, but they help on later debugging!
   }
   else if (Type == GL_FRAGMENT_SHADER)
   {
      shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
   }

   // the Source needs to be passed as const char array because the API doesn't have strings!
   const char* src = Source.c_str(); // get the c string version of the string
   glShaderSource(shaderObject, 1, &src, nullptr);
   // Once we have the shader we can now compile the actual shader object itself now that we have the source code there
   glCompileShader(shaderObject);

   return shaderObject;
}

// Returns the handle (a GLuint) to the actual GPU shader program
// Its job is to take whatever the result of compiling a vertex shader and assembling them in some way
GLuint CreateShaderProgram(const std::string& vertexShaderSource,
                           const std::string& fragmentShaderSource)
{
   // This is going to be our pipeline. glCreateProgram() returns an empty object which we'll fill in the vertex and fragment shaders
   GLuint programObject = glCreateProgram();
   GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource); // create another function (commonly seen in libraries), which takes the type of shader we want to build (as an enum) and the vertex shader source code
   GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource); // same here
   
   // Attach the shaders
   glAttachShader(programObject, myVertexShader);
   glAttachShader(programObject, myFragmentShader);
   // Link them together in the program object
   glLinkProgram(programObject);

   // Validate the program
   glValidateProgram(programObject);
   // in another lesson: detach and delete the shader using glDetachShader and glDeleteShader

   return programObject;
}

// Responsible for creating a pipeline with a vertex and a fragment shader once we have the actual geometry
void CreateGraphicsPipeline()
{
   // We need somewhere to hold the graphics pipeline
   // Create the shader
   // Since shaders themselves are just text information that we gonna compile (in the CompileShader method in glShaderSource(etc)), we're going to create two global strings
   gGraphicsPipelineShaderProgram = CreateShaderProgram(gVertexShaderSource, gFragmentShaderSource); // Create this function above
}

// Try to run some opengl function to check if it's properly set
// Turns out it need to get the opengl library. In the video he suggests using Glad tool.
// That tool will get us a header that had all the openGL functions provided.
void GetOpenGLVersionInfo()
{
   // Have some cout lines only to check openGL functions. It also works for sending these information for others whenever something isn't working properly
   std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
   std::cout << "Renderder: " << glGetString(GL_RENDERER) << std::endl;
   std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
   std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

// Responsible for getting some vertex data on our GPU
void VertexSpecification()
{
   // The goal here is to create some vertices (so it can be done on the CPU side)

   // Using opengl floats is good practice
   // Use initialization list to give this vector some values
   // Lives on the CPU
   const std::vector<GLfloat> vertexPosition
   {
      // x    y     z
      -0.8f, -0.8f, 0.0f, // vertex 1
      0.8f, -0.8f, 0.0f,  // vertex 2
      0.0f, 0.8f, 0.0f    // vertex 3
   };

   // Start setting things up on the GPU:
   // How to get to the GPU: set a vertex array object (VAO) then a vertex buffer object (VBO) - which will actually contain that vector's data

   // VBA:
   // 1. Generate the VBA (how many, where to place it - openGL uses an integer to act as a handle into some object)
   // Declare that object as global variable using opengl unsigned integer
   glGenVertexArrays(1, &gVertexArrayObject);
   // 2. Bind the object array. Binding means to select it; it's like saying "use this one that's been just created".
   glBindVertexArray(gVertexArrayObject);
   
   // Start generating the VBO
   // 1. Generate the VBO
   // The object is going to be global as well
   // We're passing the addresses because this is a C based API!!!
   glGenBuffers(1, &gVertexBufferObject);
   // 2. Select that buffer by calling the bind function
   // This function takes 2 parameters and the first one is the target which in docs.gl we can see them and their purpose!
   glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
   // 3. Populate with some data: params in docs.gl
   glBufferData(GL_ARRAY_BUFFER, // target
                vertexPosition.size() * sizeof(GLfloat), // size - size of our data in BYTES!! How big is the buffer
                vertexPosition.data(), // pointer to the data - since we're using a vector here we can pass .data() which returns a pointer to the raw array. If it was a regular array just pass in the array
                GL_STATIC_DRAW); // the last param is an enum that tells how we're gonna use the data - the triagles are gonna change a lot? are they gonna be streamed in? in our case only draw for now

   // Now that we have the data, how do get to that data here:
   // Enable an attribute
   glEnableVertexAttribArray(0);
   // How to use it:
   glVertexAttribPointer(0,
                         3, // x, y, z attributes
                         GL_FLOAT,
                         GL_FALSE,
                         0,
                         (void*)0);

   // Clean up - Close things when we're done
   // Use the bind function and pass in 0 since we don't want to bind to anything
   glBindVertexArray(0);
   // Disable anything we've enabled
   glDisableVertexAttribArray(0);
}

void InitializeProgram()
{
   // Initialize SDL:
   if (SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      std::cout << "SDL2 could not initialize video subsystem" 
                << std::endl;
      exit(1);
   }

   // After initializing SDL and before creating the window, set some opengl properties/attributes
   
   // Set version 4.1 (working on Mac, Linux, Windows)
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

   // Use PROFILE_CORE because it disables deprecated functions which guarantees old and unsupported functions won't get used
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

   // Double buffering allows for smoother transition of things
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   // Depth buffer gets 24 bits so we get more precision when determining objects or overlapping 
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

   // Create a window
   gGraphicApplicationWindow = SDL_CreateWindow("OpenGL Window",
                                                10, 40, 
                                                gScreenWidth, gScreenHeight,
                                                SDL_WINDOW_OPENGL);

   if (gGraphicApplicationWindow == nullptr)
   {
      std::cout << "SDL_Window was not able to be created"
                << std::endl;
      exit(1);
   }

   // Setup opengl context
   gOpenGLContext = SDL_GL_CreateContext(gGraphicApplicationWindow);
   if (gOpenGLContext == nullptr)
   {
      std::cout << "OpenGL context not available\n";
      exit(1);
   }

   // Initialize the Glad library
   if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) // loads up a bunch of function pointers and then retrieves their addresses
   {
      std::cout << "Glad was not initialized" << std::endl;
      exit(1);
   }

   // Call GetOpenGLVersionInfo() to check if the openGL commands are working
   GetOpenGLVersionInfo();
}

void Input()
{
   // For handling inputs in SDL, we'll use events
   SDL_Event e;
   // constantly pull and see if there are any events here
   while (SDL_PollEvent(&e) != 0)
   {
      if (e.type == SDL_QUIT)
      {
         // "terminate the program"
         std::cout << "Goodbye!" << std::endl;
         gQuit = true;
      }
   }
}

void PreDraw()
{

}

void Draw()
{

}

// It'll handle input, do some updates baased on the inputs, and render (which can be broken up into different stages that take place before render per se!)
void MainLoop()
{
   while (!gQuit)
   {
      Input();
      
      PreDraw();

      Draw();

      // Update the screen
      SDL_GL_SwapWindow(gGraphicApplicationWindow);
   }
}

// Removes all the setup that has been used such as SDL, deallocate any memory used
void CleanUp()
{
   // Destroy the SDL window
   SDL_DestroyWindow(gGraphicApplicationWindow);
   SDL_Quit();
}


int main(int argc, char* argv[])
{
   // Initial steps for having a graphical application:

   InitializeProgram();

   VertexSpecification();

   CreateGraphicsPipeline();

   MainLoop();

   CleanUp();

   return 0;
}