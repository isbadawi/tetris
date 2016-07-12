#include <cstdint>

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

class TetrisGame {
private:
  uint64_t Score;
  uint64_t Level;
  uint64_t Lines;

public:
  bool handleEvent(const sf::Event &Event);
  void update(sf::Time Delta);
  void display(sf::RenderWindow &Window);
};

bool TetrisGame::handleEvent(const sf::Event &Event) {
  if (Event.type == sf::Event::Closed) {
    return false;
  }

  return true;
}

void TetrisGame::update(sf::Time Delta) {
}

void TetrisGame::display(sf::RenderWindow &Window) {
}

int main() {
  sf::RenderWindow Window(sf::VideoMode(200, 200), "SFML works!");
  TetrisGame Game;

  bool Quit = false;
  sf::Clock Clock;
  while (!Quit && Window.isOpen()) {
    sf::Event Event;
    while (Window.pollEvent(Event)) {
      Quit = !Game.handleEvent(Event);
    }

    Game.update(Clock.restart());
    Window.clear();
    Game.display(Window);
    Window.display();
  }

  Window.close();

  return 0;
}
