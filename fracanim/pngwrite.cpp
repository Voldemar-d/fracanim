#include "pngwrite.h"

#pragma warning(push)
#pragma warning(disable:4996)

void write_png_file(const std::string& name, const int width, const int height, uint8_t* rgba, std::vector<uint8_t>& buf) {
	auto fp = fopen(name.c_str(), "wb");
	if (!fp)
		return;
	auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	for (;;) {
		if (!png_ptr)
			break;
		auto png_info = png_create_info_struct(png_ptr);
		if (!png_info)
			break;
		if (setjmp(png_jmpbuf(png_ptr)))
			break;
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, png_info, width, height, 8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
		buf.resize(width * height * 3);
		auto data = buf.data();
		std::vector<uint8_t*> rows(height);
		for (int i = 0; i < height; i++) {
			rows[i] = data + (i * width * 3);
			for (int j = 0; j < width; j++) {
				int i1 = (i * width + j) * 3, i2 = (i * width + j) * 4;
				data[i1++] = rgba[i2++];
				data[i1++] = rgba[i2++];
				data[i1++] = rgba[i2++];
			}
		}
		png_set_rows(png_ptr, png_info, rows.data());
		png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, nullptr);
		png_write_end(png_ptr, png_info);
		break;
	}

	png_destroy_write_struct(&png_ptr, nullptr);
	fclose(fp);
}

void write_png_proc(const std::string& name, const int width, const int height, uint8_t* rgba, uint8_t* data, std::promise<void> pr) {
	auto fp = fopen(name.c_str(), "wb");
	if (!fp)
		return;
	auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	for (;;) {
		if (!png_ptr)
			break;
		auto png_info = png_create_info_struct(png_ptr);
		if (!png_info)
			break;
		if (setjmp(png_jmpbuf(png_ptr)))
			break;
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, png_info, width, height, 8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
		std::vector<uint8_t*> rows(height);
		for (int i = 0; i < height; i++) {
			rows[i] = data + (i * width * 3);
			for (int j = 0; j < width; j++) {
				int i1 = (i * width + j) * 3, i2 = (i * width + j) * 4;
				data[i1++] = rgba[i2++];
				data[i1++] = rgba[i2++];
				data[i1++] = rgba[i2++];
			}
		}
		// signal start saving image to file
		pr.set_value();
		png_set_rows(png_ptr, png_info, rows.data());
		png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, nullptr);
		png_write_end(png_ptr, png_info);
		break;
	}

	png_destroy_write_struct(&png_ptr, nullptr);
	fclose(fp);
}

#pragma warning(pop)