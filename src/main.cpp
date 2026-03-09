#include "application.hpp"
#include "game.hpp"

#include "logging.hpp"
#include "settings.hpp"

#include "util/parse.hpp"

int main () {
    Application app {};
    INFO("Finished app initialization");
    Game game {};
    INFO("Finished game initialization, running game...");
    parse_xyz("dryopteris-erythrosora-01 (1).xyz");
    app.run(game);
    app.exit();
    return 0;
}