#pragma once

#include <png.h>
#include <cstdint>
#include <vector>
#include <future>

void write_png_file(const std::string& name, const int width, const int height, uint8_t* rgba, std::vector<uint8_t>& buf);
void write_png_proc(const std::string& name, const int width, const int height, uint8_t* rgba, uint8_t* data, std::promise<void> pr);