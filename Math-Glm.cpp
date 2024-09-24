#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

// glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_transform.hpp>

#include <glm/gtx/string_cast.hpp>

// C++ Standard Libraries
#include <iostream>

int main()
{
   // Create a 'vertex' (i.e. a point). This is the local coordinates.
   // 1.0f at the end is the 'w' coordinate. w = 1 means we have a position. w = 0 means direction
   glm::vec4 vertex{ 1.0f, 5.0f, 1.0f, 1.0f };

   // Create a model matrix for our geometry.
   // Initialize with '1' for identity matrix.
   // NOTE: do not count on GLM to provide you an identity matrix!
   glm::mat4 model{ 1.0f };

   // Perform some transformations (i.e. moving us in the world). We are now in 'world space'

   // Scaling Matrix
   glm::mat4 scalingMatrix = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.0f, 2.0f, 2.0f });

   // Rotation Matrix
   glm::mat4 rotationMatrix = glm::rotate(glm::mat4{ 1.0f }, glm::radians(180.0f), glm::vec3{ 0.f ,1.f ,0.f });

   // Tranlation Matrix
   glm::mat4 translationMatrix = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.f, 0.f, -2.f });

   // The order of operations DOES matter... Also it goes from right to left o.o
   // translate first, then rotate, then scale
   model = scalingMatrix * rotationMatrix * translationMatrix;

   // Trick to print off  each column
   std::cout << glm::to_string(model[0]) << std::endl;
   std::cout << glm::to_string(model[1]) << std::endl;
   std::cout << glm::to_string(model[2]) << std::endl;
   std::cout << glm::to_string(model[3]) << std::endl;

   // Now apply our 'model' matrix to the vertex
   glm::vec4 worldspace_vertex = (model * vertex);

   std::cout << '\n' << "our vertex in world space\n";
   std::cout << glm::to_string(worldspace_vertex) << std::endl;


   return 0;
}