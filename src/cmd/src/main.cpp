#include <engine/core/scene.h>

#include <engine/core/image.h>

int main()
{
    fumeng::engine::Scene scn;

    fumeng::engine::Image img;

    img.load_from_file("C:\\Users\\etwll\\Pictures\\Camera Roll\\Snipaste_2021-08-07_20-38-55.png");

    img.save_to_file("C:\\Users\\etwll\\Pictures\\Camera Roll\\Snipaste.png");

    scn.test();
    
    return 0;
}