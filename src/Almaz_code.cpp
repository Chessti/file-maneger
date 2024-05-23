#include <conio.h>
#include <tchar.h>
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

class FileManager {
 public:
  FileManager() : selected_index(0) {
    GetCurrentDirectory(MAX_PATH, current_path);
    update_directory_listing();
  }

  void run() {
    char key;
    while (true) {
      display_directory();
      key = _getch();

      switch (key) {
        case 72:  // up arrow
          move_cursor_up();
          break;
        case 80:  // down arrow
          move_cursor_down();
          break;
        case 13:  // enter
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
  std::vector<std::wstring> directory_entries;
  size_t selected_index;
  wchar_t current_path[MAX_PATH];

  void update_directory_listing() {
    directory_entries.clear();
    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile(L"*", &find_data);

    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        directory_entries.push_back(find_data.cFileName);
      } while (FindNextFile(hFind, &find_data));
      FindClose(hFind);
    }
    selected_index = 0;
  }

  void display_directory() const {
    system("cls");  // clear the screen
    std::wcout << L"Current directory: " << current_path << L"\n\n";
    for (size_t i = 0; i < directory_entries.size(); ++i) {
      if (i == selected_index) {
        std::wcout << L"> ";
      } else {
        std::wcout << L"  ";
      }
      std::wcout << directory_entries[i];
      if (is_directory(directory_entries[i])) {
        std::wcout << L"/";
      }
      std::wcout << L"\n";
    }
  }

  bool is_directory(const std::wstring& name) const {
    DWORD attributes = GetFileAttributes(name.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY));
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
    if (is_directory(directory_entries[selected_index])) {
      SetCurrentDirectory(directory_entries[selected_index].c_str());
      GetCurrentDirectory(MAX_PATH, current_path);
      update_directory_listing();
    }
  }

  void move_to_parent_directory() {
    SetCurrentDirectory(L"..");
    GetCurrentDirectory(MAX_PATH, current_path);
    update_directory_listing();
  }

  void delete_file_or_directory() {
    if (is_directory(directory_entries[selected_index])) {
      RemoveDirectory(directory_entries[selected_index].c_str());
    } else {
      DeleteFile(directory_entries[selected_index].c_str());
    }
    update_directory_listing();
  }
};

int main() {
  FileManager fm;
  fm.run();
  return 0;
}