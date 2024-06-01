#include "SDL.h"
#include <glad/glad.h>
#include <iostream>

// Globals (prefixed with a g)
int gScreenHeight = 480;
int gScreenWidth = 640;
SDL_Window* gGraphicApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

bool gQuit = false; // if true, quit

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

   MainLoop();

   CleanUp();

   return 0;
}