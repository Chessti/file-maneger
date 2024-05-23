#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>

class FileManager {
 public:
  FileManager();
  void run();

 private:
  void display();
  void handleInput();
  void listFiles(const std::string& path);
  void changeDirectory(const std::string& path);
  void deleteFile(const std::string& path);
  void copyFile(const std::string& source, const std::string& destination);
  void moveFile(const std::string& source, const std::string& destination);
  void printFile(const std::string& path);
  char getch();

  std::string currentPath;
  std::vector<std::string> files;
  int cursorPosition;
  std::string markedFile;
};

#endif
