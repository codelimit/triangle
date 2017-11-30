#include "eng.h"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  bool running{true};
  std::string pressedBtn;

  if (!initAll()) {
    running = false;
  }

  uint32_t winId = initWindow(480, 480);
  if (winId == 0) {
    running = false;
  }

  while (running) {
    std::string btn = getEvent();
    if (!btn.empty()) {
      if (btn == "QUIT") {
        running = false;
      }
      std::cout << "Button pressed: " << btn << std::endl;
    }

    std::ifstream file("vertexes.txt");
    if (!file) {
      std::cerr << "Could not open the vertex data file " << std::endl;
      running = false;
    }

    triangle tr;
    file >> tr;

    renderTriangle(tr);
    swapBuffers(winId);
  }

  shutDown(winId);

  return 0;
}
