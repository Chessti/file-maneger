#include <dirent.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<string> getDirectoryContents(const string& path) {
  vector<string> contents;
  DIR* dir = opendir(path.c_str());
  if (dir == nullptr) {
    perror("opendir");
    return contents;
  }
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    contents.push_back(entry->d_name);
  }
  closedir(dir);
  return contents;
}

void displayDirectoryContents(const vector<string>& contents, int highlight) {
  for (int i = 0; i < contents.size(); ++i) {
    if (i == highlight) {
      attron(A_REVERSE);
    }
    mvprintw(i, 0, contents[i].c_str());
    if (i == highlight) {
      attroff(A_REVERSE);
    }
  }
}

int main() {
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  string currentPath = ".";
  int highlight = 0;
  vector<string> contents = getDirectoryContents(currentPath);

  while (true) {
    clear();
    contents = getDirectoryContents(currentPath);
    displayDirectoryContents(contents, highlight);
    refresh();

    int ch = getch();
    switch (ch) {
      case KEY_UP:
        if (highlight > 0) {
          highlight--;
        }
        break;
      case KEY_DOWN:
        if (highlight < contents.size() - 1) {
          highlight++;
        }
        break;
      case 10:  // Enter key
      {
        string selected = contents[highlight];
        struct stat info;
        if (stat((currentPath + "/" + selected).c_str(), &info) == 0 &&
            S_ISDIR(info.st_mode)) {
          currentPath += "/" + selected;
          highlight = 0;
        }
      } break;
      case 127:  // Backspace key
        if (currentPath != ".") {
          size_t pos = currentPath.find_last_of('/');
          currentPath = currentPath.substr(0, pos);
          highlight = 0;
        }
        break;
      case 'q':  // Exit program
        endwin();
        return 0;
      default:
        break;
    }
  }

  endwin();
  return 0;
}
