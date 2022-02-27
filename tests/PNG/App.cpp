#include <spng.h>

#include <inttypes.h>
#include <stdio.h>
#include <iostream>

class App
{

private:

	int ret = 0;
	FILE* png;
	spng_ihdr ihdr;
	spng_plte plte = { 0 };
	spng_text* text = NULL;
	spng_ctx* ctx = NULL;
	size_t out_size, out_width;
	unsigned char* out = NULL;
	/* Output format, does not depend on source PNG format except for
		   SPNG_FMT_PNG, which is the PNG's format in host-endian or
		   big-endian for SPNG_FMT_RAW.
		   Note that for these two formats <8-bit images are left byte-packed */
	int fmt = SPNG_FMT_PNG;
	/* Creating an encoder context requires a flag */
	spng_ctx* enc = spng_ctx_new(SPNG_CTX_ENCODER);

	bool isNoText = false;

public:

	App()
	{
		ctx = spng_ctx_new(0);
	}

	~App()
	{
		spng_ctx_free(ctx);
		spng_ctx_free(enc);
		free(out);
	}

	static std::string getTypeColor(const spng_ihdr& ihdr) {
		if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE)
		{
			return "grayscale";
		}
		else if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR)
		{
			return "truecolor";
		}
		else if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
		{
			return "indexed color";
		}
		else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA)
		{
			return "grayscale with alpha";
		}
		else
		{
			return "truecolor with alpha";
		}
	}

	static void printGeneralInformation(const spng_ihdr& ihdr){
		std::cout << "width: " << ihdr.width << "\n";
		std::cout << "height: " << ihdr.height << "\n";
		std::cout << "bit depth: " << ihdr.bit_depth << "\n";
		std::cout << "color type: " << ihdr.color_type << " - " << getTypeColor(ihdr) << "\n";

		std::cout << "compression method: " << ihdr.compression_method << "\n";
		std::cout << "filter method: " << ihdr.filter_method << "\n";
		std::cout << "interlace method: " << ihdr.interlace_method << "\n";
	}

	int run(int argc, char** argv)
	{
		if (argc < 2)
		{
			printf("no input file\n");
			throw std::exception("error");
		}

		png = fopen(argv[1], "rb");

		if (png == NULL)
		{
			printf("error opening input file %s\n", argv[1]);
			throw std::exception("error");
		}


		if (ctx == NULL)
		{
			printf("spng_ctx_new() failed\n");
			throw std::exception("error");
		}

		/* Ignore and don't calculate chunk CRC's */
		spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

		/* Set memory usage limits for storing standard and unknown chunks,
		   this is important when reading untrusted files! */
		size_t limit = 1024 * 1024 * 64;
		spng_set_chunk_limits(ctx, limit, limit);

		/* Set source PNG */
		spng_set_png_file(ctx, png); /* or _buffer(), _stream() */


		ret = spng_get_ihdr(ctx, &ihdr);

		if (ret)
		{
			printf("spng_get_ihdr() error: %s\n", spng_strerror(ret));
			throw std::exception("error");
		}

		printGeneralInformation(ihdr);


		ret = spng_get_plte(ctx, &plte);

		if (ret && ret != SPNG_ECHUNKAVAIL)
		{
			printf("spng_get_plte() error: %s\n", spng_strerror(ret));
			throw std::exception("error");
		}

		if (!ret)
		{ printf("palette entries: %" PRIu32 "\n", plte.n_entries); }


		/* With SPNG_FMT_PNG indexed color images are output as palette indices,
		   pick another format to expand them. */
		if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
		{ fmt = SPNG_FMT_RGB8; }

		ret = spng_decoded_image_size(ctx, fmt, &out_size);

		if (ret)
		{ throw std::exception("error"); }

		out = static_cast<unsigned char*>(malloc(out_size));
		if (out == NULL)
		{ throw std::exception("error"); }

		/* This is required to initialize for progressive decoding */
		ret = spng_decode_image(ctx, NULL, 0, fmt, SPNG_DECODE_PROGRESSIVE);

		if (ret)
		{
			printf("progressive spng_decode_image() error: %s\n", spng_strerror(ret));
			throw std::exception("error");
		}

		/* ihdr.height will always be non-zero if spng_get_ihdr() succeeds */
		out_width = out_size / ihdr.height;

		spng_row_info row_info = { 0 };

		do
		{
			ret = spng_get_row_info(ctx, &row_info);
			if (ret)
			{ break; }

			ret = spng_decode_row(ctx, out + row_info.row_num * out_width, out_width);
		} while (!ret);

		if (ret != SPNG_EOI)
		{
			printf("progressive decode error: %s\n", spng_strerror(ret));

			if (ihdr.interlace_method)
			{
				printf("last pass: %d, scanline: %" PRIu32 "\n", row_info.pass, row_info.scanline_idx);
			}
			else
			{
				printf("last row: %" PRIu32 "\n", row_info.row_num);
			}
		}

		/* Alternatively you can decode the image in one go,
		   this doesn't require a separate initialization step. */
		/* ret = spng_decode_image(ctx, out, out_size, SPNG_FMT_RGBA8, 0);
		if(ret)
		{
			printf("spng_decode_image() error: %s\n", spng_strerror(ret));
			goto error;
		} */


		uint32_t n_text;

		ret = spng_get_text(ctx, NULL, &n_text);

		if (ret == SPNG_ECHUNKAVAIL)
		{
			ret = 0;
			isNoText = true; /* no text chunks found in file */
			throw std::exception("Error");
		}

		if (ret)
		{
			printf("spng_get_text() error: %s\n", spng_strerror(ret));
			throw std::exception("error");
		}

		text = static_cast<spng_text*>(malloc(n_text * sizeof(spng_text)));

		if (text == NULL)
		{ throw std::exception("error"); }

		ret = spng_get_text(ctx, text, &n_text);

		if (ret)
		{
			printf("spng_get_text() error: %s\n", spng_strerror(ret));
			isNoText = true;
			throw std::exception("Error");
		}

		uint32_t i;
		for (i = 0; i < n_text; i++)
		{
			char* type_str = "tEXt";
			if (text[i].type == SPNG_ITXT)
			{ type_str = "iTXt"; }
			else if (text[i].type == SPNG_ZTXT)
			{ type_str = "zTXt"; }

			printf("\ntext type: %s\n", type_str);
			printf("keyword: %s\n", text[i].keyword);

			if (text[i].type == SPNG_ITXT)
			{
				printf("language tag: %s\n", text[i].language_tag);
				printf("translated keyword: %s\n", text[i].translated_keyword);
			}

			printf("text is %scompressed\n", text[i].compression_flag ? "" : "not ");
			printf("text length: %lu\n", (unsigned long int)text[i].length);
			printf("text: %s\n", text[i].text);
		}

		if (isNoText){
			free(text);

			/* The encoder supports all PNG formats but format conversion support is limited */
			if (fmt != SPNG_FMT_PNG)
			{ throw std::exception("skip encode"); }

			/* This example reencodes the decoded image */



			/* Encode to internal buffer managed by the library */
			spng_set_option(enc, SPNG_ENCODE_TO_BUFFER, 1);

			/* Alternatively you can set an output FILE* or stream with spng_set_png_file() or spng_set_png_stream() */

			/* In this case we're reencoding to the same PNG format */
			spng_set_ihdr(enc, &ihdr);

			/* Copy the palette from the source file */
			if (plte.n_entries > 0)
			{ spng_set_plte(enc, &plte); }

			/* SPNG_FMT_PNG is a special value that matches the format in ihdr */
			fmt = SPNG_FMT_PNG;

			/* SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
			ret = spng_encode_image(enc, out, out_size, fmt, SPNG_ENCODE_FINALIZE);

			if (ret)
			{
				printf("spng_encode_image() error: %s\n", spng_strerror(ret));
				throw std::exception("encode error");
			}

			size_t png_size;
			void* png_buf = NULL;

			/* Get the internal buffer of the finished PNG */
			png_buf = spng_get_png_buffer(enc, &png_size, &ret);

			if (png_buf == NULL)
			{
				printf("spng_get_png_buffer() error: %s\n", spng_strerror(ret));
			}

			/* User owns the buffer after a successful call */
			free(png_buf);
		}

		return ret;
	}

};

int main(int argc, char** argv)
{
	return (new App())->run(argc, argv);
}