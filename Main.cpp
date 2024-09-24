// Third Party Libraries
#include "SDL.h"
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// C++ Standard Libraries
#include <iostream>
#include <vector>
#include <string>
#include <fstream>


// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV Globals VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

/** Setting up SDL */
// Screen Dimensions
int gScreenHeight = 480;
int gScreenWidth = 640;
SDL_Window* gGraphicApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// Main loop flag
bool gQuit = false; // if true, quit

/** Pipeline */
// Shader
/** 
* The following stores the unique id for the graphics pipeline program object 
*  that will be used for our OpenGL draw calls. 
* Program Object (for our shaders) - another way to say that's our graphics 
*  pipeline, something that has a handle to a pipeline that we compile that 
*  has the vertex shader and fragment shader.
*/
GLuint gGraphicsPipelineShaderProgram = 0;

/** OpenGL Objects */
/** 
* VAO encapsulate all of the items needed to render an object.
* For example, we may have multiple vertex buffer objects (VBO) related to
*  rendering one object. The VAO allows us to setup the OpenGL state to render
*  that object using the correct layout and correct buffers with one call after 
*  being setup.
* 
* In OpenGL GLuint are used as sort of identifiers for some objects because 
*  this is a C based API.
*/
// VAO:
GLuint gVertexArrayObject = 0;

/** 
* VBO store information relating to vertices(e.g.positions, normals, textures).
* VBOs are our mechanism for arraging geometry on the GPU.
*/
// VBO:
GLuint gVertexBufferObject = 0;

/** 
* Index Buffer Object(IBO) 
* This is used to store the array of indices that we want to draw from, when we
*  do indexed drawing.
*/
GLuint gIndexBufferObject = 0;

/**
* Shaders
* Here we setup two shaders, a vertex shader and a fragment shader.
* At a minimum, every Modern OpenGL program needs a vertex and a fragment shader.
*/
float g_uOffset = 0.0f;

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Globals ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// VVVVVVVVVVVVVVVVVVVVVVVVVV Error Handling Routines VVVVVVVVVVVVVVVVVVVVVVVVVV

/** 
* From https://www.youtube.com/watch?v=uTidLlObMMw&list=PLvv0ScY6vfd9zlZkIIqGDeG5TUWswkMox&index=17&ab_channel=MikeShah
* Working with glGetError(), which returns the value of the error flag. See @https://docs.gl/gl4/glGetError
* It looks at the openGL state machine and reports any errors to us.
* This function however only gets cleaned out to record another error, once it's called. Therefore we'll need some routine that
*  sort of clears this error state:
*/

/** 
* GLClearAllErrors() is static so it's limited to this file (not strictly 
*  necessary).
* It'll constinuously call glGetError so it gets cleared out and can report another error if any, until there's no more errors.
*/
static void GLClearAllErrors()
{
   // Consecutively call glGetError over and over again until it's in a state
   //  that is clear, that is there's no errors here. 
   while (glGetError() != GL_NO_ERROR) {}
}

/** 
* Improve the function by adding more information that can be visualized in the output:
* @param Function The actual function call
* @param Line The line number. We would use the macro __LINE__
* @return true if an error occurs.
*/
static bool GLCheckErrorStatus(const char* Function, int Line)
{
   // glGetError() returns a GLenum:
   while (GLenum error = glGetError())
   {
      // so, if we retrieve an error, return our status
      std::cout << "OpenGL Error: " << error 
                << "\tLine: " << Line 
                << "\tFunction: " << Function 
                << std::endl;
      return true;
   }
   return false;
}

/**
* Those functions should be used together. We'd have to call the clear function
*  and then check the error status, but only after we make a function call.
* So programmers normally create different kinds of macros to handle this.
*
* @param x The function to be called
* Then, call the clear function,
*  then do the function call x,
*  and then check for the errors
* 
* That macro is an error checker.
* To use it, we would go to where we might have the problem (in the lesson he
*  shows the quad isn't being drawn, so he goes to the Draw() method and wrap
*  some calls into the macro:
*     GLCheck(glBindVertexArray(gVertexArrayObject);)
*     GLCheck(glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);)
*     GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_INT, 0);)
*/
// To pass the arguments to GLCheckErrorStatus, use # since it's a little string
#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^^^^^^^^^^^^

