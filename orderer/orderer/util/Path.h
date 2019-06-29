#pragma once

#include <string>
#include <unistd.h>

namespace MED {

    namespace STRING {

        std::string GetCurrentWorkingDir(void) {
            char buff[FILENAME_MAX];
            getcwd(buff, FILENAME_MAX);
            std::string current_working_dir(buff);
            return current_working_dir;
        }

    }
}


