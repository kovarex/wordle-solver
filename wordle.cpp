#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <cassert>
#include <map>
#include <conio.h>

constexpr int WORD_LENGTH = 5;
class LetterCounts;

struct Word
{
  Word() = default;
  Word(const std::string& input) { assert(input.size() == WORD_LENGTH); for (int i = 0; i < WORD_LENGTH; ++i) data[i] = input[i]; }
  Word(const char* input) { assert(strlen(input) == WORD_LENGTH); for (int i = 0; i < WORD_LENGTH; ++i) data[i] = input[i]; }
  constexpr char operator[](int i) const { return data[i]; }
  constexpr char& operator[](int i) { return data[i]; }
  constexpr bool operator==(const Word& other) const { for (int i = 0; i < WORD_LENGTH; ++i) if (this->data[i] != other.data[i]) return false; return true; }
  bool contains(char ch) const { for (int i = 0; i < WORD_LENGTH; ++i) if (data[i] == ch) return true; return false; }
  int count(char ch) const { int result = 0; for (int i = 0; i < WORD_LENGTH; ++i) if (data[i] == ch) ++result; return result; }
  int first(char ch) const { for (int i = 0; i < WORD_LENGTH; ++i) if (data[i] == ch) return i; return int(-1); }
  size_t wordValue(const LetterCounts& letterCounts) const;
  std::string str() const { std::string result; for (int i = 0; i < WORD_LENGTH; ++i) result += this->data[i]; return result; }
  char data[WORD_LENGTH];
};

struct InputLine
{
  InputLine(const Word& word, const Word& result) : word(word), result(result) {}

  bool checkX(const Word& word, size_t i) const
  {
    if (!word.contains(this->word[i]))
      return true;
    int count = 0;
    for (size_t j = 0; j < i; ++j)
      if (this->word[i] == this->word[j] &&
          this->result[j] != 'x')
        ++count;
    return word.count(this->word[i]) <= count;
  }

  bool matches(const Word& word) const
  {
    for (size_t i = 0; i < WORD_LENGTH; ++i)
      if (result[i] == 'x')
      {
        if (!this->checkX(word, i))
          return false;
      }
      else if (result[i] == 'y')
      {
        if (word[i] == this->word[i])
          return false;
        if (!word.contains(this->word[i]))
          return false;
      }
      else if (result[i] == 'g')
      {
        if (word[i] != this->word[i])
          return false;
      }
    return true;
  }
  bool containsMatching(char ch) const
  {
    for (size_t i = 0; i < WORD_LENGTH; ++i)
      if (this->word[i] == ch && (this->result[i] == 'y' || this->result[i] == 'g'))
        return true;
    return false;
  }

  Word word;
  Word result;
};

struct Input
{
  Input() = default;
  Input(const std::string& filename)
  {
    std::fstream file;
    file.open("input.txt", std::ios::in);
    std::string word, result;
    while(std::getline(file, word) && std::getline(file, result))
      this->data.emplace_back(word, result);
  }
  bool matches(const Word& word) const
  {
    for (const InputLine& line : this->data)
      if (!line.matches(word))
        return false;
    return true;
  }

  std::vector<InputLine> data;
};

InputLine generateGuess(const Word& word, const Word& goalWord)
{
  Word result;
  for (size_t i = 0; i < WORD_LENGTH; ++i)
    if (word[i] == goalWord[i])
      result[i] = 'g';
    else if (!goalWord.contains(word[i]))
      result[i] = 'x';
    else
    {
      int count = 0;
      for (size_t j = 0; j < i; ++j)
        if (word[i] == word[j])
          ++count;
      if (goalWord.count(word[i]) >= count + 1)
        result[i] = 'y';
      else
        result[i] = 'x';
    }
  return InputLine(word, result);
}

class Dictionary;

class LetterCounts
{
public:
  LetterCounts(const Dictionary& dictionary);
  size_t operator[](char ch) const { return this->data[ch - 'a']; }
  size_t& operator[](char ch) { return this->data[ch - 'a']; }
private:
  std::vector<size_t> data;
};

size_t Word::wordValue(const LetterCounts& letterCounts) const
{
  size_t result = 0;
  for (size_t i = 0; i < WORD_LENGTH; ++i)
  {
    bool countedAlready = false;
    for (size_t j = 0; j < i; ++j)
      if (this->data[i] == this->data[j])
        countedAlready = true;
    if (!countedAlready)
      result += letterCounts[this->data[i]];
  }
  return result;
}

