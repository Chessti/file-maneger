#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

// Function to get character from terminal without waiting for enter
char getch() {
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0) perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
  return buf;
}

class FileManager {
 public:
  FileManager() : selected_index(0) {
    if (getcwd(current_path, sizeof(current_path)) == nullptr) {
      perror("getcwd() error");
    }
    update_directory_listing();
  }

  void run() {
    char key;
    while (true) {
      display_directory();
      key = getch();

      switch (key) {
        case 'w':  // up
          move_cursor_up();
          break;
        case 's':  // down
          move_cursor_down();
          break;
        case '\n':  // enter
          enter_directory();
          break;
        case 8:  // backspace
          move_to_parent_directory();
          break;
        case 'd':  // del (mapped to 'd' for simplicity)
          delete_file_or_directory();
          break;
        case 'q':  // quit
          return;
        default:
          break;
      }
    }
  }

 private:
  std::vector<std::string> directory_entries;
  size_t selected_index;
  char current_path[PATH_MAX];

  void update_directory_listing() {
    directory_entries.clear();
    DIR* dir = opendir(current_path);
    if (dir) {
      struct dirent* entry;
      while ((entry = readdir(dir)) != nullptr) {
        directory_entries.push_back(entry->d_name);
      }
      closedir(dir);
    }
    selected_index = 0;
  }

  void display_directory() const {
    system("clear");  // clear the screen
    std::cout << "Current directory: " << current_path << "\n\n";
    for (size_t i = 0; i < directory_entries.size(); ++i) {
      if (i == selected_index) {
        std::cout << "> ";
      } else {
        std::cout << "  ";
      }
      std::cout << directory_entries[i];
      struct stat sb;
      if (stat(directory_entries[i].c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        std::cout << "/";
      }
      std::cout << "\n";
    }
  }

  void move_cursor_up() {
    if (selected_index > 0) {
      --selected_index;
    }
  }

  void move_cursor_down() {
    if (selected_index < directory_entries.size() - 1) {
      ++selected_index;
    }
  }

  void enter_directory() {
    struct stat sb;
    if (stat(directory_entries[selected_index].c_str(), &sb) == 0 &&
        S_ISDIR(sb.st_mode)) {
      chdir(directory_entries[selected_index].c_str());
      if (getcwd(current_path, sizeof(current_path)) == nullptr) {
        perror("getcwd() error");
      }
      update_directory_listing();
    }
  }

  void move_to_parent_directory() {
    chdir("..");
    if (getcwd(current_path, sizeof(current_path)) == nullptr) {
      perror("getcwd() error");
    }
    update_directory_listing();
  }

  void delete_file_or_directory() {
    struct stat sb;
    if (stat(directory_entries[selected_index].c_str(), &sb) == 0) {
      if (S_ISDIR(sb.st_mode)) {
        rmdir(directory_entries[selected_index].c_str());
      } else {
        unlink(directory_entries[selected_index].c_str());
      }
      update_directory_listing();
    }
  }
};

int main() {
  FileManager fm;
  fm.run();
  return 0;
}
