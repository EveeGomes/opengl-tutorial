#include "SDL.h"
#include <iostream>

// Globals (prefixed with a g)
int gScreenHeight = 640;
int gScreenWidth = 480;
SDL_Window* gGraphicApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

bool gQuit = false; // if true, quit

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
                                                0, 0, 
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
   if (gOpenGLContext)
   {
      std::cout << "OpenGL context not available\n";
      exit(1);
   }
}

void Input()
{

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