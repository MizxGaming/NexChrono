#ifndef LARGE_DIGITS_H
#define LARGE_DIGITS_H

#include <vector>
#include <string>

// Using a std::vector for easy indexing and management.
// Each digit's art is normalized to the same height for consistent rendering.
const std::vector<std::vector<std::string>> large_digits_data = {
    // 0
    {
        "   ___    ",
        "  / _ \\   ",
        " | | | |  ",
        " | | | |  ",
        " | |_| |  ",
        "  \\___/   ",
        "         "
    },
    // 1
    {
        "   __    ",
        "  /  |   ",
        "  `| |   ",
        "   | |   ",
        "  _| |_  ",
        " |_____| ",
        "         "
    },
    // 2
    {
        "   _____   ",
        "  / ___ `. ",
        " |_/___) | ",
        "  .'____.' ",
        " / /_____  ",
        " |_______| ",
        "         "
    },
    // 3
    {
        " ##### ",
        "#     #",
        "      #",
        " ##### ",
        "      #",
        "#     #",
        " ##### "
    },
    // 4
    {
        "#     #",
        "#     #",
        "#     #",
        " ##### ",
        "      #",
        "      #",
        "      #"
    },
    // 5
    {
        " ##### ",
        "#     #",
        "#      ",
        " ##### ",
        "      #",
        "#     #",
        " ##### "
    },
    // 6
    {
        " ##### ",
        "#      ",
        "#      ",
        " ##### ",
        "#     #",
        "#     #",
        " ##### "
    },
    // 7
    {
        "#######",
        "      #",
        "      #",
        "     # ",
        "    #  ",
        "   #   ",
        "  #    "
    },
    // 8
    {
        " ##### ",
        "#     #",
        "#     #",
        " ##### ",
        "#     #",
        "#     #",
        " ##### "
    },
    // 9
    {
        " ##### ",
        "#     #",
        "#     #",
        " ##### ",
        "      #",
        "      #",
        " ##### "
    },
    // : (colon) at index 10
    {
        "       ",
        "   #   ",
        "       ",
        "       ",
        "   #   ",
        "       ",
        "       "
    }
};

#endif // LARGE_DIGITS_H
