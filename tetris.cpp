#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

typedef std::vector<std::vector<int> > Tetromino;
std::vector<Tetromino> TETROMINOES = {
{
  // L
  {0, 0, 0, 0, 0},
  {0, 0, 0, 1, 0},
  {0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
},

{
  // T
  {0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0},
  {0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
},

{
  // Reverse L
  {0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0},
  {0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
},

{
  // Square
  {0, 0, 0, 0, 0},
  {0, 0, 1, 1, 0},
  {0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
},

{
  // Line
  {0, 0, 0, 0, 0},
  {0, 0, 0, 1, 0},
  {0, 0, 0, 1, 0},
  {0, 0, 0, 1, 0},
  {0, 0, 0, 1, 0},
},

{
  // S
  {0, 0, 0, 0, 0},
  {0, 0, 1, 1, 0},
  {0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
},

{
  // Reverse S
  {0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0},
  {0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
},

};

static Tetromino& randomTetromino() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, TETROMINOES.size() - 1);
  return TETROMINOES[dis(gen)];
}

class TetrisGame {
private:
  std::vector<std::vector<sf::Color> > Grid;
  Tetromino *Current;
  Tetromino *Next;
  uint64_t Score;
  uint64_t Level;
  uint64_t Lines;

public:
  static const int Rows = 20;
  static const int Cols = 10;
  TetrisGame() : Score(0), Level(1), Lines(0),
                 Current(nullptr), Next(&randomTetromino()),
                 Grid(Rows, std::vector<sf::Color>(Cols, sf::Color::Black)) {}
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

  unsigned int BlockSize = (Height - 2 * Margin) / Rows;

  sf::RectangleShape GridBox(sf::Vector2f(BlockSize * Cols, Height - 2 * Margin));
  GridBox.setOutlineColor(sf::Color::White);
  GridBox.setOutlineThickness(3);
  GridBox.setFillColor(sf::Color::Black);
  GridBox.setPosition(Margin, Margin);
  Window.draw(GridBox);

  for (int i = 0; i < Rows; ++i) {
    for (int j = 0; j < Cols; ++j) {
      sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
      Block.setOutlineColor(sf::Color::Black);
      Block.setOutlineThickness(2);
      Block.setFillColor(Grid[i][j]);
      Block.setPosition(
          Margin + j * BlockSize,
          Margin + (20 - i - 1) * BlockSize);
      Window.draw(Block);
    }
  }

  sf::RectangleShape NextBox(sf::Vector2f(BlockSize * 5, BlockSize * 5));
  NextBox.setOutlineColor(sf::Color::White);
  NextBox.setOutlineThickness(3);
  NextBox.setFillColor(sf::Color::Black);
  unsigned int NextBoxX = Width / 2 + (Width / 2 - BlockSize * 5) / 4;
  unsigned int NextBoxY = Height / 8;
  NextBox.setPosition(NextBoxX, NextBoxY);
  Window.draw(NextBox);

  for (int i = 0; i < Next->size(); ++i) {
    for (int j = 0; j < (*Next)[0].size(); ++j) {
      if ((*Next)[i][j]) {
        sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
        Block.setOutlineColor(sf::Color::Black);
        Block.setOutlineThickness(2);
        Block.setFillColor(sf::Color::White);
        Block.setPosition(
            NextBoxX + j * BlockSize,
            NextBoxY + i * BlockSize);
        Window.draw(Block);
      }
    }
  }

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
  std::srand(std::time(0));
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
