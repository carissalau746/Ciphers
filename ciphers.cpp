#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

const string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function declarations go at the top of the file so we can call them
// anywhere in our program, such as in main or in other functions.
// Most other function declarations are in the included header
// files.

// When you add a new helper function, make sure to declare it up here!
vector<char> decryptHelper(const QuadgramScorer& scorer, string text);
/**
 * Print instructions for using the program.
 */
void printMenu();

int main() {
  Random::seed(time(NULL));
  string command;

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";
    
    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;

    //Make the scorer for future use
      ifstream quadgramsFile("english_quadgrams.txt");
      vector<string> quadgrams;
      vector<int> counts;
      string currentLine;
      int count; 
      int cnt = 0;

      while(quadgramsFile >> currentLine) { 
        int commaIndex = currentLine.find(',');
        string quadgram = currentLine.substr(0,commaIndex);
        string fileCount = currentLine.substr(commaIndex+1);
        int count = stoi(fileCount);
        quadgrams.push_back(quadgram);
        counts.push_back(count);
      }
      QuadgramScorer scorer(quadgrams, counts);

    if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    }

    if (command == "C" || command == "c") {
      caesarEncryptCommand();
    }

    if (command == "F" || command == "f") {
      //Decrypting file 
      string inputFile, outputFile, wholeFile;
      getline(cin, inputFile);
      getline(cin, outputFile);
      fstream encryptedFile(inputFile);
      ofstream decryptedFile(outputFile);
      vector<char> cipher;
      bool notFirst = false;
      //cout << "going to start reading in from the file now" << endl;
      while(getline(encryptedFile, currentLine)) {
        wholeFile += currentLine + "\n";
      }
      cipher = decryptHelper(scorer, wholeFile);
      wholeFile = applySubstCipher(cipher, wholeFile);
      decryptedFile << wholeFile;
      cout << wholeFile;
    }

    if (command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    }

    if (command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    }


    if (command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    }

    if (command == "D" || command == "d") {
      ifstream wordsFile ("dictionary.txt");
      vector<string> dictionary;
      string current;
      while(wordsFile >> current) {
        dictionary.push_back(current);
      }
      caesarDecryptCommand(dictionary);
    }

    cout << endl;

  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher from File" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

char rot(char c, int amount) { //wraparound since 26 % 26 = 0
  return ALPHABET.at((ALPHABET.find(c) + amount) % 26);
}

string rot(const string& line, int amount) {
  string output = "";
  for(int i = 0; i < line.length(); i++) {
    char currChar = line.at(i);
    if(isalpha(currChar)) {
      currChar = toupper(currChar);
      currChar = rot(currChar, amount);
      output += currChar;
    }
    else if(isspace(currChar))
      output += currChar;
  }
  return output;
}

void caesarEncryptCommand() {
  string text, inputRotate;
  int rotate;
  getline(cin, text);
  getline(cin, inputRotate);
  rotate = stoi(inputRotate);
  text = rot(text, rotate);
  cout << text;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void rot(vector<string>& strings, int amount) {
  for(int i = 0; i < strings.size(); i++) {
    strings.at(i) = rot(strings.at(i), amount);
  }
}

string clean(const string& s) {
  string textToDecrypt = s;
  string currentWord = "";
  bool wasSpace = true;
  for(int i = 0; i < textToDecrypt.length(); i++) {
    char currChar = textToDecrypt.at(i);
    if(isalpha(currChar)) {
      if(!(isupper(currChar))) 
        currChar = toupper(currChar);
      currentWord += currChar;
      wasSpace = false;
    } //do nothing if not a character
    else if(isspace(currChar) && !wasSpace) {
      wasSpace = true;
    }
  }
  return currentWord;
}

vector<string> splitBySpaces(const string& s) {
  string textToDecrypt = s;
  vector<string> text;
  bool wasSpace = true; //in case multiple consecutive spaces are in one text
  string currentWord = "";
  //Load the words to decrypt into the text vector
  for(int i = 0; i < textToDecrypt.length(); i++) {
    char currChar = textToDecrypt.at(i);
    if(isalpha(currChar)) {
      if(!(isupper(currChar))) 
        currChar = toupper(currChar);
      currentWord += currChar;
      wasSpace = false;
    } //do nothing if not a character
    else if(isspace(currChar) && !wasSpace) {
      //cout << "pushing this : " << currentWord << endl;
      text.push_back(currentWord); 
      currentWord = "";
      wasSpace = true;
    }
  }
  if(!(currentWord.empty())) //If last word was not followed by a space
    text.push_back(currentWord); 
  return text;
}

string joinWithSpaces(const vector<string>& words) {
  string output = "";
  for(int i = 0; i < words.size(); i++) {
    if(i == words.size()-1) {
      output += words.at(i); 
      continue;
    }
    output += words.at(i) + " ";
  }
  return output;
}

bool inDictionary(const string word, const vector<string>& dict) {
  for(int i = 0; i < dict.size(); i++) {
    if(word == dict.at(i))
      return true;
  }
  return false;
}

int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  int count = 0;
  for(int i = 0; i < words.size(); i++) {
    if(inDictionary(words.at(i), dict)) 
      count++;
  }
  return count;
}


