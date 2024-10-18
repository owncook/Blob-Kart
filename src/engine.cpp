#include "engine.h"
#include <ctime>

enum state {start, character, play, over};
state screen;

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

    // Load and play background music
    if (!backgroundMusic.openFromFile("../intro.wav")) {
      std::cerr << "Error: Could not load music file." << std::endl;
    } else {
      backgroundMusic.setLoop(false);  // Loop the music
      backgroundMusic.play();         // Start playback
    }
}

Engine::~Engine() {
  backgroundMusic.stop();
}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "Mario Kart!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    const int BORDER_GAP = 40;
    const int TRACK_WIDTH = 150;
    color track_color = color(.125,.125,.125);

    /*
     * Drawing Track
     */

    // Right side (starting side)
    track.push_back(make_unique<Rect>(shapeShader, vec2(width - BORDER_GAP - TRACK_WIDTH/2, height/2), vec2(TRACK_WIDTH,height - BORDER_GAP*2), track_color));

    // Top
    track.push_back(make_unique<Rect>(shapeShader, vec2(width/2, height - BORDER_GAP - TRACK_WIDTH/2), vec2(width - BORDER_GAP * 2 - TRACK_WIDTH, TRACK_WIDTH), track_color));

    // Left
    track.push_back(make_unique<Rect>(shapeShader, vec2(BORDER_GAP + TRACK_WIDTH/2, height/2), vec2(TRACK_WIDTH,height - BORDER_GAP*2), track_color));

    // Left vertical part of curve
    track.push_back(make_unique<Rect>(shapeShader, vec2(track[2]->getRight() + 80 + TRACK_WIDTH/2, track[1]->getBottom() - BORDER_GAP*2 - 490 / 2), vec2(TRACK_WIDTH, 490), track_color));

    // Right vertical part of curve
    track.push_back(make_unique<Rect>(shapeShader, vec2(track[3]->getRight() + TRACK_WIDTH, track[1]->getBottom() - BORDER_GAP*2 - 490 / 2), vec2(TRACK_WIDTH, 490), track_color));

    // Bottom left of curve connecting track
    track.push_back(make_unique<Rect>(shapeShader, vec2(track[2]->getRight() + 80 / 2, BORDER_GAP + TRACK_WIDTH/2),vec2(80, TRACK_WIDTH), track_color));

    // Top middle part of curve
    track.push_back(make_unique<Rect>(shapeShader, vec2((track[4]->getLeft() - track[3]->getRight())/2 + track[3]->getRight(), track[4]->getTop() - TRACK_WIDTH/2), vec2((track[4]->getLeft() - track[3]->getLeft())/2 - 38, TRACK_WIDTH), track_color));

    // Bottom right segment connecting curve to starting rectangle
    track.push_back(make_unique<Rect>(shapeShader, vec2(track[4]->getRight() + (track[0]->getLeft() - track[4]->getRight())/2, BORDER_GAP + TRACK_WIDTH/2), vec2(365, TRACK_WIDTH), track_color));

    /*
     * Drawing Starting line
     */

    int checkerX = width - BORDER_GAP - TRACK_WIDTH + 8; int checkerY = 602;
    float checkerR = 1; float checkerG = 1; float checkerB = 1;
    for (int i = 0; i < 30; i++) {
        checkers.push_back(make_unique<Rect>(shapeShader, vec2 {checkerX, checkerY}, vec2{15, 15},color {checkerR, checkerG, checkerB, 1}));
        checkerX += 15;
        checkerR = 1 - checkerR;
        checkerG = 1 - checkerG;
        checkerB = 1 - checkerB;
        if (i == 9 || i == 19) {
            checkerY -= 15;
            checkerX = width - BORDER_GAP - TRACK_WIDTH + 8;
            checkerR = 1 - checkerR;
            checkerG = 1 - checkerG;
            checkerB = 1 - checkerB;
        }
    }

    /*
     * Drawing Checkpoints
     */

    // Starting line checkpoint
    checkpoints.push_back(make_unique<Rect>(shapeShader, vec2(track[0]->getPosX(), track[1]->getBottom()), vec2(TRACK_WIDTH, 5), color(1,0,0, 0)));

    // Left most checkpoint
    checkpoints.push_back(make_unique<Rect>(shapeShader, vec2(track[2]->getPosX(), track[3]->getPosY()), vec2(TRACK_WIDTH, 5), color(1,0,0, 0)));

    // Right vertical of curve checkpoint
    checkpoints.push_back(make_unique<Rect>(shapeShader, vec2(track[4]->getPosX(), track[4]->getPosY()), vec2(TRACK_WIDTH, 5), color(1,0,0, 0)));


    /*
     * Drawing Carts
     */
    int cartOffset = 0;
    float cartR = 1; float cartG = 0; float cartB = 0;
    for (int i = 0; i < 2; i++) {
        vector<unique_ptr<Shape>> cart;
        // cart
        int cartX = (width - BORDER_GAP - (TRACK_WIDTH / 2) - 35) + cartOffset;
        int cartY = 400;
        // wheels
        int wheelBLx = cartX - 15; int wheelBLy = cartY - 20;
        int wheelFLx = cartX - 15; int wheelFLy = cartY + 20;
        int wheelBRx = cartX + 15; int wheelBRy = cartY - 20;
        int wheelFRx = cartX + 15; int wheelFRy = cartY + 20;
        // wheel BL
        cart.push_back(make_unique<Rect>(shapeShader, vec2{wheelBLx, wheelBLy}, vec2{10, 20}, color{0.3, 0.3, 0.3, 1}));
        // wheel FL
        cart.push_back(make_unique<Rect>(shapeShader, vec2{wheelFLx, wheelFLy}, vec2{10, 20}, color{0.3, 0.3, 0.3, 1}));
        // wheel BR
        cart.push_back(make_unique<Rect>(shapeShader, vec2{wheelBRx, wheelBRy}, vec2{10, 20}, color{0.3, 0.3, 0.3, 1}));
        // wheel FR
        cart.push_back(make_unique<Rect>(shapeShader, vec2{wheelFRx, wheelFRy}, vec2{10, 20}, color{0.3, 0.3, 0.3, 1}));
        // cart
        cart.push_back(make_unique<Rect>(shapeShader, vec2{cartX, cartY}, vec2{30, 65}, color {cartR, cartG, cartB, 1}));
        cart.push_back(make_unique<Rect>(shapeShader, vec2{cartX, cartY}, vec2{20, 55}, color{cartR, cartG + 0.25f, cartB + 0.25f, 1}));
        // character
        cart.push_back(make_unique<Rect>(shapeShader, vec2(cartX, cartY), vec2(10,10), charColors[i]));
        carts.push_back(std::move(cart));
        cartOffset += 75;
        cartR = 0; cartG = 0; cartB = 1;
    }

    /*
     * Drawing start screen shapes
     */

    startScreenShapes.push_back(make_unique<Rect>(shapeShader, vec2(width/2, height - (height - 267)/2), vec2(width, height - 267), color(0,1,1)));
    startScreenShapes.push_back(make_unique<Rect>(shapeShader, vec2(width / 2, height - 235), vec2(900, 200), color(0,0,0)));
    startScreenShapes.push_back(make_unique<Rect>(shapeShader, startScreenShapes[1]->getPos(), vec2(880, 180), color(1,1,1)));

    int cartX = width + 100;
    int cartY = 290;
    startScreenShapes.push_back(make_unique<Rect>(shapeShader, vec2 {cartX, cartY + 20}, vec2(20, 20), color(0.69, 0.16, 0.89)));
    startScreenShapes.push_back(make_unique<Rect>(shapeShader, vec2 {cartX, cartY}, vec2(75, 40), color(1,0,0)));
    startScreenShapes.push_back(make_unique<Rect>(shapeShader, vec2 {cartX - 25, cartY - 20}, vec2(20, 20), color(0.3,0.3,0.3)));
    startScreenShapes.push_back(make_unique<Rect>(shapeShader, vec2 {cartX + 25, cartY - 20}, vec2(20, 20), color(0.3,0.3,0.3)));

    /*
     * Pond
     */
    pond = make_unique<Rect>(shapeShader, vec2(825, 400), vec2(275, 350), color {0.2, 0.84, 0.92, 1});

    /*
     * Boost panels
     */
    boost_panels.push_back(make_unique<Rect>(shapeShader, vec2(track[1]->getPosX(),track[1]->getTop() - 40), vec2(300, 40), color(.9,.9,0)));
    boost_panels.push_back(make_unique<Rect>(shapeShader, vec2(track[7]->getPosX(),track[7]->getBottom() + 40), vec2(300, 40), color(.9,.9,0)));
    boost_panels.push_back(make_unique<Rect>(shapeShader, vec2(track[2]->getPosX(),track[2]->getTop() - 260), vec2(40, 180), color(.9,.9,0)));

    /*
     * Items sets - FOR FUTURE IMPLEMENTATION
     */

    /*
    // Set one (bottom left)
    vector<unique_ptr<Shape>> boxes;
    int box_gap = 15;
    int box_size = 20;
    int boxX = track[2]->getLeft() + box_gap + box_size/2;
    int boxY = track[5]->getTop() + box_size/2;
    for (int i = 0; i < 3; i++) {
        boxes.push_back(make_unique<Rect>(shapeShader, vec2(boxX, boxY), vec2(27,27), color(.7,0,.7,1)));
        boxX += box_size + 2*box_gap;
    }
    item_sets.push_back(std::move(boxes));

    boxes.clear();

    // Set 2 (top of track)
    box_gap = 13;
    boxX = boost_panels[0]->getPosX();
    boxY = boost_panels[0]->getBottom() - box_gap - box_size/2;
    for (int i = 0; i < 2; i++) {
        boxes.push_back(make_unique<Rect>(shapeShader, vec2(boxX, boxY), vec2(27,27), color(.7,0,.7,1)));
        boxY -= box_size + 2*box_gap;
    }
    item_sets.push_back(std::move(boxes));

    boxes.clear();

    // Set 3 (top of curve)

    box_gap = 15;
    boxX = track[6]->getPosX();
    boxY = track[6]->getTop() - box_gap - box_size/2;
    for (int i = 0; i < 3; i++) {
        boxes.push_back(make_unique<Rect>(shapeShader, vec2(boxX, boxY), vec2(27,27), color(.7,0,.7,1)));
        boxY -= box_size + 2*box_gap;
    }
    item_sets.push_back(std::move(boxes));
     */

    /*
     * Character select screen
     */
    int char_box_width = 175,
        char_box_height = 300,
        xPos, yPos = height - 40 - char_box_height/2,
        color_index = 0;
    for (int i = 0; i < 2; i++) {
        xPos = 100 + char_box_width/2;
        for (int j = 0; j < 4; j++) {
            characters.push_back(make_unique<Rect>(shapeShader, vec2(xPos, yPos), vec2(char_box_width + 15, char_box_height + 15), color(0,0,0)));
            characters.push_back(make_unique<Rect>(shapeShader, vec2(xPos, yPos), vec2(char_box_width, char_box_height), color(1,1,1)));
            characters.push_back(make_unique<Rect>(shapeShader, vec2(xPos, yPos - 10), vec2(char_box_width - 100, char_box_height - 175), blob_colors[color_index]));
            xPos += 100 + char_box_width;
            color_index++;
        }
        yPos -= 40 + char_box_height;
    }

    button.push_back(make_unique<Rect>(shapeShader, vec2(width - 110, 45), vec2(210, 85), color(.7,0,0)));
    button.push_back(make_unique<Rect>(shapeShader, button[0]->getPos(), vec2(200, 75), color(1,0,0)));

    for (int i = 0; i < 300; i++) {
        color randColor = {float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), 1.0f};
        confetti.push_back(make_unique<Rect>(shapeShader, vec2 (rand() % width, height + 2 + (rand() % height)), vec2(rand() % 14, rand() % 15), randColor));
    }

    int podiumWidth = 300,
        podiumHeight = 100,
        borderGap = 150,
        podiumX = podiumWidth/2 + borderGap;

    podium.push_back(make_unique<Rect>(shapeShader, vec2(podiumX, 200), vec2(podiumWidth, podiumHeight), color(.4,0,0)));
    podiumHeight += 200;
    podiumX += podiumWidth;
    podium.push_back(make_unique<Rect>(shapeShader, vec2(podiumX, podium[0]->getBottom() + podiumHeight / 2), vec2(podiumWidth, podiumHeight), color(.4,0,0)));
    podiumHeight -= 100;
    podiumX += podiumWidth;
    podium.push_back(make_unique<Rect>(shapeShader, vec2(podiumX, podium[0]->getBottom() + podiumHeight / 2), vec2(podiumWidth, podiumHeight), color(.4,0,0)));

    int blobHeight = 125,
        blobWidth = 75;
    // First place
    winningBlobs.push_back(make_unique<Rect>(shapeShader, vec2(podium[1]->getPosX(), podium[1]->getTop() + blobHeight / 2), vec2(blobWidth, blobHeight), color(1,1,1)));
    // Second place
    winningBlobs.push_back(make_unique<Rect>(shapeShader, vec2(podium[2]->getPosX(), podium[2]->getTop() + blobHeight / 2), vec2(blobWidth, blobHeight), color(1,1,1)));



}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);
    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool buttonOverlapsMouse; bool characterOverlapsMouse;

    if (keys[GLFW_KEY_SPACE] && screen == start) {
        screen = character;
    }

    if (screen == character) {
        buttonOverlapsMouse = button[0]->isOverlapping(vec2(MouseX, MouseY));

        static int redSelectIndex = 0;
        if (!redSelect) { // Red character select
            int characterHover = 0;
            for (int i = 0; i < blob_colors.size(); i++){
                bool characterBoxHover = characters[characterHover]->isOverlapping(vec2(MouseX,MouseY));
                if (characterBoxHover) {
                    // Sets the box to red
                    characters[characterHover]->setColor(color(1,0,0));
                    if (mousePressedLastFrame && !mousePressed) {
                        charColors[0] = blob_colors[i];
                        redSelect = true;
                        carts[0][6]->setColor(charColors[0]);
                        redSelectIndex = characterHover;
                        break;
                    }
                } else {
                    characters[characterHover]->setColor(color(0,0,0));
                }
                characterHover += 3;
            }
        } else if (!blueSelect) { // Blue character select
            int characterHover = 0;
            for (int i = 0; i < blob_colors.size(); i++){
                bool characterBoxHover = characters[characterHover]->isOverlapping(vec2(MouseX,MouseY));
                if (characterBoxHover) {
                    // Sets the box to red
                    characters[characterHover]->setColor(color(0,0,1));
                    if (mousePressedLastFrame && !mousePressed) {
                        charColors[1] = blob_colors[i];
                        blueSelect = true;
                        carts[1][6]->setColor(charColors[1]);
                    }
                } else {
                    if (characterHover == redSelectIndex) {
                        characters[redSelectIndex]->setColor(color(1,0,0));
                    } else {
                        characters[characterHover]->setColor(color(0,0,0));
                    }
                }

                characterHover += 3;
            }
        }


        if (buttonOverlapsMouse && redSelect && blueSelect) {
            if (mousePressed) {
                // click
                button[0]->setColor(color{0.2, 0, 0});
                button[1]->setColor(color{0.5, 0, 0});
            } else {
                // hover
                button[0]->setColor(color{1, 0.5, 0.5});
                button[1]->setColor(color{1, 0.5, 0.5});
            }
            // release click
            if (mousePressedLastFrame && !mousePressed && blueSelect && redSelect) {
                time_t curr_time;
                time(&curr_time);
                countdown = (unsigned long) curr_time;
                screen = play;
            }
        // original fill
        } else if (!mousePressed) {
            button[0]->setColor(color {0.7, 0, 0});
            button[1]->setColor(color {1, 0, 0});
            if (!redSelect || !blueSelect) {
                button[0]->setColor(color{0.2, 0, 0});
                button[1]->setColor(color{0.5, 0, 0});
            }
        }
    }

    if (screen == play) {

        bool red_on_track;
        for (auto &t : track) {
            red_on_track = false;
            if (t->isOverlapping(carts[0][4]->getPos())) {
                red_on_track = true;
                break;
            }
        }

        bool blue_on_track;
        for (auto &t : track) {
            blue_on_track = false;
            if (t->isOverlapping(carts[1][4]->getPos())) {
                blue_on_track = true;
                break;
            }
        }

        bool red_on_boost;
        for (auto &t : boost_panels) {
            red_on_boost = false;
            if (t->isOverlapping(carts[0][4]->getPos())) {
                red_on_boost = true;
                break;
            }
        }

        bool blue_on_boost;
        for (auto &t : boost_panels) {
            blue_on_boost = false;
            if (t->isOverlapping(carts[1][4]->getPos())) {
                blue_on_boost = true;
                break;
            }
        }

        if (red_on_boost)       {cart_speed_red  = 6.00;}
        else if (red_on_track)  {cart_speed_red  = 3.15;}
        else                    {cart_speed_red  = 0.35;}
        if (blue_on_boost)      {cart_speed_blue = 6.00;}
        else if (blue_on_track) {cart_speed_blue = 3.15;}
        else                    {cart_speed_blue = 0.35;}

        if (!go) {
            time_t curr_time;
            time(&curr_time);
            if (curr_time - countdown >= 4) {
                time(&curr_time);
                timer = (unsigned long) curr_time;
                go = true;
            }
        }

        if (go) {
            // RED CART
            if (keys[GLFW_KEY_W]) {
                for (auto &item: carts[0]) {
                    if (carts[0][4]->getTop() < height) {
                        item->move(vec2{0, cart_speed_red});
                    }
                    item->rotateUp();
                }
            }
            if (keys[GLFW_KEY_S]) {
                for (auto &item: carts[0]) {
                    if (carts[0][4]->getBottom() > 0) {
                        item->move(vec2{0, -cart_speed_red});
                    }
                    item->rotateDown();
                }
            }
            if (keys[GLFW_KEY_A]) {
                for (auto &item: carts[0]) {
                    if (carts[0][4]->getLeft() > 17) {
                        item->move(vec2{-cart_speed_red, 0});
                    }
                    item->rotateLeft();
                }
            }
            if (keys[GLFW_KEY_D]) {
                for (auto &item: carts[0]) {
                    if (carts[0][4]->getRight() < width - 17) {
                        item->move(vec2{cart_speed_red, 0});
                    }
                    item->rotateRight();
                }
            }

            // BLUE CART
            if (keys[GLFW_KEY_UP]) {
                for (auto &item: carts[1]) {
                    if (carts[1][4]->getTop() < height) {
                        item->move(vec2{0, cart_speed_blue});
                    }
                    item->rotateUp();
                }
            }
            if (keys[GLFW_KEY_DOWN]) {
                for (auto &item: carts[1]) {
                    if (carts[1][4]->getBottom() > 0) {
                        item->move(vec2{0, -cart_speed_blue});
                    }
                    item->rotateDown();
                }
            }
            if (keys[GLFW_KEY_LEFT]) {
                for (auto &item: carts[1]) {
                    if (carts[1][4]->getLeft() > 17) {
                        item->move(vec2{-cart_speed_blue, 0});
                    }
                    item->rotateLeft();
                }
            }
            if (keys[GLFW_KEY_RIGHT]) {
                for (auto &item: carts[1]) {
                    if (carts[1][4]->getRight() < width - 17) {
                        item->move(vec2{cart_speed_blue, 0});
                    }
                    item->rotateRight();
                }
            }
        }
    }

    if (screen == over) {
        buttonOverlapsMouse = button[0]->isOverlapping(vec2(MouseX, MouseY));

        if (buttonOverlapsMouse) {
            if (mousePressed) {
                // click
                button[0]->setColor(color{0.2, 0, 0});
                button[1]->setColor(color{0.5, 0, 0});
            } else {
                // hover
                button[0]->setColor(color{1, 0.5, 0.5});
                button[1]->setColor(color{1, 0.5, 0.5});
            }
            // release click
            if (mousePressedLastFrame && !mousePressed) {
                carLap = {0, 0};
                carCheckpoints = {2,2};
                carts.clear();
                checkers.clear();
                track.clear();
                checkpoints.clear();
                boost_panels.clear();
                // item_sets.clear();
                // item_shower.clear();
                startScreenShapes.clear();
                characters.clear();
                button.clear();
                confetti.clear();
                podium.clear();
                winningBlobs.clear();
                redSelect = false;
                blueSelect = false;
                go = false;
                charColors[0] = color(.7,0,0);
                charColors[1] = color(0.12, 0.25, 0.96);
                initShapes();
                screen = start;
            }
            // original fill
        } else if (!mousePressed) {
            button[0]->setColor(color{0.7, 0, 0});
            button[1]->setColor(color{1, 0, 0});
        }
    }
    mousePressedLastFrame = mousePressed;
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (!introFinished && backgroundMusic.getStatus() == sf::SoundSource::Stopped) {
      // Intro music has finished
      introFinished = true;

      // Load and play loop music
      if (!backgroundMusic.openFromFile("../loop.wav")) {
        std::cerr << "Error: Could not load loop music file." << std::endl;
      } else {
        backgroundMusic.setLoop(true);  // Loop continuously
        backgroundMusic.play();         // Start playback
      }
    }

    if (screen == play) {
        // Red car checkpoint system
        for (int cp_num = 0; cp_num < checkpoints.size(); cp_num++) {
            if (checkpoints[cp_num]->isOverlapping(carts[0][4]->getPos())) {
                if (carCheckpoints[0] == cp_num - 1) {
                    carCheckpoints[0]++;
                } else if (carCheckpoints[0] == 2 && cp_num == 0) {
                    carCheckpoints[0] = 0;
                    carLap[0]++;
                }
            }
        }

        // Blue car checkpoint system
        for (int cp_num = 0; cp_num < checkpoints.size(); cp_num++) {
            if (checkpoints[cp_num]->isOverlapping(carts[1][4]->getPos())) {
                if (carCheckpoints[1] == cp_num - 1) {
                    carCheckpoints[1]++;
                } else if (carCheckpoints[1] == 2 && cp_num == 0) {
                    carCheckpoints[1] = 0;
                    carLap[1]++;
                }
            }
        }

        /*
        // red car item box collection
        for (int i = 0; i < item_sets.size(); i++) {
            for (int j = 0; j < item_sets[i].size(); j++) {
                if (item_sets[i][j]->isOverlapping(carts[0][4]->getPos())) {
                    item_sets[i].erase(item_sets[i].begin() + j);
                }
            }
        }

        // blue car item box collection
        for (int i = 0; i < item_sets.size(); i++) {
            for (int j = 0; j < item_sets[i].size(); j++) {
                if (item_sets[i][j]->isOverlapping(carts[1][4]->getPos())) {
                    item_sets[i].erase(item_sets[i].begin() + j);
                }
            }
        }
         */

        if (carLap[0] > 3 || carLap[1] > 3) {
            screen = over;
        }
    }
    if (screen == over) {
        for (auto &piece : confetti) {
            piece->moveY(-piece->getSize().y / 10.0);
            if (piece->getPosY() < 0) {
                piece->setPos(vec2 {rand() % width, height + piece->getSize().y});
            }
        }
    }

}

