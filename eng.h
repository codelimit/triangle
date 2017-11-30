#include <iostream>

#ifndef ENG_H_
#define ENG_H_

#ifdef _WIN32
#ifdef BUILD_ENG
#define ENG __declspec(dllexport)
#else
#define ENG __declspec(dllimport)
#endif
#else
#define ENG
#endif

struct vertex {
  float x;
  float y;
  vertex() : x(0.f), y(0.f) {}
};

struct ENG triangle {
  vertex v[3];
  triangle() {
    v[0] = vertex();
    v[1] = vertex();
    v[2] = vertex();
  }
};

std::istream &ENG operator>>(std::istream &is, vertex &);
std::istream &ENG operator>>(std::istream &is, triangle &);

bool ENG initAll();

unsigned int ENG initWindow(int a, int b);

void ENG renderTriangle(const triangle &);

void ENG swapBuffers(unsigned int id);

std::string ENG getEvent();

void ENG shutDown(unsigned int);

#endif /* ENG_H_ */
