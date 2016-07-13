#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <vector>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

class Tetromino {
public:
  enum Kind {
    I, O, T, J, L, S, Z,
    NumKinds
  };
  typedef std::vector<std::vector<int> > Shape;

  Tetromino(Kind Type) : Type(Type), ShapeIndex(0) {}

  static Tetromino CreateRandom() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, Tetromino::NumKinds - 1);
    return Tetromino(static_cast<Tetromino::Kind>(dis(gen)));
  }

  Shape& getShape();
  sf::Color getColor();
  void rotateLeft();
  void rotateRight();

private:
  Kind Type;
  int ShapeIndex;
};

std::map<Tetromino::Kind, sf::Color> COLORS = {
  { Tetromino::I, sf::Color::White },
  { Tetromino::O, sf::Color::Red },
  { Tetromino::T, sf::Color::Yellow },
  { Tetromino::J, sf::Color::Blue },
  { Tetromino::L, sf::Color::Magenta },
  { Tetromino::S, sf::Color::Cyan },
  { Tetromino::Z, sf::Color::Green },
};

std::map<Tetromino::Kind, std::vector<Tetromino::Shape>> SHAPES = {
  {
    Tetromino::I,
    {
      {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
      }, {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
      }
    }
  },
  {
    Tetromino::O,
    {
      {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
      }
    }
  },
  {
    Tetromino::T,
    {
      {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
      }, {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0}
      }, {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 1, 0, 0}
      }, {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0}
      }
    }
  },
  {
    Tetromino::J,
    {
      {
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0}
      }, {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
      }, {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
      }, {
        {1, 1, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0}
      }
    }
  },
  {
    Tetromino::L,
    {
      {
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0}
      }, {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}
      }, {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
      }, {
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
      }
    }
  },
  {
    Tetromino::S,
    {
      {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
      }, {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
      }
    }
  },
  {
    Tetromino::Z,
    {
      {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
      }, {
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
      }
    }
  }
};

Tetromino::Shape& Tetromino::getShape() {
  return SHAPES[Type][ShapeIndex];
}

sf::Color Tetromino::getColor() {
  return COLORS[Type];
}

void Tetromino::rotateLeft() {
  ShapeIndex = (ShapeIndex - 1) % SHAPES[Type].size();
}

void Tetromino::rotateRight() {
  ShapeIndex = (ShapeIndex + 1) % SHAPES[Type].size();
}

class TetrisGame {
private:
  std::vector<std::vector<sf::Color> > Grid;
  Tetromino Current;
  Tetromino Next;
  int32_t CurrentX;
  int32_t CurrentY;
  uint64_t Score;
  uint64_t Level;
  uint64_t Lines;

public:
  static const uint32_t Rows = 20;
  static const uint32_t Cols = 10;
  TetrisGame() : Score(0), Level(1), Lines(0),
                 Current(Tetromino::CreateRandom()),
                 Next(Tetromino::CreateRandom()),
                 CurrentX(3), CurrentY(0),
                 Grid(Rows, std::vector<sf::Color>(Cols, sf::Color::Black)) {}
  bool handleEvent(const sf::Event &Event);
  void update(sf::Time Delta);
  void display(sf::RenderWindow &Window, sf::Font &Font);
};

bool TetrisGame::handleEvent(const sf::Event &Event) {
  if (Event.type == sf::Event::Closed) {
    return false;
  }

  if (Event.type == sf::Event::KeyPressed) {
    switch (Event.key.code) {
    case sf::Keyboard::Up:
    case sf::Keyboard::X:
      Current.rotateRight();
      break;
    case sf::Keyboard::Z:
      Current.rotateLeft();
      break;
    default:
      break;
    }
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

  Tetromino::Shape &Shape = Current.getShape();
  for (int i = 0; i < Shape.size(); ++i) {
    for (int j = 0; j < Shape[i].size(); ++j) {
      if (Shape[i][j]) {
        sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
        Block.setOutlineColor(sf::Color::Black);
        Block.setOutlineThickness(2);
        Block.setFillColor(Current.getColor());
        Block.setPosition(
            Margin + (CurrentX + j) * BlockSize,
            Margin + (CurrentY + i) * BlockSize);
        Window.draw(Block);
      }
    }
  }

  sf::RectangleShape NextBox(sf::Vector2f(BlockSize * 6, BlockSize * 4));
  NextBox.setOutlineColor(sf::Color::White);
  NextBox.setOutlineThickness(3);
  NextBox.setFillColor(sf::Color::Black);
  unsigned int NextBoxX = Width / 2 + (Width / 2 - BlockSize * 5) / 4;
  unsigned int NextBoxY = Height / 8;
  NextBox.setPosition(NextBoxX, NextBoxY);
  Window.draw(NextBox);

  Tetromino::Shape &NextShape = Next.getShape();
  for (int i = 0; i < NextShape.size(); ++i) {
    for (int j = 0; j < NextShape[i].size(); ++j) {
      if (NextShape[i][j]) {
        sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
        Block.setOutlineColor(sf::Color::Black);
        Block.setOutlineThickness(2);
        Block.setFillColor(Next.getColor());
        Block.setPosition(
            NextBoxX + (j + 1) * BlockSize,
            NextBoxY + (i * 1) * BlockSize);
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
    return 1;
  }
  sf::Music Music;
  if (!Music.openFromFile("TetrisTheme.ogg")) {
    return 1;
  }
  Music.setLoop(true);
  Music.play();

  TetrisGame Game;

  bool Quit = false;
  sf::Clock Clock;
  while (!Quit && Window.isOpen()) {
    sf::Event Event;
    while (Window.pollEvent(Event)) {
      if (Event.type == sf::Event::KeyPressed) {
        if (Event.key.code == sf::Keyboard::M) {
          if (Music.getStatus() != sf::SoundSource::Playing) {
            Music.play();
          } else {
            Music.pause();
          }
        }
      }
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
