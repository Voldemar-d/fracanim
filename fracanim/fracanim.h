#pragma once

#include <chrono>

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif
