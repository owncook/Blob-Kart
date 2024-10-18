#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <ctime>
#include <vector>
#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>
#include <SFML/Audio.hpp>

#include "shader/shaderManager.h"
#include "font/fontRenderer.h"
#include "shapes/rect.h"
#include "shapes/shape.h"

using std::vector, std::unique_ptr, std::make_unique, glm::ortho, glm::mat4, glm::vec3, glm::vec4;

/**
 * @brief The Engine class.
 * @details The Engine class is responsible for initializing the GLFW window, loading shaders, and rendering the game state.
 */
class Engine {
private:
    /// @brief The actual GLFW window.
    GLFWwindow* window{};

    sf::Music backgroundMusic;
    bool introFinished = false;

    /// @brief The width and height of the window.
    const unsigned int width = 1200, height = 800; // Window dimensions

    /// @brief Keyboard state (True if pressed, false if not pressed).
    /// @details Index this array with GLFW_KEY_{key} to get the state of a key.
    bool keys[1024];

    /// @brief Responsible for loading and storing all the shaders used in the project.
    /// @details Initialized in initShaders()
    unique_ptr<ShaderManager> shaderManager;

    /// @brief Responsible for rendering text on the screen.
    /// @details Initialized in initShaders()
    unique_ptr<FontRenderer> fontRenderer;

    // Shapes
    vector<vector<unique_ptr<Shape>>> carts;
    vector<unique_ptr<Shape>> checkers;
    vector<unique_ptr<Shape>> track;
    vector<unique_ptr<Shape>> checkpoints;
    unique_ptr<Shape> pond;

    vector<unique_ptr<Shape>> boost_panels;
    // vector<vector<unique_ptr<Shape>>> item_sets;
    // vector<unique_ptr<Shape>> item_shower;
    vector<unique_ptr<Shape>> startScreenShapes;

    // Character select screen
    vector<unique_ptr<Shape>> characters;
    vector<unique_ptr<Shape>> button;
    bool redSelect = false;
    bool blueSelect = false;
    vector<color> charColors = {color(.7,0,0), color(0.12, 0.25, 0.96)}; // Default red and blue
    // Red, orange, yellow, green, blue, purple, black, brown
    vector<color> blob_colors = {color(.7,0,0), color(0.96, 0.59, 0.12), color(0.96, 0.87, 0.12), color(0.18, 0.54, 0.16), color(0.08, 0.06, 0.67), color(0.69, 0.16, 0.89), color(0,0,0), color(0.36, 0.18, 0.02)};

    // End screen shapes
    vector<unique_ptr<Shape>> confetti;
    vector<unique_ptr<Shape>> podium;
    vector<unique_ptr<Shape>> winningBlobs;

    // Checkpoint and Lap counters
    vector<int> carCheckpoints = {2,2};
    vector<int> carLap = {0,0};

    float cart_speed_red;
    float cart_speed_blue;

    // Shaders
    Shader shapeShader;
    Shader textShader;

    double MouseX, MouseY;
    bool mousePressedLastFrame = false;

    // Timer
    unsigned long timer;
    unsigned long countdown;
    bool go = false;

    /// @note Call glCheckError() after every OpenGL call to check for errors.
    GLenum glCheckError_(const char *file, int line);
    /// @brief Macro for glCheckError_ function. Used for debugging.
#define glCheckError() glCheckError_(__FILE__, __LINE__)

public:
    /// @brief Constructor for the Engine class.
    /// @details Initializes window and shaders.
    Engine();

    /// @brief Destructor for the Engine class.
    ~Engine();

    /// @brief Initializes the GLFW window.
    /// @return 0 if successful, -1 otherwise.
    unsigned int initWindow(bool debug = false);

    /// @brief Loads shaders from files and stores them in the shaderManager.
    /// @details Renderers are initialized here.
    void initShaders();

    /// @brief Initializes the shapes to be rendered.
    void initShapes();

    /// @brief Pushes back a new colored rectangle to the confetti vector.
    void spawnConfetti();

    /// @brief Processes input from the user.
    /// @details (e.g. keyboard input, mouse input, etc.)
    void processInput();

    /// @brief Updates the game state.
    /// @details (e.g. collision detection, delta time, etc.)
    void update();

    /// @brief Renders the game state.
    /// @details Displays/renders objects on the screen.
    void render();

    /* deltaTime variables */
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame (used to calculate deltaTime)

    // -----------------------------------
    // Getters
    // -----------------------------------

    /// @brief Returns true if the window should close.
    /// @details (Wrapper for glfwWindowShouldClose()).
    /// @return true if the window should close
    /// @return false if the window should not close
    bool shouldClose();

    /// Projection matrix used for 2D rendering (orthographic projection).
    /// We don't have to change this matrix since the screen size never changes.
    /// OpenGL uses the projection matrix to map the 3D scene to a 2D viewport.
    /// The projection matrix transforms coordinates in the camera space into normalized device coordinates (view space to clip space).
    /// @note The projection matrix is used in the vertex shader.
    // 4th quadrant
    mat4 PROJECTION = ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
    // 1st quadrant
//        mat4 PROJECTION = ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

};

#endif //GRAPHICS_ENGINE_H