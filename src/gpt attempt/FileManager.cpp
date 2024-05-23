#include "FileManager.h"

#include <termios.h>
#include <unistd.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

FileManager::FileManager()
    : currentPath(fs::current_path()), cursorPosition(0), markedFile("") {}

void FileManager::run() {
  listFiles(currentPath);
  while (true) {
    display();
    handleInput();
  }
}

void FileManager::display() {
  system("clear");
  for (size_t i = 0; i < files.size(); ++i) {
    if (i == cursorPosition) {
      std::cout << "> " << files[i] << std::endl;
    } else {
      std::cout << "  " << files[i] << std::endl;
    }
  }
}

void FileManager::handleInput() {
  char ch = getch();
  switch (ch) {
    case 'w':
      if (cursorPosition > 0) cursorPosition--;
      break;
    case 's':
      if (cursorPosition < files.size() - 1) cursorPosition++;
      break;
    case 10:  // Enter key
      if (fs::is_directory(files[cursorPosition])) {
        changeDirectory(files[cursorPosition]);
      }
      break;
    case 127:  // Backspace key
      changeDirectory("..");
      break;
    case 'd':
      deleteFile(files[cursorPosition]);
      break;
    case 'm':
      markedFile = files[cursorPosition];
      break;
    case 'c':
      if (!markedFile.empty()) {
        copyFile(markedFile, currentPath);
        markedFile = "";
      }
      break;
    case 'v':
      if (!markedFile.empty()) {
        moveFile(markedFile, currentPath);
        markedFile = "";
      }
      break;
    case 'p':
      printFile(files[cursorPosition]);
      break;
    default:
      break;
  }
}

void FileManager::listFiles(const std::string& path) {
  files.clear();
  for (const auto& entry : fs::directory_iterator(path)) {
    files.push_back(entry.path().filename().string());
  }
  cursorPosition = 0;
}

void FileManager::changeDirectory(const std::string& path) {
  currentPath = fs::canonical(fs::path(currentPath) / path);
  std::cout << "Changed directory to: " << currentPath
            << std::endl;  // Отладочный вывод
  listFiles(currentPath);
}

void FileManager::deleteFile(const std::string& path) {
  fs::remove(fs::path(currentPath) / path);
  listFiles(currentPath);
}

void FileManager::copyFile(const std::string& source,
                           const std::string& destination) {
  fs::copy(fs::path(currentPath) / source, fs::path(currentPath) / destination);
  listFiles(currentPath);
}

void FileManager::moveFile(const std::string& source,
                           const std::string& destination) {
  fs::rename(fs::path(currentPath) / source,
             fs::path(currentPath) / destination);
  listFiles(currentPath);
}

void FileManager::printFile(const std::string& path) {
  std::ifstream file(fs::path(currentPath) / path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      std::cout << line << std::endl;
    }
    file.close();
  }
  std::cout << "Press any key to return..." << std::endl;
  getch();
}

char FileManager::getch() {
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0) {
    perror("tcsetattr()");
  }
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0) {
    perror("tcsetattr ICANON");
  }
  if (read(0, &buf, 1) < 0) {
    perror("read()");
  }
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0) {
    perror("tcsetattr ~ICANON");
  }
  return buf;
}
