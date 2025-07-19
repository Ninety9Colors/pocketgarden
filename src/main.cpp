#include "application.hpp"
#include "game.hpp"

int main () {
    Application app {};
    Game game {};
    //game.host("darek", "TEMPORARY WORLD");
    app.run(game);
    app.exit();
    return 0;
}