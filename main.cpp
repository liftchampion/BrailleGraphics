#include <iostream>
#include <fstream>

#include "GraphicsDrawer.h"


std::map<uint64_t, int> parse_map(const std::string& filename) {
  std::map<uint64_t, int> res;
  std::ifstream f;
  f.open(filename);
  while (f) {
    uint64_t key;
    int val;
    f >> key;
    f.ignore();
    f >> val;
    res[key] = val;
  }
  return res;
}

int main()
{
  std::map<uint64_t, int> map = parse_map("/Users/ggerardy/CLION/coursera_brown/graphics/data");
  GraphicsDrawer<uint64_t, int> graphics_drawer(map.begin(), std::next(map.begin(), 100));

  graphics_drawer.draw();

  return 0;
}