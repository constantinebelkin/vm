#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>

#define VCPU_NUMBER 1

struct Display;
struct Keyboard;
struct ROM;

struct Machine {
    struct Display *display;
    struct Keyboard *keyboard;
    struct ROM *rom;

    Machine(const char firmware_path[]);
    ~Machine();

    void run();

//private:
    struct Impl;
    struct Impl *_impl;
};

#endif