class Dictionary : public std::vector<Word>
{
public:
  Dictionary(const std::string filename = "english.txt")
  {
    std::fstream file;
    file.open(filename, std::ios::in);
    if (!file.is_open())
      throw std::runtime_error("Can't open " + filename);
    std::string word;
    while(std::getline(file, word))
      if (word.size() == 5)
        this->insert(word);
    file.close();
  }
  void insert(const Word& word)
  {
    this->push_back(word);
  }
  void remove(const Word& word)
  {
    for (size_t i = 0; i < this->size(); ++i)
      if ((*this)[i] == word)
      {
        this->erase(this->begin() + i);
        return;
      }
  }
  void sortByWordValue()
  {
    LetterCounts letterCounts(*this);
    std::sort(this->begin(), this->end(), [&letterCounts](const Word& a, const Word& b) { return a.wordValue(letterCounts) > b.wordValue(letterCounts); });
  }

  void printWordValues() const
  {
    LetterCounts letterCounts(*this);
    for (const Word& word : *this)
      printf("%s: %u\n", word.str().c_str(), word.wordValue(letterCounts));
  }
  void printMatching(const Input& input) const
  {
    size_t result = 0;
    for (const Word& word : *this)
      if (input.matches(word))
      {
        printf("%s\n", word.str().c_str());
        ++result;
      }
    printf("Count: %u\n", result);
  }
  size_t matchingCount(const Input& input) const
  {
    size_t result = 0;
    for (const Word& word : *this)
      if (input.matches(word))
        ++result;
    return result;
  }
};

LetterCounts::LetterCounts(const Dictionary& dictionary)
{
  this->data.resize('z' - 'a' + 1);
  for (const Word& word : dictionary)
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
      assert(word[i] >= 'a' && word[i] <= 'z');
      ++(*this)[word[i]];
    }
}

class Strategy
{
public:
  virtual Word next(const Input& input, Dictionary& dictionary) const = 0;
  virtual std::string name() const = 0;
};

class SimpleStrategy : public Strategy
{
public:
  SimpleStrategy(const Word& firstWord) : firstWord(firstWord) {}
  virtual Word next(const Input& input, Dictionary& dictionary) const override
  {
    if (input.data.empty())
      return firstWord;
    for (const Word& word : dictionary)
      if (input.matches(word))
        return word;
    return Word();
  }
  virtual std::string name() const override { return "Simple strategy(" + firstWord.str() + ")"; };

  const Word firstWord;
};

class LetterCountStrategy : public Strategy
{
public:
  LetterCountStrategy(const Dictionary& dictionary)
    : sortedDictionary(dictionary)
  {
    sortedDictionary.sortByWordValue();
  }
  virtual Word next(const Input& input, Dictionary& dictionary) const override
  {
    for (const Word& word : this->sortedDictionary)
      if (input.matches(word))
        return word;
    return Word();
  }
  virtual std::string name() const override { return "Letter count strategy"; };

  Dictionary sortedDictionary;
};

// the success rate is 99.9723%, which means there is just one word from the dictionary (I don't know which one, it failed on)
class BestRemoverStrategy : public SimpleStrategy
{
public:
  BestRemoverStrategy(const Word& firstWord, const Dictionary& dictionary)
    : SimpleStrategy(firstWord)
  {}

  virtual Word next(const Input& input, Dictionary& dictionary) const override
  {
    if (input.data.empty())
      return this->firstWord;

    std::vector<size_t> matchingWords;
    for (size_t i = 0; i < dictionary.size(); ++i)
      if (input.matches(dictionary[i]))
        matchingWords.push_back(i);
    if (matchingWords.empty())
      return Word();
    if (matchingWords.size() == 1 || matchingWords.size() > 100 || input.data.size() >= 5)
      return dictionary[matchingWords[0]];

    std::vector<size_t> scores;
    scores.resize(dictionary.size());
    for (size_t goalWord : matchingWords)
    {
      for (size_t candidateIndex = 0; candidateIndex < dictionary.size(); ++candidateIndex)
      {
        const Word& candidate = dictionary[candidateIndex];
        Input inputCopy(input);
        inputCopy.data.emplace_back(generateGuess(candidate, dictionary[goalWord]));
        size_t matchesLeft = 0;
        for (size_t test : matchingWords)
          if (inputCopy.matches(dictionary[test]))
            ++matchesLeft;
        if (matchesLeft == 0)
          throw std::runtime_error("Error");
        scores[candidateIndex] += matchesLeft;
      }
    }

    size_t best = size_t(-1);
    size_t bestScore = size_t(-1);
    for (size_t i = 0; i < dictionary.size(); ++i)
      if (scores[i] && scores[i] < bestScore)
      {
        bestScore = scores[i];
        best = i;
      }
    for (size_t matchingWord : matchingWords)
      if (scores[matchingWord] == bestScore)
        return dictionary[matchingWord];
    return dictionary[best];
  }
  virtual std::string name() const override { return "Best remover strategy"; };
};

