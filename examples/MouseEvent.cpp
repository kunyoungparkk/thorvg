#include "Example.h"

/************************************************************************/
/* ThorVG Drawing Contents                                              */
/************************************************************************/

struct UserExample : tvgexam::Example
{
    tvg::Shape* cursorShape;
    tvg::Shape* lineShape;
    bool isDrawing = false;

    bool content(tvg::Canvas* canvas, uint32_t w, uint32_t h) override
    {
        //Create a Scene
        auto scene = tvg::Scene::gen();

        //Create a Shape
        cursorShape = tvg::Shape::gen();
        //Appends Paths
        cursorShape->moveTo(-50, -50);
        cursorShape->lineTo(50, -50);
        cursorShape->lineTo(50, 50);
        cursorShape->lineTo(-50, 50);
        cursorShape->close();
        cursorShape->fill(255, 0, 0);
        scene->push(cursorShape);

        lineShape = tvg::Shape::gen();
        lineShape->moveTo(0, 0);
        lineShape->strokeWidth(10);
        lineShape->strokeFill(0, 255, 0);
        scene->push(lineShape);

        //Draw the Scene onto the Canvas
        canvas->push(scene);

        return true;
    }

    bool motion(tvg::Canvas* canvas, int32_t x, int32_t y) override
    {
        if (isDrawing) {
            lineShape->lineTo(x, y);
        }
        cursorShape->translate(x,y);
        canvas->update();
        return true; 
    }
    bool clickdown(tvg::Canvas* canvas, int32_t x, int32_t y) override 
    { 
        isDrawing = true;
        cursorShape->fill(0, 0, 255);
        lineShape->moveTo(x, y);
        canvas->update();
        return true; 
    }
    bool clickup(tvg::Canvas* canvas, int32_t x, int32_t y) override 
    {
        isDrawing = false;
        cursorShape->fill(255, 0, 0);
        canvas->update();
        return true; 
    }
};


/************************************************************************/
/* Entry Point                                                          */
/************************************************************************/

int main(int argc, char **argv)
{
    return tvgexam::main(new UserExample, argc, argv, true, 1024, 1024);
}
