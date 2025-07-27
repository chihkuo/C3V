#include <iostream>
#include <iomanip>
#include "utility.h"


void printDurationText(const char* text, double duration) {
    const char* color;

    //return;    

    if (duration < 5.0)
        color = GREEN_COLOR;
    else if (duration < 10.0)
        color = YELLOW_COLOR;
    else
        color = RED_COLOR;

    std::cout << std::left << std::fixed << std::setprecision(2) << "  " << color << std::setw(35) << text
              << std::right << std::setw(5) << duration << " ms" << RESET_COLOR << std::endl;
}
