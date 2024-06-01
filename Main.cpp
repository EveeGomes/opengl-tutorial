#include "SDL.h"
#include <stdio.h>

void InitializeProgram()
{
   
}

// It'll handle input, do some updates baased on the inputs, and render (which can be broken up into different stages that take place before render per se!)
void MainLoop()
{

}

// Removes all the setup that has been used such as SDL, deallocate any memory used
void CleanUp()
{

}


int main(int argc, char* argv[])
{
   // Initial steps for having a graphical application:

   InitializeProgram();

   MainLoop();

   CleanUp();

   return 0;
}