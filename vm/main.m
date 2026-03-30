#import "VMApplication.h"

int main(int argc, char const* argv[]) {
    @autoreleasepool {
        VMApplication *const application = VMApplication.sharedApplication;
        application.delegate = application;
        [application run];
    }

    return 0;
}
