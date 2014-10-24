#version 330 core
/// @brief a basic unshaded solid colour shader

layout (location = 0)out vec4 fragColour;


/// @brief the colour to shade draw with
uniform vec4 Colour;

void main ()
{
  fragColour = Colour;
}

