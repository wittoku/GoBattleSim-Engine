
#include "GoBattleSim_extern.h"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

std::string get_file_contents(std::ifstream &ifs)
{
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: input [game_master]" << std::endl;
        return -1;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs.good())
    {
        std::cerr << "bad file: " << argv[1] << std::endl;
        return -1;
    }

    if (argc >= 3)
    {
        std::ifstream gm_ifs(argv[2]);
        if (!gm_ifs.good())
        {
            std::cerr << "bad file: " << argv[2] << std::endl;
            return -1;
        }
        GBS_config(get_file_contents(gm_ifs).c_str());
    }

    GBS_prepare(get_file_contents(ifs).c_str());

    GBS_run();

    auto output = GBS_collect();

    std::cout << output << std::endl;

    return 0;
}
