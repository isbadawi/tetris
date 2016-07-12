#include <cstdint>
#include <iostream>
#include <sstream>

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

class TetrisGame {
private:
  uint64_t Score;
  uint64_t Level;
  uint64_t Lines;

public:
  TetrisGame() : Score(0), Level(1), Lines(0) {}
  bool handleEvent(const sf::Event &Event);
  void update(sf::Time Delta);
  void display(sf::RenderWindow &Window, sf::Font &Font);
};

bool TetrisGame::handleEvent(const sf::Event &Event) {
  if (Event.type == sf::Event::Closed) {
    return false;
  }

  return true;
}

void TetrisGame::update(sf::Time Delta) {
}

static std::string formatInt(uint64_t Val) {
  std::ostringstream Stream;
  Stream << Val;
  return Stream.str();
}

void TetrisGame::display(sf::RenderWindow &Window, sf::Font &Font) {
  unsigned int Width = Window.getSize().x;
  unsigned int Height = Window.getSize().y;
  unsigned int Margin = 10;

  sf::RectangleShape Grid(sf::Vector2f(Width / 2, Height - 2 * Margin));
  Grid.setOutlineColor(sf::Color::White);
  Grid.setOutlineThickness(3);
  Grid.setFillColor(sf::Color::Black);
  Grid.setPosition(Margin, Margin);
  Window.draw(Grid);

  unsigned int FontSize = 50;
  sf::Text ScoreLabel("Score", Font, FontSize);
  sf::Text ScoreValue(formatInt(Score), Font, FontSize);
  sf::Text LinesLabel("Lines", Font, FontSize);
  sf::Text LinesValue(formatInt(Lines), Font, FontSize);
  sf::Text LevelLabel("Level", Font, FontSize);
  sf::Text LevelValue(formatInt(Level), Font, FontSize);

  ScoreLabel.setPosition(2 * Width / 3, 7 * Height / 12);
  ScoreValue.setPosition(2 * Width / 3, 7 * Height / 12 + 1 * FontSize);
  LinesLabel.setPosition(2 * Width / 3, 7 * Height / 12 + 2 * FontSize);
  LinesValue.setPosition(2 * Width / 3, 7 * Height / 12 + 3 * FontSize);
  LevelLabel.setPosition(2 * Width / 3, 7 * Height / 12 + 4 * FontSize);
  LevelValue.setPosition(2 * Width / 3, 7 * Height / 12 + 5 * FontSize);

  Window.draw(ScoreLabel);
  Window.draw(ScoreValue);
  Window.draw(LinesLabel);
  Window.draw(LinesValue);
  Window.draw(LevelLabel);
  Window.draw(LevelValue);

}

int main() {
  sf::RenderWindow Window(sf::VideoMode(1024, 768), "Tetris");
  sf::Font Font;
  if (!Font.loadFromFile("Arial.ttf")) {
    std::cerr << "error loading font Arial.ttf\n";
    return 1;
  }
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
    Game.display(Window, Font);
    Window.display();
  }

  Window.close();

  return 0;
}
