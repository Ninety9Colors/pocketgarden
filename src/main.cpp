#include "application.hpp"
#include "game.hpp"

int main () {
    Application app {};
    Game game {"darek"};
    app.run(game);
    app.exit();
    return 0;
}