#include "sketch2d/sketch2d.h"

PImage img1, img2;
PFont font;

void setup()
{
    size(displayWidth, displayHeight);
    noCursor();
    img1 = loadImage("image9.jpg");
    img2 = loadImage("image10.jpg");
    font = loadFont("Roboto-Regular.ttf");
}

void draw()
{
    background(gray(122));

    if (mousePressed)
    {
        image(img1, mouseX, mouseY, img1.width, img1.height);
    }
    else
    {
        image(img2, mouseX, mouseY, img2.width, img2.height);
    }

    if (keyPressed)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            quit();
        }
        else if (key == GLFW_KEY_SPACE)
        {
            saveFrame("screenshot.png");
        }
    }

    textFont(font);
    textAlign(NVG_ALIGN_CENTER);
    textSize(30);
    textLeading(5);
    text("test everything here", width/2, height/2);
}

void shutdown()
{
}

int main() {
    return sketch2d_main(0,0);
}
