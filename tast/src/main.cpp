#include "tinytast.hpp"
#include "jsonkit_config.h"

#include <string.h>

int main(int argc, char* argv[])
{
    if (argc > 1 && strcmp(argv[1], "--log") == 0)
    {
        jsonkit::set_logreport(stderr);
        return RUN_TAST(argc-1, argv+1);
    }
    return RUN_TAST(argc, argv);
}