void caesarDecryptCommand(const vector<string>& dict) {
  string textToDecrypt;
  vector<string> text;
  getline(cin, textToDecrypt);
  text = splitBySpaces(textToDecrypt);


  //Time to decrypt
  bool foundDecryption = false;
  for(int i = 0; i < 26; i++) { //Rotate
    int validWords = 0;
    string decryptedText = "";
    for(int j = 0; j < text.size(); j++) {
      string current = rot(text.at(j), i);
      if(inDictionary(current, dict)) {
        validWords++;
      } 
      if(j == text.size()-1) { //do not add space at end of decrypted text
        decryptedText += current;
        continue;
      }
      decryptedText += current + " ";
    }
    if(validWords > text.size()/2) {
      cout << decryptedText << endl; 
      foundDecryption = true;
    }
  } 
  if(!foundDecryption) {
    cout << "No good decryptions found";
  }
}

#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstCipher(const vector<char>& cipher, const string& s) {
  string output = "";
  for(int i = 0; i < s.length(); i++) {
    char current = toupper(s.at(i));
    if(isalpha(current)) {
      current = cipher.at(current-65); //ASCII "A" starts at 65
    }
    output += current;
  }
  return output;
}

void applyRandSubstCipherCommand() {
  string text;
  getline(cin, text);
  vector<char> cipher = genRandomSubstCipher();
  cout << applySubstCipher(cipher, text);
}

#pragma endregion SubstEnc

#pragma region SubstDec

double scoreString(const QuadgramScorer& scorer, const string& s) {
  double totalScore = 0.0;
  string text = clean(s);
  //cout << "Scoring the string now: " << text << endl;
  for(int i = 0; i < text.length()-3; i++) {
    string currQuadgram = text.substr(i,4); //make substr length 4
    //cout << "Looking at " << currQuadgram << " " << i << " " << i+4 << endl;
    totalScore += scorer.getScore(currQuadgram);
    //cout << scorer.getScore(text.substr(i,4)) << endl;
  }
  return totalScore;
}


void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  string text;
  getline(cin, text);
  //cout << "Computing englishness of " << text << endl;
  cout << scoreString(scorer, text);
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& ciphertext) {
  vector<char> ogCipher = genRandomSubstCipher();
  string ogCipherString = applySubstCipher(ogCipher, ciphertext);
  double ogCipherScore = scoreString(scorer, ogCipherString);
  int failedTrials = 0; //did not result in a replacement
  vector<char> newCipher;
  int rand1, rand2; //instantiate these outside of the loop
  char temp;  //to prevent taking too long by making/destroying constantly
  string newCipherString;
  double newCipherScore;
  while(failedTrials < 1000){
    newCipher = ogCipher;
    rand1 = Random::randInt(25);
    rand2 = Random::randInt(25);
    while(rand2 == rand1) {
      rand2 = Random::randInt(25);
    }

    temp = newCipher[rand1];
    newCipher[rand1] = newCipher[rand2];
    newCipher[rand2] = temp;
    
    newCipherString = applySubstCipher(newCipher, ciphertext);
    newCipherScore = scoreString(scorer, newCipherString);

    if(newCipherScore > ogCipherScore) {
      failedTrials = 0;
      ogCipher = newCipher;
      ogCipherScore = newCipherScore;
    }
    else 
      failedTrials++;
  }
  return ogCipher;
}

vector<char> decryptHelper(const QuadgramScorer& scorer, string text) {
  vector<char> bestCipher, newCipher;
  double bestScore;
  int newScore;
  for(int i = 0; i < 25; i++) {
      if(i == 0) {
        bestCipher = decryptSubstCipher(scorer, text);
        bestScore = scoreString(scorer, applySubstCipher(bestCipher, text));
        continue;
      }
      newCipher = decryptSubstCipher(scorer, text);
      newScore = scoreString(scorer, applySubstCipher(newCipher, text));
      if(newScore > bestScore) {
        bestCipher = newCipher;
        bestScore = newScore;
      }
  }
  return bestCipher;
}

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  vector<char> bestCipher, newCipher;
  double bestScore;
  int newScore;
  string text; 
  getline(cin, text);
  for(int i = 0; i < 25; i++) {
      if(i == 0) {
        bestCipher = decryptSubstCipher(scorer, text);
        bestScore = scoreString(scorer, applySubstCipher(bestCipher, text));
        continue;
      }
      newCipher = decryptSubstCipher(scorer, text);
      newScore = scoreString(scorer, applySubstCipher(newCipher, text));
      if(newScore > bestScore) {
        bestCipher = newCipher;
        bestScore = newScore;
      }
  }
  cout << applySubstCipher(bestCipher, text);
}

#pragma endregion SubstDec