size_t testStrategy(Dictionary& dictionary, const Strategy& strategy)
{
  size_t result = 0;
  for (const Word& goalWord : dictionary)
  {
    Input input;
    for (uint32_t i = 0; i < 6; ++i)
    {
      Word next = strategy.next(input, dictionary);
      if (next == goalWord)
      {
        ++result;
        break;
      }
      input.data.emplace_back(generateGuess(next, goalWord));
    }
  }
  printf("%s: %g%%\n", strategy.name().c_str(), 100.0 * double(result) / double(dictionary.size()));
  return result;
}

struct StrategyResult
{
  StrategyResult() = default;
  StrategyResult(Dictionary& dictionary, Strategy* strategy)
    : strategy(strategy)
    , result(testStrategy(dictionary, *strategy))
  {}
  bool operator<(const StrategyResult& other) const { return this->result < other.result; }

  size_t result = 0;
  std::unique_ptr<Strategy> strategy;
};

void interactive(const Strategy& strategy)
{
  Input input;//("input.txt");
  Dictionary dictionary;

  do
  {
    tryAgain:
    Word next = strategy.next(input, dictionary);
    if (!input.data.empty())
      dictionary.printMatching(input);
    printf("next: %s\n", next.str().c_str());
    if (dictionary.matchingCount(input) <= 1)
      return;
    Word wordInput;
    for (size_t i = 0; i < WORD_LENGTH;)
    {
      char ch = _getch();
      if (ch == 'n')
      {
        dictionary.remove(next);
        printf("Discarding ...\n");
        goto tryAgain;
      }
      if (ch != 'x' && ch != 'y' && ch != 'g')
        continue;
      wordInput[i] = ch;
      printf("%c", ch);
      ++i;
    }
    printf("\n");
    input.data.emplace_back(next.str().c_str(), wordInput);
  } while(true);
}

void compareStrategies()
{
  Dictionary dictionary;
  std::vector<StrategyResult> results;
  results.emplace_back(dictionary, new BestRemoverStrategy("tares", dictionary));
  results.emplace_back(dictionary, new SimpleStrategy("raped"));
  results.emplace_back(dictionary, new SimpleStrategy("taper"));

  results.emplace_back(dictionary, new LetterCountStrategy(dictionary));

  std::sort(results.begin(), results.end());
  printf("------ Results\n");
  for (StrategyResult& result : results)
    printf("%s: %g%%\n", result.strategy->name().c_str(), 100.0 * double(result.result) / double(dictionary.size()));
}

void check(Word goal, Word guess, Word result)
{
  assert(generateGuess(guess, goal).result == result);
  Input input;
  input.data.emplace_back(guess, result);
  assert(input.matches(goal));
}

int main()
{

  assert(generateGuess("hello", "proxy").result == "xxxxy");
  assert(generateGuess("house", "proxy").result == "xyxxx");
  assert(generateGuess("shoot", "proxy").result == "xxgxx");
  assert(generateGuess("brown", "proxy").result == "xggxx");
  assert(generateGuess("aroma", "proxy").result == "xggxx");
  assert(generateGuess("proxy", "proxy").result == "ggggg");
  assert(generateGuess("actor", "array").result == "gxxxy");
  assert(generateGuess("array", "actor").result == "gyxxx");
  check("aorta", "aaron", "gygyx");
  check("aorta", "outdo", "yxyxx");
  check("actor", "array", "gyxxx");
  check("appro", "pappy", "yygxx");


  //compareStrategies();

  printf("x = Nothing, y = yellow, g = green, n = discard the provided word\n");
  while (true)
    interactive(BestRemoverStrategy("tares", Dictionary()));
  _getch();
  return 0;
}
