/**
 * @file ModelDestroyer.cpp
 * @author Ryan Purse
 * @date 18/05/2022
 */


#include "ModelDestroyer.h"
#include "stb_image.h"

void destroy::windowIcon(GLFWimage &icon)
{
    if (icon.pixels != nullptr)
        stbi_image_free(icon.pixels);
    icon.pixels = nullptr;
    icon.height = 0;
    icon.width = 0;
}
