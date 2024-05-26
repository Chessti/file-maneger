#include <ncurses.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

void displayFiles(const std::vector<fs::directory_entry>& files, int selected,
                  const fs::path& markedFile) {
  clear();
  for (size_t i = 0; i < files.size(); ++i) {
    if (files[i].path() == markedFile) {
      attron(A_BOLD | A_UNDERLINE);  // Mark the file
    }
    if (i == selected) {
      attron(A_REVERSE);
    }
    mvprintw(i, 0, "%s", files[i].path().filename().string().c_str());
    if (i == selected) {
      attroff(A_REVERSE);
    }
    if (files[i].path() == markedFile) {
      attroff(A_BOLD | A_UNDERLINE);
    }
  }
  refresh();
}

std::vector<fs::directory_entry> getFiles(const fs::path& path) {
  std::vector<fs::directory_entry> files;
  for (const auto& entry : fs::directory_iterator(path)) {
    files.push_back(entry);
  }
  return files;
}

void deleteFileOrDirectory(const fs::path& path) {
  try {
    if (fs::is_directory(path)) {
      fs::remove_all(path);
    } else {
      fs::remove(path);
    }
  } catch (const fs::filesystem_error& e) {
    mvprintw(LINES - 1, 0, "Error deleting: %s", e.what());
    refresh();
    getch();  // Wait for user input
  }
}

void displayFileContent(const fs::path& filePath) {
  clear();
  std::ifstream file(filePath);
  if (file.is_open()) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();

    int offset = 0;
    int ch;
    while (true) {
      clear();
      for (int i = 0; i < LINES - 1 && i + offset < lines.size(); ++i) {
        mvprintw(i, 0, "%s", lines[i + offset].c_str());
      }
      refresh();
      ch = getch();
      if (ch == 'q' || ch == 27) {  // Exit on 'q' or ESC key
        break;
      } else if (ch == KEY_UP) {
        if (offset > 0) {
          --offset;
        }
      } else if (ch == KEY_DOWN) {
        if (offset < lines.size() - (LINES - 1)) {
          ++offset;
        }
      }
    }
  } else {
    mvprintw(0, 0, "Error opening file: %s", filePath.string().c_str());
    refresh();
    getch();  // Wait for user input to return
  }
}

void copyFile(const fs::path& src, const fs::path& dst) {
  try {
    if (fs::is_directory(src)) {
      fs::copy(src, dst, fs::copy_options::recursive);
    } else {
      fs::copy(src, dst);
    }
  } catch (const fs::filesystem_error& e) {
    mvprintw(LINES - 1, 0, "Error copying: %s", e.what());
    refresh();
    getch();  // Wait for user input
  }
}

void moveFile(const fs::path& src, const fs::path& dst) {
  try {
    fs::rename(src, dst);
  } catch (const fs::filesystem_error& e) {
    mvprintw(LINES - 1, 0, "Error moving: %s", e.what());
    refresh();
    getch();  // Wait for user input
  }
}

int main() {
  initscr();  // Инициализация ncurses
  noecho();  // Отключаем отображение набираемых символов
  cbreak();  // Отключаем буферизацию ввода
  keypad(stdscr, TRUE);  // Включаем поддержку функциональных клавиш

  fs::path current_path = fs::current_path();
  std::vector<fs::directory_entry> files = getFiles(current_path);
  int selected = 0;
  fs::path markedFile;
  enum class Operation { None, Copy, Move } operation = Operation::None;

  displayFiles(files, selected, markedFile);

  int ch;
  while ((ch = getch()) != 'q') {  // Выход из программы при нажатии 'q'
    switch (ch) {
      case KEY_UP:
        if (selected > 0) {
          --selected;
        }
        break;
      case KEY_DOWN:
        if (selected < files.size() - 1) {
          ++selected;
        }
        break;
      case 10:  // Enter key
        if (fs::is_directory(files[selected])) {
          current_path /= files[selected].path().filename();
          files = getFiles(current_path);
          selected = 0;
        }
        break;
      case 263:  // Backspace key
        if (current_path.has_parent_path()) {
          current_path = current_path.parent_path();
          files = getFiles(current_path);
          selected = 0;
        }
        break;
      case KEY_DC:  // Delete key
      case 'd':
        deleteFileOrDirectory(files[selected].path());
        files = getFiles(current_path);
        if (selected >= files.size()) {
          selected = files.size() - 1;
        }
        break;
      case 'p':  // Display file content
        if (fs::is_regular_file(files[selected])) {
          displayFileContent(files[selected].path());
          // Redraw file list after displaying content
          displayFiles(files, selected, markedFile);
        }
        break;
      case 's':  // Mark file
        markedFile = files[selected].path();
        break;
      case 'c':  // Copy marked file
        if (markedFile.empty()) {
          break;
        }
        copyFile(markedFile, current_path / markedFile.filename());
        markedFile.clear();
        files = getFiles(current_path);
        break;
      case 'z':  // Move marked file
        if (markedFile.empty()) {
          break;
        }
        moveFile(markedFile, current_path / markedFile.filename());
        markedFile.clear();
        files = getFiles(current_path);
        break;
      default:
        break;
    }
    displayFiles(files, selected, markedFile);
  }

  endwin();  // Завершение работы с ncurses
  return 0;
}
