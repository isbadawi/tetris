#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "config.h"

class Mode {
public:
  virtual void handleEvent(const sf::Event &Event __unused) {}
  virtual void update() {}
  virtual void display(sf::RenderWindow &Window __unused, sf::Font &Font __unused) {}
};

static void centerTextHorizontally(sf::Text &Text, sf::RenderWindow &Window) {
  sf::FloatRect Rect = Text.getLocalBounds();
  float X = (Window.getSize().x - (Rect.left + Rect.width)) / 2.0f;
  Text.setPosition(X, Text.getPosition().y);
}

const unsigned int MAX_HIGH_SCORES = 10;
class HighScores : public Mode {
private:
  std::vector<std::pair<std::string, uint64_t>> Scores;
  bool PlayerIsTyping;
  std::string *PlayerNameInput;
  std::function<void()> EndCallback;

  std::string *addScore(std::string Name, uint64_t Score);
public:
  void loadFromFile(const char *Path);
  void saveToFile(const char *Path);
  bool isHighScore(uint64_t Score);
  void recordNewHighScore(uint64_t Score);
  void handleEvent(const sf::Event &Event);
  void display(sf::RenderWindow &Window, sf::Font &Font);
  void setEndCallback(std::function<void()> Callback);
};

void HighScores::loadFromFile(const char *Path) {
  std::ifstream Stream;
  Stream.open(Path);
  assert(!Stream.fail());

  std::string Line;
  while (std::getline(Stream, Line)) {
    size_t Comma = Line.find(',');
    assert(Comma != std::string::npos);
    std::string Name = Line.substr(0, Comma);
    int Score = std::stoi(Line.substr(Comma + 1));
    assert(Score >= 0);
    addScore(Name, (uint64_t) Score);
  }
  assert(Scores.size() <= MAX_HIGH_SCORES);

  std::sort(Scores.begin(), Scores.end(), [](auto &a, auto &b) {
    return b.second < a.second;
  });
}

void HighScores::saveToFile(const char *Path) {
  std::ofstream Stream;
  Stream.open(Path);
  assert(!Stream.fail());

  for (auto &Entry : Scores) {
    Stream << Entry.first << ',' << Entry.second << '\n';
  }
}

bool HighScores::isHighScore(uint64_t Score) {
  return Scores.size() < MAX_HIGH_SCORES ||
    Score > Scores.back().second;
}

std::string *HighScores::addScore(std::string Name, uint64_t Score) {
  assert(isHighScore(Score));
  if (Scores.size() == MAX_HIGH_SCORES) {
    Scores.pop_back();
  }
  auto Pos = std::find_if(Scores.begin(), Scores.end(), [Score](auto &e) {
    return e.second < Score;
  });
  return &(*Scores.insert(Pos, std::make_pair(Name, Score))).first;
}

void HighScores::setEndCallback(std::function<void()> Callback) {
  EndCallback = Callback;
}

void HighScores::recordNewHighScore(uint64_t Score) {
  PlayerIsTyping = true;
  PlayerNameInput = addScore("", Score);
}

void HighScores::handleEvent(const sf::Event &Event) {
  if (Event.type == sf::Event::KeyPressed &&
      Event.key.code == sf::Keyboard::Return) {
    if (PlayerIsTyping) {
      assert(PlayerNameInput);
      if (!PlayerNameInput->empty()) {
        PlayerIsTyping = false;
        PlayerNameInput = nullptr;
        return;
      }
    } else {
      assert(EndCallback);
      EndCallback();
      return;
    }
  }

  if (!PlayerIsTyping) {
    return;
  }

  if (Event.type == sf::Event::TextEntered) {
    if (Event.text.unicode == '\b') {
      if (!PlayerNameInput->empty()) {
        PlayerNameInput->erase(PlayerNameInput->size() - 1, 1);
      }
    } else if (isalpha(Event.text.unicode)) {
      *PlayerNameInput += Event.text.unicode;
    }
  }
}

void HighScores::display(sf::RenderWindow &Window, sf::Font &Font) {
  sf::Text HighScoreLabel("HIGH SCORES", Font, 100);
  HighScoreLabel.setPosition(0, 0);
  centerTextHorizontally(HighScoreLabel, Window);
  Window.draw(HighScoreLabel);

  unsigned Index = 1;
  for (auto &Entry : Scores) {
    std::stringstream Line;
    Line << Index++ << ". " << Entry.first << " " << Entry.second;
    sf::Text Label(Line.str(), Font, 50);

    if (&Entry.first == PlayerNameInput) {
      Label.setFillColor(sf::Color::Yellow);
    }

    float ItemHeight = Window.getSize().y / (MAX_HIGH_SCORES + 3);
    float Y = ItemHeight * Index;
    Label.setPosition(0, Y);
    centerTextHorizontally(Label, Window);
    Window.draw(Label);
  }
}