/** 
* LoadShaderAsString takes a filepath as an argument and will read line by line
*  a file and return a string that is meant to be compiled at runtime for a
*  vertex, fragment, geometry, tesselation, or computer shader.
* E.g.
*  LoadShaderAsString("./shaders/filepath");
* @param Filename Path to the shader file
* @return Entire file stored as a single string
*/
std::string LoadShaderAsString(const std::string& Filename)
{
   // Resulting shader program loaded as a single string
   std::string Result = "";

   // Read the file line by line
   std::string Line = "";
   // Input file to bring in (@param: Read the content as a string)
   std::ifstream MyFile(Filename.c_str());

   // if the file was found, parse/read one line at a time
   if (MyFile.is_open())
   {
      while (std::getline(MyFile, Line))
      {
         // concatenate
         Result += Line + '\n';
      }
      // Close the file before leaving the if statement
      MyFile.close();
   }

   return Result;
}

/** 
* CompileShader will compile any valid vertex, fragment, geometry,
*  tesselation, or compute shader. E.g. Compile a vertex shader: 
*  CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*  Compile a fragment shader:
*  CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
* 
* @param Type: we use 'type' field to determine which shader we are
*  going to compile.
* @param Source: the shader source code.
* @return id of the shaderObject.
*/

// Where the actual compilation happens
GLuint CompileShader(GLuint Type, const std::string& Source)
{
   // We want to create a shader object based on which enum we have:
   GLuint shaderObject = 0; // is it ok to initialize to 0?

   if (Type == GL_VERTEX_SHADER)
   {
      // Create a vertex shader:
      // these checking steps aren't necessary,it could be only these 
      //  creation lines, but they help on later debugging!
      shaderObject = glCreateShader(GL_VERTEX_SHADER); 
   }
   else if (Type == GL_FRAGMENT_SHADER)
   {
      // Create a fragment shader:
      shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
   }

   // the Source needs to be passed as const char array because the API 
   //  doesn't have strings!
   const char* src = Source.c_str(); // get the c string version of the string
   glShaderSource(shaderObject, 1, &src, nullptr);
   // Once we have the shader we can now compile the actual shader object 
   //  itself now that we have the source code there.
   glCompileShader(shaderObject);

   /** Error checking */
   // Retrieve the result of our compilation
   int result = 0; // is it ok 0?
   // Our goal with glGetShaderiv is to retrieve the compilation status
   glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

   if (result == GL_FALSE)
   {
      int length;
      glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
      char* errorMessages = new char[length]; // could also use alloc here.
      glad_glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

      if (Type == GL_VERTEX_SHADER)
      {
         std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n"
                   << errorMessages
                   << "\n";
      }
      else if (Type == GL_FRAGMENT_SHADER)
      {
         std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n"
                   << errorMessages
                   << "\n";
      }

      // Reclaim our memory
      delete[] errorMessages;

      // Delete our broken shader
      glDeleteShader(shaderObject);
   }

   return shaderObject;
}

/** 
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex
*  Shader and a Fragment Shader
* 
* @param vertexShaderSource source code as string
* @param fragmentShaderSource source code as a string
* @return id of the program object
*/

// Returns the handle (a GLuint) to the actual GPU shader program.
// Its job is to take whatever the result of compiling a vertex shader and 
//  assembling them in some way.
GLuint CreateShaderProgram(const std::string& vertexShaderSource,
                           const std::string& fragmentShaderSource)
{
   // This is going to be our pipeline. glCreateProgram() returns an empty 
   //  object which we'll fill in the vertex and fragment shaders.
   // Create a new program object:
   GLuint programObject = glCreateProgram();
   // Compile our shaders:
   // Create another function (commonly seen in libraries), which takes the 
   //  type of shader we want to build (as an enum) and the vertex shader 
   //  source code.
   GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
   GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource); // same here
   
   // Attach the shaders (Link our two shader programs together. 
   //  Consider this the equivalent of taking two .cpp files, and linking them 
   //  into one executable file).
   glAttachShader(programObject, myVertexShader);
   glAttachShader(programObject, myFragmentShader);
   // Link them together in the program object
   glLinkProgram(programObject);

   // Validate the program
   glValidateProgram(programObject);
   
   // Once our final program object has been created, we can detach and then 
   //  delete our individual shaders.
   glDetachShader(programObject, myVertexShader);
   glDetachShader(programObject, myFragmentShader);
   // Delete the individual shaders once we are done
   glDeleteShader(myVertexShader);
   glDeleteShader(myFragmentShader);

   return programObject;
}

