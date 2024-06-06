// fracanim.h : Include file for standard system include files,
// or project specific include files.

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
