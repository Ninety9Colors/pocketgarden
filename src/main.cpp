#include "application.hpp"
#include "game.hpp"

#include "logging.hpp"
#include "settings.hpp"

int main () {
    Application app {};
    INFO("Finished app initialization");
    Game game {};
    INFO("Finished game initialization, running game...");
    app.run(game);
    app.exit();
    return 0;
}