#ifndef ROM_H
#define ROM_H

#define ROM_ADDRESS 0x0

struct ROM {
    ROM(const char firmware_path[]);
    ~ROM();

private:
    struct Impl;
    struct Impl *_impl;
};

#endif