// Responsible for creating a pipeline with a vertex and a fragment shader 
//  once we have the actual geometry.
void CreateGraphicsPipeline()
{
   // Instead of loading the source code, we'll read in the files:
   std::string VertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
   std::string FragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");

   // We need somewhere to hold the graphics pipeline
   // Create the shader
   // Since shaders themselves are just text information that we gonna compile 
   //  (in the CompileShader method in glShaderSource(etc)), we're going to 
   //  create two global strings.
   gGraphicsPipelineShaderProgram = CreateShaderProgram(VertexShaderSource, 
                                                        FragmentShaderSource);
}

// Try to run some opengl functions to check if it's properly set.
// Turns out it needs to get the opengl library. In the video he suggests using 
// Glad tool.
// That tool will get us a header that had all the openGL functions provided.
void GetOpenGLVersionInfo()
{
   // Have some cout lines only to check openGL functions. It also works for 
   //  sending these information for others whenever something isn't working 
   //  properly.
   std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
   std::cout << "Renderder: " << glGetString(GL_RENDERER) << std::endl;
   std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
   std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

/** 
* Setup your geometry during the vertex specification step
* 
* @return void
*/

// Responsible for getting some vertex data on our GPU
void VertexSpecification()
{
   /**
   * Geometry Data:
   * Here we are going to store x, y, and z position attributes within
   *  vertexPositions for the data.
   * For now, this information is just stored in the CPU, and we are going
   *  to store this data on the GPU shortly, in a call to glBufferData which
   *  will store the information into a vertex buffer object.
   * Note that he has segregated the data from the opengl calls which follow
   *  in this function.
   * It is not strictly necessary, but he finds the code is cleaner if opengl
   *  (GPU) related functions are packed closer together versus CPU operations.
   */

   // The goal here is to create some vertices (so it can be done on the CPU 
   //  side).

   // Using opengl floats is good practice
   // Use initialization list to give this vector some values
   // Lives on the CPU
   const std::vector<GLfloat> vertexData
   {
      /** 
      * The order in which the vertices are written don't really matter.
      * However, it'll follow a counterclockwise (CCW) order so if in the
      *  1st triangle it starts from the BL vertex, it'll then go to the 
      *  BR and then TL.
      * That's called the "Winding order" which is the direction that our
      *  vertices are laid out. It tells which direction is the front of
      *  the triangle. In this example, the triangle is facing out towards 
      *  us as the user.
      * OpenGL follows the right-hand coordinate system:
      *  thumb:  x
      *  index:  y
      *  middle: z
      * Also, to tell the winding order, we use the right hand and check
      *  the direction the fingers curl.
      *
      */
      // x     y     z
      // 0 - Vertex
      -0.5f, -0.5f, 0.0f, // Bottom Left vertex position
       1.0f,  0.0f, 0.0f, // BL Color
      // 1 - Vertex
       0.5f, -0.5f, 0.0f, // Bottom Right vertex position
       0.0f,  1.0f, 0.0f, // BR Color
      // 2 - Vertex
      -0.5f,  0.5f, 0.0f, // Top Left vertex position
       0.0f,  0.0f, 1.0f, // TL Color
      // 3 - Vertex
       0.5f,  0.5f, 0.0f, // Top Right vertex position
       1.0f,  0.0f, 0.0f, // TR Color
   };

   // Start setting things up on the GPU:
   // How to get to the GPU: set a vertex array object (VAO) then a vertex 
   //  buffer object (VBO) - which will actually contain the vector's data.

   /** 
   * Vertex Arrays Object (VAO) Setup:
   * Note: we can think of the VAO as a 'wrapper around' all of the VBOs,
   *  in the sense that it encapsulates all VBO state that we are setting up.
   *  Thus, it's also important that we glBindVertexArray (i.e. select the
   *  VAO we want to use) before our VBO operations.
   */

   // VAO:
   // 1. Generate the VAO (how many, where to place it - openGL uses an integer 
   //  to act as a handle into some object).
   // Declare that object as global variable using opengl unsigned integer
   glGenVertexArrays(1, &gVertexArrayObject);
   // 2. Bind the object array. Binding means to select it.
   //  It's like saying "use this one that's been just created".
   glBindVertexArray(gVertexArrayObject);
   

   // Start generating the Vertex Buffer Object (VBO)
   // 1. Generate/create a new VBO
   // Note: we'll see this pattern of code often in opengl of creating and 
   //  biding to a buffer.
   // The object is going to be global as well.
   // We're passing the addresses because this is a C based API!!!
   glGenBuffers(1, &gVertexBufferObject);
   // 2. Select that buffer by calling the bind function ('selecting the active 
   //  buffer object' that we want to work with in opengl).
   // This function takes 2 parameters and the first one is the target which in 
   //  docs.gl we can see them and their purpose!
   glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
   // 3. Populate with some data: params in docs.gl
   // As we have this currently binded buffer, we populate the data from our 
   //  'vertexPositions' (which is on the CPU), onto a buffer that will live on 
   //  the GPU!
   glBufferData(GL_ARRAY_BUFFER, // target (kind of buffer we are working with; e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
                vertexData.size() * sizeof(GLfloat), // size - size of our data in BYTES!! How big is the buffer
                vertexData.data(), // (raw array of data) pointer to the data - since we're using a vector here we can pass .data() which returns a pointer to the raw array. If it was a regular array just pass in the array
                GL_STATIC_DRAW // the last param is an enum that tells how we're gonna use the data - the triagles are gonna change a lot? are they gonna be streamed in? in our case only draw for now
               );
   // Now that we have the data, tell opengl 'how' the information in our 
   //  buffer will be used:

   /** 
   * Now, as we've removed repeated data from vertexData, the VBO will have less data stored as well, and to be able to use those data we'll need to set
   *  an Index Buffer Object (IBO) or Element Buffer Object (EBO) that consists of indices of elements in the VBO. The goal of this IBO is to select
   *  indices to draw a shape (in our case a triangle), but respecting the winding order!
   * For example, we could have an IBO as: 2, 0, 1, 3, 2, 1. There are many orders that could work as well.
   * This is a way of preventing sending too much information to the GPU.
   * 
   * Set the Index Buffer Object (IBO i.e. EBO)
   */
   
   /** 
   * Before step 3, we need to have data to send to the glBufferData function.
   * Since this is an Index buffer object, we'll use a vector of GLunint.
   * As for our convinience, we can use the initializing list and send the indices we've decided to use above.
   */
   const std::vector<GLuint> indexBufferData{ 2,0,1,3,2,1 };

   // 1. Generate a buffer, an IBO
   glGenBuffers(1, &gIndexBufferObject);
   // 2. Bind it
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
   // 3. Populate the buffer with some data. This essentially shipping data to the GPU!
   glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                indexBufferData.size() * sizeof(GLuint),
                indexBufferData.data(),
                GL_STATIC_DRAW
               );

   // Enable an attribute
   glEnableVertexAttribArray(0);
   /** 
   * For the specific attribute in our vertex specification, we use
   *  'glVertexAttribPointer' to figure out how we are going to move
   *  through the data.
   */
   // How to use it:
   glVertexAttribPointer(0, // Attribute 0 corresponds to the enabled glEnableVertexAttribArray. In the future we'll see in our vertex shader this also correspond to (layout=0) which selects these attributes.
                         3, // The number of components (e.g. x, y, z = 3 components/attributes)
                         GL_FLOAT, // Type
                         GL_FALSE, // Is the data normalized
                         sizeof(GL_FLOAT)*6, // Stride (how to get to the next component) -> in this example, we have x,y,z and r,g,b as 1 vertex, so we gotta read the x,y,z and hop 3 floats to get to the next vertex's x,y,z information!
                         (void*)0 // Offset (nothing changes since x,y,z info starts at the 0th position in the VBO)
                         );

   // Linking up the attributes in our VAO
   // Color information
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1,
                         3, // r, g, b
                         GL_FLOAT,
                         GL_FALSE,
                         sizeof(GL_FLOAT)*6,
                         (GLvoid*)(sizeof(GL_FLOAT)*3) // Since r,g,b is the "second" information in the vertex, it doesn't start at position 0, so we have to specify how many bytes we gotta jump from the position to the rgb data, in this case 3 floats. So we use: sizeof(GL_FLOAT)*3 instead of 0
                         );


   // Clean up - Close things when we're done
   // Use the bind function and pass in 0 since we don't want to bind to 
   //  anything (Unbind our currently bound VAO).
   glBindVertexArray(0);
   // Disable anything we've enabled (Disable any attributes we opened in 
   //  our Vertex Attribute Array, as we do not want to leave them open).
   glDisableVertexAttribArray(0);
   // Disable the second attribute
   glDisableVertexAttribArray(1);
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

   // After initializing SDL and before creating the window, set some opengl 
   //  properties/attributes.
   
   // Set version 4.1 (working on Mac, Linux, Windows)
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

   // Use PROFILE_CORE because it disables deprecated functions which guarantees
   //  old and unsupported functions won't get used
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

   // Double buffering allows for smoother transition of things
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   // Depth buffer gets 24 bits so we get more precision when determining
   //  objects or overlapping.
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

   // Create a window
   gGraphicApplicationWindow = SDL_CreateWindow("OpenGL Window",
                                                SDL_WINDOWPOS_UNDEFINED,
                                                SDL_WINDOWPOS_UNDEFINED, 
                                                gScreenWidth,
                                                gScreenHeight,
                                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

   if (gGraphicApplicationWindow == nullptr)
   {
      std::cout << "SDL_Window was not able to be created! SDL Error: "
                << SDL_GetError()
                << "\n";
      exit(1);
   }

   // Setup OpenGL Graphics Context (a big opengl object that encapsulates 
   //  everything).
   gOpenGLContext = SDL_GL_CreateContext(gGraphicApplicationWindow);
   if (gOpenGLContext == nullptr)
   {
      std::cout << "OpenGL context could not be created. SDL Error: "
                << SDL_GetError()
                << "\n";
      exit(1);
   }

   // Initialize the Glad library
   if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) // loads up a bunch of function 
                                                 //  pointers and then retrieves
                                                 //  their addresses.
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
         std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
         gQuit = true;
      }
   }

   // Retrieve keyboard state
   const Uint8* state = SDL_GetKeyboardState(NULL);
   if (state[SDL_SCANCODE_UP])
   {
      g_uOffset += 0.01f;
      std::cout << "g_uOffset: " << g_uOffset << std::endl;
   }
   if (state[SDL_SCANCODE_DOWN])
   {
      g_uOffset -= 0.01f;
      std::cout << "g_uOffset: " << g_uOffset << std::endl;
   }
}
/**
* Typically we will use this for setting some sort of 'state'
* Note: some of the calls may take at different stages (post-processing)
*  of the pipeline.
* @return void
*/

