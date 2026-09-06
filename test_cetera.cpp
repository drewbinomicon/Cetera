#define main cetera_main
#include "cetera.cpp"
#undef main

#include <iostream>
#include <cassert>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

void test_CollectFiles() {
    fs::path temp_dir = fs::temp_directory_path() / "test_collect_files_xyz";
    fs::remove_all(temp_dir); // Ensure it's clean
    fs::create_directories(temp_dir);
    fs::create_directories(temp_dir / "subdir");

    auto touch = [](const fs::path& p) {
        std::ofstream(p).put('a');
    };

    // Files that should be collected
    touch(temp_dir / "test1.png");
    touch(temp_dir / "test2.JPG");
    touch(temp_dir / "subdir" / "test4.jpeg");
    touch(temp_dir / "subdir" / "test5.zip");
    touch(temp_dir / "subdir" / "test6.gz");
    touch(temp_dir / "a.png");
    touch(temp_dir / "b.jpeg");
    touch(temp_dir / "subdir" / ".png"); // Handled as length 4, ends in .png

    // Files that should NOT be collected
    touch(temp_dir / "test3.txt");
    touch(temp_dir / "subdir" / "test7.bin");
    touch(temp_dir / "subdir" / "noext");

    std::vector<std::string> files;
    CollectFiles(temp_dir.string(), files);

    std::sort(files.begin(), files.end());

    std::cout << "Files found:\n";
    for(const auto& f : files) std::cout << f << "\n";



    assert(files.size() == 8);

    std::vector<std::string> expected_files = {
        (temp_dir / "a.png").string(),
        (temp_dir / "b.jpeg").string(),
        (temp_dir / "subdir" / ".png").string(),
        (temp_dir / "subdir" / "test4.jpeg").string(),
        (temp_dir / "subdir" / "test5.zip").string(),
        (temp_dir / "subdir" / "test6.gz").string(),
        (temp_dir / "test1.png").string(),
        (temp_dir / "test2.JPG").string()
    };
    std::sort(expected_files.begin(), expected_files.end());

    for (size_t i = 0; i < files.size(); ++i) {
        assert(files[i] == expected_files[i]);
    }

    fs::remove_all(temp_dir);
    std::cout << "All CollectFiles tests passed!\n";
}

int main() {
    test_CollectFiles();
    return 0;
}
