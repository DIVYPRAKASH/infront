#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <utility>
#include <string>

using namespace std::filesystem;

auto isEqualFile(const path& a, const path& b) {
  if (a.filename() != b.filename())
    return false;
  std::ifstream fileA( a, std::ios::binary );
  std::ifstream fileB( b, std::ios::binary );

  char c{}, d{};
  while (fileA.read(reinterpret_cast<char*>(&c), sizeof c) 
      && fileB.read(reinterpret_cast<char*>(&d), sizeof d)) {
    if (c != d) return false; 
    if (!fileA.good() && fileB.good()) return false;
    if (fileA.good() && !fileB.good()) return false;
  } 

  return true;
}

void printToFile(std::vector<std::map<path,std::set<path>>> const & v) {
    auto path = current_path().append("output.txt");
    if (exists(path))
    {
        std::remove("output.txt");
    }
    auto output = std::ofstream{ "output.txt" };

    for (const auto& i : v)
    {
        for (const auto& j : i)
        {
            output << j.first << '\n';
            for (const auto& k : j.second)
            {
                output << '\t' << k.parent_path() << '\n';
            }
        }
    }
}

std::vector<std::map<path,std::set<path>>> findDuplicates(std::map<std::size_t,std::vector<path>> const & iFilePaths) {
    std::vector<std::map<path,std::set<path>>> allDuplicates;
    for(const auto & pair : iFilePaths)
    {
        std::map<path,std::set<path>>duplicates;
        for (auto it1 = pair.second.begin(); (it1 != pair.second.end()); ++it1)
        {
            for (auto it2 = std::next(it1,1); (it2 != pair.second.end()); ++it2)
            {
                if(isEqualFile(*it1, *it2))
                {
                    // Woot! Found some...
                    duplicates[it1->filename()].emplace(*it1);
                    duplicates[it1->filename()].emplace(*it2);
                }
            }
        }
        allDuplicates.emplace_back(duplicates);
    }

  return allDuplicates;
}

int main(int argc, char* argv[]) {
    auto inputDirectoryPath = path{};
    if(argc > 2) 
    { 
        std::cout << "Only directory path is needed." << std::endl;
        return -1;
    }
    else if(argc == 1)
    {
        inputDirectoryPath.assign(current_path());
    }
    else if(argc == 2) 
    {
        inputDirectoryPath.assign(argv[1]);
    }

    if (!exists(inputDirectoryPath))
    {
        std::cout << "path does not exist" << std::endl;
        return -1;
    }
    
    if (!is_directory(inputDirectoryPath))
    {
        std::cout << "Only directory path is needed." << std::endl;
        return -1;
    }

    if (is_empty(inputDirectoryPath))
    {
        std::cout << "directory is empty\n";
        return -1;
    }

    std::map<std::size_t,std::vector<path>> filePaths;
    for (const auto& i : recursive_directory_iterator(inputDirectoryPath)) 
    {
        if (is_regular_file(i))
        {
            filePaths[file_size(i.path())].emplace_back(i.path());
        }
    }

  auto duplicates = findDuplicates(filePaths);
  printToFile(duplicates);
}