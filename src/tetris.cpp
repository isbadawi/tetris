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

template<typename T>
static void centerTextHorizontally(sf::Text &Text, T &HasSize) {
  sf::FloatRect Rect = Text.getLocalBounds();
  float X = (HasSize.getSize().x - (Rect.left + Rect.width)) / 2.0f;
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
  unsigned Height = Window.getSize().y;
  sf::Text HighScoreLabel("HIGH SCORES", Font, Height / 8);
  HighScoreLabel.setPosition(0, 0);
  centerTextHorizontally(HighScoreLabel, Window);
  Window.draw(HighScoreLabel);

  unsigned Index = 1;
  for (auto &Entry : Scores) {
    std::stringstream Line;
    Line << Index++ << ". " << Entry.first << " " << Entry.second;
    sf::Text Label(Line.str(), Font, Height / 15);

    if (&Entry.first == PlayerNameInput) {
      Label.setFillColor(sf::Color::Yellow);
    }

    float ItemHeight = (3 * Height / 4) / MAX_HIGH_SCORES;
    float Y = (Height / 6) + ItemHeight * (Index - 1);
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
  unsigned Height = Window.getSize().y;
  sf::Text Logo("TETRIS", Font, Height / 4);
  Logo.setPosition(0, 0);
  centerTextHorizontally(Logo, Window);
  Window.draw(Logo);

  for (unsigned I = 0, E = MenuItems.size(); I != E; ++I) {
    sf::Text Label(MenuItems[I].first, Font, Height / 15);
    if (I == Index) {
      Label.setFillColor(sf::Color::Yellow);
    }

    float ItemHeight = (Height / 2.0f) / MenuItems.size();
    float Y = (Height / 2.0f) + ItemHeight * I;
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

  bool isValid() { return Type != NumKinds; }
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
  Tetromino Saved;
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

  void jumpDown();
  sf::Vector2i downDestination();
  void onPieceDown();

public:
  static const uint32_t Rows = 20;
  static const uint32_t Cols = 10;
  TetrisGame() : Score(0), Level(1), Lines(0),
                 Current(Tetromino::CreateRandom()),
                 Next(Tetromino::CreateRandom()),
                 Saved(Tetromino::Kind::NumKinds),
                 CurrentPos(3, 0),
                 Grid(Rows, std::vector<sf::Color>(Cols, sf::Color::Black)),
                 Paused(false), GameOver(false) {}
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
  Saved = Tetromino(Tetromino::Kind::NumKinds);
  CurrentPos.x = 3;
  CurrentPos.y = 0;
  Grid = std::vector<std::vector<sf::Color>>(
      Rows, std::vector<sf::Color>(Cols, sf::Color::Black));
  Paused = false;
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

sf::Vector2i TetrisGame::downDestination() {
  auto Y = CurrentPos.y;
  while (currentPosIsValid()) {
    CurrentPos.y++;
  }
  CurrentPos.y--;
  auto Result = CurrentPos;
  CurrentPos.y = Y;
  return Result;
}

void TetrisGame::onPieceDown() {
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
      std::fill(Grid[0].begin(), Grid[0].end(), sf::Color::Black);
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

void TetrisGame::moveDown() {
  CurrentPos.y++;
  if (!currentPosIsValid()) {
    CurrentPos.y--;
    onPieceDown();
  }
}

void TetrisGame::jumpDown() {
  CurrentPos = downDestination();
  onPieceDown();
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
    case sf::Keyboard::Space:
      jumpDown();
      break;
    case sf::Keyboard::S:
      if (!Saved.isValid()) {
        Saved = Current;
        Current = Next;
        Next = Tetromino::CreateRandom();
      } else {
        std::swap(Current, Saved);
        if (!currentPosIsValid()) {
          std::swap(Current, Saved);
        }
      }
      break;
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
  unsigned int Height = Window.getSize().y;
  unsigned int Margin = 10;

  unsigned int BlockSize = (Height - 2 * Margin) / Rows;

  sf::Transform T;
  T.translate(Margin, Margin);

  sf::RectangleShape GridBox(sf::Vector2f(BlockSize * Cols, Height - 2 * Margin));
  GridBox.setOutlineColor(sf::Color::White);
  GridBox.setOutlineThickness(3);
  GridBox.setFillColor(sf::Color::Black);
  Window.draw(GridBox, T);

  auto drawBlock = [&](auto X, auto Y, auto Outline, auto Fill) {
    sf::RectangleShape Block(sf::Vector2f(BlockSize, BlockSize));
    Block.setOutlineColor(Outline);
    Block.setOutlineThickness(2);
    Block.setFillColor(Fill);
    Block.setPosition(X * BlockSize, Y * BlockSize);
    Window.draw(Block, T);
  };

  auto drawShape = [&](auto Shape, auto Pos, auto Outline, auto Fill) {
    for (unsigned i = 0; i < Shape.size(); ++i) {
      for (unsigned j = 0; j < Shape[i].size(); ++j) {
        if (Shape[i][j]) {
          drawBlock(Pos.x + j, Pos.y + i, Outline, Fill);
        }
      }
    }
  };

  for (unsigned i = 0; i < Rows; ++i) {
    for (unsigned j = 0; j < Cols; ++j) {
      drawBlock(j, i, sf::Color::Black, Grid[i][j]);
    }
  }


  Tetromino::Shape &Shape = Current.getShape();
  drawShape(Shape, downDestination(), sf::Color::White, sf::Color(0x99, 0x9d, 0xa0));
  drawShape(Shape, CurrentPos, sf::Color::Black, Current.getColor());

  sf::RectangleShape NextBox(sf::Vector2f(BlockSize * 6, BlockSize * 4));
  NextBox.setOutlineColor(sf::Color::White);
  NextBox.setOutlineThickness(3);
  NextBox.setFillColor(sf::Color::Black);

  T.translate(GridBox.getSize().x + Margin * 2, 0);

  unsigned int FontSize = Height / 15;
  sf::Text NextLabel("NEXT", Font, FontSize);
  NextLabel.setPosition(0, NextBox.getSize().y + Margin * 2);
  centerTextHorizontally(NextLabel, NextBox);

  Window.draw(NextBox, T);
  Window.draw(NextLabel, T);

  drawShape(Next.getShape(), sf::Vector2i(1, 0), sf::Color::Black, Next.getColor());

  sf::RectangleShape SaveBox(sf::Vector2f(BlockSize * 6, BlockSize * 4));
  SaveBox.setOutlineColor(sf::Color::White);
  SaveBox.setOutlineThickness(3);
  SaveBox.setFillColor(sf::Color::Black);

  T.translate(NextBox.getSize().x + Margin * 2, 0);

  sf::Text SavedLabel("SAVED", Font, FontSize);
  SavedLabel.setPosition(0, SaveBox.getSize().y + Margin * 2);
  centerTextHorizontally(SavedLabel, SaveBox);

  Window.draw(SaveBox, T);
  Window.draw(SavedLabel, T);

  if (Saved.isValid()) {
    drawShape(Saved.getShape(), sf::Vector2i(1, 0), sf::Color::Black, Saved.getColor());
  }

  sf::Text ScoreLabel("Score", Font, FontSize);
  sf::Text ScoreValue(formatInt(Score), Font, FontSize);
  sf::Text LinesLabel("Lines", Font, FontSize);
  sf::Text LinesValue(formatInt(Lines), Font, FontSize);
  sf::Text LevelLabel("Level", Font, FontSize);
  sf::Text LevelValue(formatInt(Level), Font, FontSize);

  T.translate(-(NextBox.getSize().x + Margin * 2), Height / 2);

  ScoreLabel.setPosition(0, 0);
  ScoreValue.setPosition(0, FontSize);
  LinesLabel.setPosition(0, 2 * FontSize);
  LinesValue.setPosition(0, 3 * FontSize);
  LevelLabel.setPosition(0, 4 * FontSize);
  LevelValue.setPosition(0, 5 * FontSize);

  Window.draw(ScoreLabel, T);
  Window.draw(ScoreValue, T);
  Window.draw(LinesLabel, T);
  Window.draw(LinesValue, T);
  Window.draw(LevelLabel, T);
  Window.draw(LevelValue, T);

  if (Paused) {
    sf::Text PausedText("PAUSED", Font, Height / 4);
    PausedText.setPosition(Window.getSize().x / 8, 3 * Height / 4);
    PausedText.setFillColor(sf::Color::White);
    PausedText.setOutlineColor(sf::Color::Black);
    PausedText.setOutlineThickness(5);
    PausedText.rotate(-45);
    Window.draw(PausedText);
  }
}

int main() {
  std::srand(std::time(0));
  sf::RenderWindow Window(sf::VideoMode(1920, 1440), "Tetris");
  Window.setFramerateLimit(60);
  sf::Font Font;
  if (!Font.loadFromFile(ASSETS_DIR "/joystix.ttf")) {
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
      if (Event.type == sf::Event::Resized) {
        Window.setView(sf::View(sf::FloatRect(
            0, 0, Event.size.width, Event.size.height)));
      }
      if (Event.type == sf::Event::KeyPressed) {
        if (Event.key.code == sf::Keyboard::M && Mode != &HighScores) {
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
