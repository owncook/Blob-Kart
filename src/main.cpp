#include "engine.h"

int main(int argc, char *argv[]) {

    Engine engine;
//    string command = "python3 ../play_music.py";
//    system(command.c_str());

    while (!engine.shouldClose()) {
        engine.processInput();
        engine.update();
        engine.render();
    }

    string command = "pkill -f play_music.py";
    system(command.c_str());

    glfwTerminate();
    return 0;
}