class Menu : public Mode {
private:
  std::vector<std::pair<std::string, std::function<void()>>> MenuItems;
  unsigned Index;

public:
  Menu() {}
  void addMenuItem(std::string Label, std::function<void()> Action);
  void handleEvent(const sf::Event &Event);
  void display(sf::RenderWindow &Window, sf::Font &Font);
};

void Menu::addMenuItem(std::string Label, std::function<void()> Action) {
  MenuItems.push_back(std::make_pair(Label, Action));
}

void Menu::handleEvent(const sf::Event &Event) {
  if (Event.type == sf::Event::KeyPressed) {
    switch (Event.key.code) {
    case sf::Keyboard::Up:
      Index = std::min(Index - 1, (unsigned) MenuItems.size() - 1);
      break;
    case sf::Keyboard::Down:
      Index = (Index + 1) % MenuItems.size();
      break;
    case sf::Keyboard::Return:
      assert(0 <= Index && Index < MenuItems.size());
      MenuItems[Index].second();
      Index = 0;
      break;
    default:
      break;
    }
  }
}

void Menu::display(sf::RenderWindow &Window, sf::Font &Font) {
  sf::Text Logo("TETRIS", Font, 200);
  Logo.setPosition(0, 0);
  centerTextHorizontally(Logo, Window);
  Window.draw(Logo);

  for (unsigned I = 0, E = MenuItems.size(); I != E; ++I) {
    unsigned int FontSize = 50;
    sf::Text Label(MenuItems[I].first, Font, FontSize);
    if (I == Index) {
      Label.setFillColor(sf::Color::Yellow);
    }

    float ItemHeight = Window.getSize().y / (MenuItems.size() * 2.0f);
    float Y = ItemHeight * (I + MenuItems.size());
    Label.setPosition(0, Y);
    centerTextHorizontally(Label, Window);
    Window.draw(Label);
  }
}

class PausableClock {
  sf::Clock Clock;

  bool Paused;
  sf::Clock Pause;
  sf::Time TimeSpentPaused;

public:
  PausableClock() {}

  sf::Time getElapsedTime() {
    return Clock.getElapsedTime() - TimeSpentPaused -
      (Paused ? Pause.getElapsedTime() : sf::Time::Zero);
  }

  sf::Time restart() {
    sf::Time ElapsedTime = getElapsedTime();
    Paused = false;
    Clock.restart();
    TimeSpentPaused = sf::Time::Zero;
    return ElapsedTime;
  }

  void pause() {
    assert(!Paused);
    Paused = true;
    Pause.restart();
  }

  void unpause() {
    assert(Paused);
    TimeSpentPaused += Pause.restart();
    Paused = false;
  }

  void togglePause() {
    if (Paused) {
      unpause();
    } else {
      pause();
    }
  }

};

static int randomIntBetween(int Low, int High) {
  std::random_device Device;
  std::mt19937 Generator(Device());
  std::uniform_int_distribution<> Distribution(Low, High);
  return Distribution(Generator);
}

class Tetromino {
public:
  enum Kind {
    I, O, T, J, L, S, Z,
    NumKinds
  };
  typedef std::vector<std::vector<int>> Shape;

  Tetromino(Kind Type) : Type(Type), ShapeIndex(0) {}

  static Tetromino CreateRandom() {
    int Kind = randomIntBetween(0, Tetromino::NumKinds - 1);
    return Tetromino(static_cast<Tetromino::Kind>(Kind));
  }

  Shape &getShape();
  sf::Color getColor();
  void rotateLeft();
  void rotateRight();

private:
  Kind Type;
  int ShapeIndex;
};

static std::map<Tetromino::Kind, sf::Color> COLORS = {
  { Tetromino::I, sf::Color::White },
  { Tetromino::O, sf::Color::Red },
  { Tetromino::T, sf::Color::Yellow },
  { Tetromino::J, sf::Color::Blue },
  { Tetromino::L, sf::Color::Magenta },
  { Tetromino::S, sf::Color::Cyan },
  { Tetromino::Z, sf::Color::Green },
};

