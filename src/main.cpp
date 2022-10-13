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

bool compareSet(const std::set<path> & pathsA, const std::set<path> & pathsB)
{
    if(pathsA.size() != pathsB.size())
        return false;
    return pathsA == pathsB;
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

std::size_t createCombinedHash(const std::set<path> & paths)
{
    std::size_t seed = 0;
    for(auto const & path : paths)
    {
        hash_combine(seed,hash_value(path));
    }
    return seed;
}

struct collatedFilePaths
{
    std::set<path> fileNames;
    std::set<path> parentPaths;
    collatedFilePaths(std::set<path> const & infileNames, std::set<path> const & inParentPaths)
    :fileNames(infileNames),parentPaths(inParentPaths)
    {}
};

void printToFile(std::map<std::size_t,std::map<path,std::set<path>>> const & v) {
    auto path = current_path().append("output.txt");
    if (exists(path))
    {
        std::remove("output.txt");
    }
    auto output = std::ofstream{ "output.txt" };

    std::map<std::size_t,collatedFilePaths> combinedSameParentPath;
    for (const auto& i : v)
    {
        if(i.second.size() > 1)
        {
            for (auto it1 = i.second.begin(); (it1 != i.second.end()); ++it1)
            {
                for (auto it = std::next(it1, 1); it != i.second.end(); ++it)
                {
                    if(compareSet(it1->second, it->second))
                    {
                        auto combinedHash = createCombinedHash(it->second);
                        auto iter = combinedSameParentPath.find(combinedHash);
                        if(iter != combinedSameParentPath.end())
                        {
                           //hash is present update the fileNames
                            iter->second.fileNames.emplace(it1->first);
                            iter->second.fileNames.emplace(it->first);
                        }
                        else
                        {
                            std::set<std::filesystem::path> fileNames;
                            fileNames.emplace(it1->first);
                            fileNames.emplace(it->first);
                            combinedSameParentPath.emplace(combinedHash, collatedFilePaths(fileNames,it1->second));
                        }   
                        // create a combined hash for all the directories
                        // cache the hash in an unorderd_map 
                    }
                    else
                    {
                        auto combinedHash = createCombinedHash(it->second);
                        std::set<std::filesystem::path> fileNames;
                        fileNames.emplace(it->first);
                        combinedSameParentPath.emplace(combinedHash, collatedFilePaths(fileNames,it1->second));
                    }
                }
            }
        }
        else
        {
            for (auto it1 = i.second.begin(); (it1 != i.second.end()); ++it1)
            {
                auto combinedHash = createCombinedHash(it1->second);
                std::set<std::filesystem::path> fileNames;
                fileNames.emplace(it1->first);
                combinedSameParentPath.emplace(combinedHash, collatedFilePaths(fileNames,it1->second));
            }
        }
    }
    for (const auto& i : combinedSameParentPath)
    {
        if(!i.second.fileNames.empty())
        {
            if(i.second.fileNames.size() > 1)
            {
                std::string s = i.second.fileNames.begin()->u8string();
                std::for_each(std::next(i.second.fileNames.begin()), i.second.fileNames.end(), [&s] (std::filesystem::path const &val) {
                    s.append(", ").append(val.u8string());
                });
                output << s << '\n';
                for (const auto& k : i.second.parentPaths)
                {
                    output << '\t' << k << '\n';
                }
            }
            else
            {
                output << *i.second.fileNames.begin() << '\n';
                for (const auto& k : i.second.parentPaths)
                {
                    output << '\t' << k << '\n';
                }
            }
        }
    }
}

std::map<std::size_t,std::map<path,std::set<path>>> findDuplicates(std::map<std::size_t,std::vector<path>> const & iFilePaths) {
    std::map<std::size_t,std::map<path,std::set<path>>> allDuplicates;
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
                    duplicates[it1->filename()].emplace(it1->parent_path());
                    duplicates[it1->filename()].emplace(it2->parent_path());
                }
            }
        }
        allDuplicates.emplace(pair.first,duplicates);
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