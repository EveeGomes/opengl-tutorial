#include "SDL.h"
#include <iostream>

// Globals (prefixed with a g)
int gScreenHeight = 640;
int gScreenWidth = 480;
SDL_Window* gGraphicApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

void InitializeProgram()
{
   // Initialize SDL:
   if (SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      std::cout << "SDL2 could not initialize video subsystem" 
                << std::endl;
      exit(1);
   }

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
}

// It'll handle input, do some updates baased on the inputs, and render (which can be broken up into different stages that take place before render per se!)
void MainLoop()
{

}

// Removes all the setup that has been used such as SDL, deallocate any memory used
void CleanUp()
{
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