static std::map<Tetromino::Kind, std::vector<Tetromino::Shape>> SHAPES = {
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

class TetrisGame : public Mode {
private:
  uint64_t Score;
  uint64_t Level;
  uint64_t Lines;
  Tetromino Current;
  Tetromino Next;
  sf::Vector2i CurrentPos;
  std::vector<std::vector<sf::Color>> Grid;
  PausableClock Tick;
  bool Paused;
  bool GameOver;
  std::function<void(uint64_t)> EndCallback;

  bool currentPosIsValid();
  void rotateLeft();
  void rotateRight();
  void moveLeft();
  void moveRight();
  void moveDown();
  void reset();

public:
  static const uint32_t Rows = 20;
  static const uint32_t Cols = 10;
  TetrisGame() : Score(0), Level(1), Lines(0),
                 Current(Tetromino::CreateRandom()),
                 Next(Tetromino::CreateRandom()),
                 CurrentPos(3, 0),
                 Grid(Rows, std::vector<sf::Color>(Cols, sf::Color::Black)) {}
  void handleEvent(const sf::Event &Event);
  void update();
  void display(sf::RenderWindow &Window, sf::Font &Font);
  void setEndCallback(std::function<void(uint64_t)> Callback);
};

void TetrisGame::reset() {
  Score = 0;
  Level = 1;
  Lines = 0;
  Current = Tetromino::CreateRandom();
  Next = Tetromino::CreateRandom();
  CurrentPos.x = 3;
  CurrentPos.y = 0;
  Grid = std::vector<std::vector<sf::Color>>(
      Rows, std::vector<sf::Color>(Cols, sf::Color::Black));
  GameOver = false;
}

void TetrisGame::setEndCallback(std::function<void(uint64_t)> Callback) {
  EndCallback = Callback;
}

bool TetrisGame::currentPosIsValid() {
  Tetromino::Shape &Shape = Current.getShape();

  // Left wall
  if (CurrentPos.x < 0) {
    for (auto &Row : Shape) {
      if (Row[-CurrentPos.x - 1]) {
        return false;
      }
    }
  }

  // Right wall
  if (CurrentPos.x + Shape[0].size() - 1 > Cols - 1) {
    for (auto &Row : Shape) {
      if (Row[Cols - CurrentPos.x]) {
        return false;
      }
    }
  }

  // Ground
  if (CurrentPos.y + Shape.size() - 1 > Rows - 1) {
    for (unsigned i = 0; i < Shape[0].size(); ++i) {
      if (Shape[Rows - CurrentPos.y][i]) {
        return false;
      }
    }
  }

  // Other pieces
  for (unsigned i = 0; i < Shape.size(); ++i) {
    for (unsigned j = 0; j < Shape[i].size(); ++j) {
      if (Shape[i][j] &&
          Grid[CurrentPos.y + i][CurrentPos.x + j] != sf::Color::Black) {
        return false;
      }
    }
  }

  return true;
}

void TetrisGame::rotateLeft() {
  Current.rotateLeft();
  if (!currentPosIsValid()) {
    Current.rotateRight();
  }
}

void TetrisGame::rotateRight() {
  Current.rotateRight();
  if (!currentPosIsValid()) {
    Current.rotateLeft();
  }
}


void TetrisGame::moveLeft() {
  CurrentPos.x--;
  if (!currentPosIsValid()) {
    CurrentPos.x++;
  }
}

void TetrisGame::moveRight() {
  CurrentPos.x++;
  if (!currentPosIsValid()) {
    CurrentPos.x--;
  }
}

void TetrisGame::moveDown() {
  CurrentPos.y++;
  if (!currentPosIsValid()) {
    CurrentPos.y--;

    Tetromino::Shape &Shape = Current.getShape();
    for (unsigned i = 0; i < Shape.size(); ++i) {
      for (unsigned j = 0; j < Shape[i].size(); ++j) {
        if (Shape[i][j]) {
          Grid[CurrentPos.y + i][CurrentPos.x + j] = Current.getColor();
        }
      }
    }
    CurrentPos.x = 3;
    CurrentPos.y = 0;
    Current = Next;
    Next = Tetromino::CreateRandom();
    Tick.restart();

    int LinesCompleted = 0;

    for (unsigned i = 0; i < Grid.size(); ++i) {
      if (std::count(Grid[i].begin(), Grid[i].end(), sf::Color::Black) == 0) {
        ++LinesCompleted;
        for (int k = i, j = k - 1; j >= 0; --j, --k) {
          std::copy(Grid[j].begin(), Grid[j].end(), Grid[k].begin());
        }
      }
    }

    Score += randomIntBetween(14, 19);
    Lines += LinesCompleted;
    Score += LinesCompleted * 100 * (LinesCompleted == 4 ? 2 : 1);
    Level = 1 + Lines / 10;

    if (!currentPosIsValid()) {
      GameOver = true;
      return;
    }
  }
}

void TetrisGame::handleEvent(const sf::Event &Event) {
  if (GameOver) {
    return;
  }

  if (Event.type == sf::Event::KeyPressed) {
    if (Event.key.code == sf::Keyboard::P) {
      Tick.togglePause();
      Paused = !Paused;
    }

    if (Paused) {
      return;
    }

    switch (Event.key.code) {
    case sf::Keyboard::Up:
    case sf::Keyboard::X:
      rotateRight();
      break;
    case sf::Keyboard::Z:
      rotateLeft();
      break;
    case sf::Keyboard::Left:
      moveLeft();
      break;
    case sf::Keyboard::Right:
      moveRight();
      break;
    case sf::Keyboard::Down:
      moveDown();
      break;
    case sf::Keyboard::Space: {
      uint64_t CurrentScore = Score;
      do {
        moveDown();
      } while (Score == CurrentScore && !GameOver);
      break;
    }
    default:
      break;
    }
  }
}

void TetrisGame::update() {
  if (GameOver) {
    if (Tick.getElapsedTime().asSeconds() >= 2) {
      assert(EndCallback);
      EndCallback(Score);
      reset();
    }
  } else if (Tick.getElapsedTime().asSeconds() >= 1.0 / Level) {
    moveDown();
    Tick.restart();
  }
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

  for (unsigned i = 0; i < Rows; ++i) {
    for (unsigned j = 0; j < Cols; ++j) {
      sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
      Block.setOutlineColor(sf::Color::Black);
      Block.setOutlineThickness(2);
      Block.setFillColor(Grid[i][j]);
      Block.setPosition(
          Margin + j * BlockSize,
          Margin + i * BlockSize);
      Window.draw(Block);
    }
  }

  Tetromino::Shape &Shape = Current.getShape();
  for (unsigned i = 0; i < Shape.size(); ++i) {
    for (unsigned j = 0; j < Shape[i].size(); ++j) {
      if (Shape[i][j]) {
        sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
        Block.setOutlineColor(sf::Color::Black);
        Block.setOutlineThickness(2);
        Block.setFillColor(Current.getColor());
        Block.setPosition(
            Margin + (CurrentPos.x + j) * BlockSize,
            Margin + (CurrentPos.y + i) * BlockSize);
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
  for (unsigned i = 0; i < NextShape.size(); ++i) {
    for (unsigned j = 0; j < NextShape[i].size(); ++j) {
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

  if (Paused) {
    sf::Text PausedText("PAUSED", Font, 200);
    PausedText.setPosition(100, Height - 200);
    PausedText.setFillColor(sf::Color::White);
    PausedText.setOutlineColor(sf::Color::Black);
    PausedText.setOutlineThickness(5);
    PausedText.rotate(-45);
    Window.draw(PausedText);
  }
}

int main() {
  std::srand(std::time(0));
  sf::RenderWindow Window(sf::VideoMode(1024, 768), "Tetris");
  sf::Font Font;
  if (!Font.loadFromFile(ASSETS_DIR "/Arial.ttf")) {
    return 1;
  }
  sf::Music Music;
  if (!Music.openFromFile(ASSETS_DIR "/TetrisTheme.ogg")) {
    return 1;
  }
  Music.setLoop(true);
  Music.play();

  HighScores HighScores;
  // FIXME(ibadawi): Where should the file be?
  HighScores.loadFromFile("high_scores.txt");

  Menu MainMenu;
  TetrisGame Game;

  Mode *Mode = &MainMenu;

  bool Quit = false;

  MainMenu.addMenuItem("Play", [&] { Mode = &Game; });
  MainMenu.addMenuItem("High Scores", [&] { Mode = &HighScores; });
  MainMenu.addMenuItem("Quit Game", [&] { Quit = true; });

  HighScores.setEndCallback([&] { Mode = &MainMenu; });

  Game.setEndCallback([&](uint64_t Score) {
    if (HighScores.isHighScore(Score)) {
      Mode = &HighScores;
      HighScores.recordNewHighScore(Score);
    } else {
      Mode = &MainMenu;
    }
  });


  while (!Quit && Window.isOpen()) {
    sf::Event Event;
    while (Window.pollEvent(Event)) {
      if (Event.type == sf::Event::Closed) {
        Quit = true;
      }
      if (Event.type == sf::Event::KeyPressed) {
        if (Event.key.code == sf::Keyboard::M) {
          if (Music.getStatus() != sf::SoundSource::Playing) {
            Music.play();
          } else {
            Music.pause();
          }
        }
      }
      Mode->handleEvent(Event);
    }

    Mode->update();
    Window.clear();
    Mode->display(Window, Font);
    Window.display();
  }

  HighScores.saveToFile("high_scores.txt");
  Window.close();

  return 0;
}