void Engine::render() {
    glClearColor(0.0f, 0.7f, 0.1f, 1.0f); // Background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    switch (screen) {
        case (start): {
            // TODO more ponds

            // TODO add a car and clouds to the title screen
            string startMessage = "Press 'space' to start!";
            string marioKart = "Blob Kart";
            string author = "By Aaron Perkel and Owen Cook";
            string controls1 = "Player 1: WASD";
            string controls2 = "Player 2: Arrow Keys";
            for (auto& shape : startScreenShapes) {
                shape->setUniforms();
                shape->draw();
            }

            for (int i = 3; i < startScreenShapes.size(); i++) {
                startScreenShapes[i]->moveX(-2);
            }
            for (int i = 5; i < startScreenShapes.size(); i++) {
                startScreenShapes[i]->rotate(0.08);
            }
            if (startScreenShapes[3]->getPosX() < -100) {
                startScreenShapes[3]->moveX(width + 200);
                startScreenShapes[4]->moveX(width + 200);
                startScreenShapes[5]->moveX(width + 200);
                startScreenShapes[6]->moveX(width + 200);
            }

            this->fontRenderer->renderText(startMessage, width / 2 - (12 * startMessage.length()), height / 2 - 200, 1, vec3{.825, .825, 0});
            this->fontRenderer->renderText(marioKart, width / 2 - (24 * marioKart.length()), startScreenShapes[1]->getPosY() - 20, 2, vec3{0,0,0});
            this->fontRenderer->renderText(author, width / 2 - (12 * author.length()), startScreenShapes[1]->getPosY() - 65, 1, vec3{0,0,0});
            this->fontRenderer->renderText(controls1, 20, 25, 1, vec3 {1, 0, 0});
            this->fontRenderer->renderText(controls2, width - (25 * controls2.length()), 25, 1, vec3 {0, 0, 1});
            break;
        }
        case (character): {
            for (unique_ptr<Shape> &character: characters) {
                character->setUniforms();
                character->draw();
            }

            for (unique_ptr<Shape> &shape: button) {
                shape->setUniforms();
                shape->draw();
            }

            string button_text = "Play";
            this->fontRenderer->renderText(button_text, button[0]->getPosX() - 50, button[0]->getPosY() - 10, 1, vec3(1,1,1));

            int character = 0;
            vector<string> blobColors = {"Red", "Orange", "Yellow", "Green", "Blue", "Purple", "Black", "Brown"};
            for (auto &bc : blobColors) {
                this->fontRenderer->renderText(bc + " Blob", characters[character]->getPosX() - (12 * (bc.length() + 5)), characters[character]->getBottom() - 22, 1, vec3(0,.1,0));
                character += 3;
            }

            string pickMessage;
            vec3 pickMessageColor;
            if (!redSelect) {
                pickMessage = "Player 1 pick";
                pickMessageColor = vec3(.9,0,0);
            } else if (!blueSelect) {
                pickMessage = "Player 2 pick";
                pickMessageColor = vec3(0,0,1);
            }
            this->fontRenderer->renderText(pickMessage, width/2 - (12 * pickMessage.length()), 50, 1, pickMessageColor);


            break;
        }
        case (play): {
            glClearColor(0.0f, 0.7f, 0.1f, 1.0f); // Background green
            glClear(GL_COLOR_BUFFER_BIT);

            // TODO add countdown to start race

            pond->setUniforms();
            pond->draw();

            for (auto& track_piece : track) {
                track_piece->setUniforms();
                track_piece->draw();
            }

            for (auto& panel : boost_panels) {
                panel->setUniforms();
                panel->draw();
            }

            /*
            for (vector<unique_ptr<Shape>> &set : item_sets) {
                for (unique_ptr<Shape> &box: set) {
                    box->setUniforms();
                    box->draw();
                }
            }
             */

            for (auto& checker : checkers) {
                checker->setUniforms();
                checker->draw();
            }

            for (auto& c : checkpoints) {
                c->setUniforms();
                c->draw();
            }

            for (vector<unique_ptr<Shape>> &cart : carts) {
                for (unique_ptr<Shape> &item: cart) {
                    item->setUniforms();
                    item->draw();
                }
            }

            string redCarLaps = "Red: Lap ";
            redCarLaps += std::to_string(carLap[0]);
            string blueCarLaps = "Blue: Lap ";
            blueCarLaps += std::to_string(carLap[1]);

            if (carLap[0] > 0) {
                this->fontRenderer->renderText(redCarLaps + "/3", 40, 10, .75, vec3{1, 1, 1});
            } else {
                this->fontRenderer->renderText("Red: Lap 1/3", 40, 10, .75, vec3{1, 1, 1});
            }

            if (carLap[1] > 0) {
                this->fontRenderer->renderText(blueCarLaps + "/3", width - 12 * (blueCarLaps.length() + 2) - 110, 10, .75, vec3{1, 1, 1});
            } else {
                this->fontRenderer->renderText("Blue: Lap 1/3", width - 12 * (blueCarLaps.length() + 2) - 110, 10, .75, vec3{1, 1, 1});
            }

            time_t curr_time;
            time(&curr_time);
            string time = "Time: " + std::to_string(curr_time - timer);
            string cd; string cd_text;
            if (go) {
                this->fontRenderer->renderText(time, 40, height - 36, 0.6, vec3{1, 1, 1});
                if (curr_time - timer < 2) {
                    this->fontRenderer->renderText("GO!", width / 2 - 45, height / 2, 3, {1, 1, 1});
                }
            } else {
                // countdown
                this->fontRenderer->renderText("Time: 0", 40, height - 36, 0.6, vec3{1, 1, 1});
                cd = std::to_string(curr_time - countdown);
                if (cd == "1") {cd_text = "3";}
                if (cd == "2") {cd_text = "2";}
                if (cd == "3") {cd_text = "1";}
                this->fontRenderer->renderText(cd_text, width / 2, height / 2, 3, {1, 1, 1});
            }
            break;

        }
        case (over): {
            // TODO add podium with characters
            // TODO add leaderboard
            if (carLap[0] > carLap[1]) {
                winningBlobs[0]->setColor(charColors[0]);
                winningBlobs[1]->setColor(charColors[1]);
            } else {
                winningBlobs[0]->setColor(charColors[1]);
                winningBlobs[1]->setColor(charColors[0]);
            }

            startScreenShapes[0]->setUniforms();
            startScreenShapes[0]->draw();

            for (auto& p : podium) {
                p->setUniforms();
                p->draw();
            }
            for (auto& blob : winningBlobs) {
                blob->setUniforms();
                blob->draw();
            }
            for (auto& piece : confetti) {
                piece->setUniforms();
                piece->draw();
            }
            for (unique_ptr<Shape> &shape : button) {
                shape->setUniforms();
                shape->draw();
            }

            string button_text = "Main Menu";
            this->fontRenderer->renderText(button_text, button[0]->getPosX() - 80, button[0]->getPosY() - 10, .75, vec3(1,1,1));

            this->fontRenderer->renderText("1", podium[1]->getPosX() - 24, podium[1]->getTop() - 70 , 2, vec3(.8,.8,0));
            this->fontRenderer->renderText("2", podium[2]->getPosX() - 24, podium[2]->getTop() - 70 , 2, vec3(.8,.8,0));
            this->fontRenderer->renderText("3", podium[0]->getPosX() - 24, podium[0]->getTop() - 70 , 2, vec3(.8,.8,0));


            for (auto& piece : confetti) {
                piece->setUniforms();
                piece->draw();
            }
            break;
        }
    }

    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            // case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            // case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}