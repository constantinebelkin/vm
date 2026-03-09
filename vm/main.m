#import "VMApplication.h"

int main(int argc, char const* argv[]) {
    VMApplication *const application = [VMApplication sharedApplication];
    [application setDelegate:application];
    [application run];
    [application dealloc];
    return 0;
}
