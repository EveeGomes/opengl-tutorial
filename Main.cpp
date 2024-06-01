#include "SDL.h"
#include <iostream>

// Globals (prefixed with a g)
int gScreenHeight = 480;
int gScreenWidth = 640;
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
}

void Input()
{
   // For handling inputs in SDL, we'll use events
   SDL_Event e;
   // constantly pull and see if there are any events here
   while (SDL_PollEvent(&e) != 0)
   {
      // if there's some event to look at, we'll handle it
      // for now, the only event that we'll handle is if the user exists the program
      if (e.type == SDL_QUIT)
      {
         // "terminate the program"
         std::cout << "Goodbye!" << std::endl;
         gQuit = true;
         // The program will actually continue and run through the gQuit input,
         // and then terminate out of the MainLoop() so it can get completed
         // and then it goes to the CleanUp().
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
   while (!gQuit) // since it's set to false, ! changes it and it turns true! So when it's set to true in some point of the code, the ! will turn into false and the while exits!
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