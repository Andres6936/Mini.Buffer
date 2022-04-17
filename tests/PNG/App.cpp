#include <SPNG/spng.h>
#include <MiniFB.h>

#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <vector>

class Decoder
{

private:

	spng_ctx* ctx = nullptr;
	std::vector <unsigned char> image;

public:

	Decoder(int argc, char** argv)
	{
		FILE* png;
		int ret = 0;


		if (argc < 2)
		{
			printf("no input file\n");
			return;
		}

		png = fopen(argv[1], "rb");

		if (png == nullptr)
		{
			printf("error opening input file %s\n", argv[1]);
			return;
		}

		ctx = spng_ctx_new(0);

		if (ctx == nullptr)
		{
			printf("spng_ctx_new() failed\n");
			return;
		}

		/* Ignore and don't calculate chunk CRC's */
		spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

		/* Set memory usage limits for storing standard and unknown chunks,
		   this is important when reading untrusted files! */
		size_t limit = 1024 * 1024 * 64;
		spng_set_chunk_limits(ctx, limit, limit);

		/* Set source PNG */
		spng_set_png_file(ctx, png); /* or _buffer(), _stream() */

		spng_ihdr ihdr{ };
		ret = spng_get_ihdr(ctx, &ihdr);

		if (ret)
		{
			printf("spng_get_ihdr() error: %s\n", spng_strerror(ret));
			return;
		}

		const char* color_name = color_type_str(static_cast<spng_color_type>(ihdr.color_type));

		printf("width: %u\n"
			   "height: %u\n"
			   "bit depth: %u\n"
			   "color type: %u - %s\n",
				ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);

		printf("compression method: %u\n"
			   "filter method: %u\n"
			   "interlace method: %u\n",
				ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);

		struct spng_plte plte = { 0 };
		ret = spng_get_plte(ctx, &plte);

		if (ret && ret != SPNG_ECHUNKAVAIL)
		{
			printf("spng_get_plte() error: %s\n", spng_strerror(ret));
			return;
		}

		if (!ret)
		{ printf("palette entries: %u\n", plte.n_entries); }


		size_t image_size;

		/* Output format, does not depend on source PNG format except for
		   SPNG_FMT_PNG, which is the PNG's format in host-endian or
		   big-endian for SPNG_FMT_RAW.
		   Note that for these two formats <8-bit images are left byte-packed */
		int fmt = SPNG_FMT_PNG;

		/* With SPNG_FMT_PNG indexed color images are output as palette indices,
		   pick another format to expand them. */
		if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
		{ fmt = SPNG_FMT_RGB8; }

		ret = spng_decoded_image_size(ctx, fmt, &image_size);

		if (ret)
		{ return; }

		image.resize(image_size);

		if (image.size() != image_size)
		{ return; }

		// Decode the image in one go
		ret = spng_decode_image(ctx, image.data(), image_size, SPNG_FMT_RGBA8, 0);

		if (ret)
		{
			printf("spng_decode_image() error: %s\n", spng_strerror(ret));
			return;
		}
	}

	~Decoder()
	{
		spng_ctx_free(ctx);
	}

	static const char* color_type_str(enum spng_color_type color_type)
	{
		switch (color_type)
		{
		case SPNG_COLOR_TYPE_GRAYSCALE:
			return "grayscale";
		case SPNG_COLOR_TYPE_TRUECOLOR:
			return "truecolor";
		case SPNG_COLOR_TYPE_INDEXED:
			return "indexed color";
		case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
			return "grayscale with alpha";
		case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
			return "truecolor with alpha";
		default:
			return "(invalid)";
		}
	}
};

class App
{

private:

	bool isRunning = true;
	mfb_window* window{ nullptr };
	std::vector <unsigned char> buffer;

public:

	App()
	{
		window = mfb_open_ex("my display", 800, 600, WF_RESIZABLE);
		buffer.resize(800 * 600 * 4);
		std::fill(buffer.begin(), buffer.end(), 255);
	}

	~App()
	{
		window = nullptr;
	}


	int run(int argc, char** argv)
	{
		if (argc < 2)
		{
			throw std::exception("No Input File for Read");
		}

		Decoder decoder{ argc, argv };

		do
		{
			int state = mfb_update_ex(window, buffer.data(), 800, 600);

			if (state < 0)
			{
				isRunning = false;
			}


			// Note that, by default, if ESC key is pressed mfb_update / mfb_update_ex
			// will return -1 (and the window will have been destroyed internally).
		} while (mfb_wait_sync(window) && isRunning);

		return 0;
	}

};

int main(int argc, char** argv)
{
	try
	{
		return App().run(argc, argv);
	}
	catch (std::exception& exception)
	{
		std::cout << "Exception Message: " << exception.what();
		return -1;
	}
}