// Responsible for setting opengl state (that's how he suggests to be done, 
//  but things can be put into the Draw function as well).
void PreDraw()
{
   // Disable depth test and face culling
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);

   // Initialize clear color
   // Setup the viewport using the size of screen
   glViewport(0, 0, gScreenWidth, gScreenHeight);
   // Background color of our scene:
   glClearColor(1.f, 1.f, 0.f, 1.f);

   // Clear Color buffer and Depth buffer
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

   // Define the pipeline we're using to make it work
   // Use our shader
   glUseProgram(gGraphicsPipelineShaderProgram);
}

void Draw()
{
   // Make the Draw call and then the pipeline will be activated.
   // So in order to draw, we gotta figure out which vertex array object 
   //  are we gonna be using. So we set it up by using the Bind function to 
   //  select the VAO.
   // Enable our attributes
   glBindVertexArray(gVertexArrayObject);
   // Now, which buffer we wanna draw from
   // Select the vertex buffer object we want to enable
   glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

   /** 
   * As we're now using IBO, we need to draw differently!
   * Instead of using glDrawArrays, we'll use glDrawElements to render data!
   */
   glDrawElements(GL_TRIANGLES,
                  6,
                  GL_UNSIGNED_INT,
                  0
                 );

   // Stop using our current graphics pipeline
   // Note: this is not necessary if we only have one graphics pipeline
   glUseProgram(0);
}

// It'll handle input, do some updates based on the inputs, and render 
//  (which can be broken up into different stages that take place before render 
//  per se!).
void MainLoop()
{
   // While application is running
   while (!gQuit)
   {
      // Handle input
      Input();
      
      // Setup anything (i.e. OpenGL State) that needs to take place before draw
      //  calls.
      PreDraw();

      // Draw calls in OpenGL
      Draw();

      // Update the screen of our specified window
      SDL_GL_SwapWindow(gGraphicApplicationWindow);
   }
}

// Removes all the setup that has been used such as SDL, deallocate any memory 
//  used.
void CleanUp()
{
   // Destroy the SDL window
   SDL_DestroyWindow(gGraphicApplicationWindow);
   SDL_Quit();
}


int main(int argc, char* argv[])
{
   // Initial steps for having a graphical application:

   // 1. Setup the graphics program
   InitializeProgram();

   // 2. Setup our geometry
   VertexSpecification();

   // 3. Create our graphics pipeline
   //  - At a minimum, this means the vertex and fragment shader
   CreateGraphicsPipeline();

   // 4. Call the main application loop
   MainLoop();

   // 5. Call the cleanup function when our program terminates
   CleanUp();

   return 0;
}