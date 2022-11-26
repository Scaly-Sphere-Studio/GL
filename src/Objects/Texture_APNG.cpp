/*
 * load6apng.cpp
 *
 * loads APNG file, saves all frames as PNG (32bpp).
 * including frames composition.
 *
 * needs regular, unpatched libpng.
 *
 * Copyright (c) 2014 Max Stepin
 * maxst at users.sourceforge.net
 *
 * zlib license
 * ------------
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#include <png.h>     /* original (unpatched) libpng is ok */

#include "SSS/GL/Objects/Texture.hpp"

#define notabc(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

#define id_IHDR 0x52444849
#define id_acTL 0x4C546361
#define id_fcTL 0x4C546366
#define id_IDAT 0x54414449
#define id_fdAT 0x54416466
#define id_IEND 0x444E4549

SSS_GL_BEGIN;
INTERNAL_BEGIN;

struct CHUNK {
    std::vector<uint8_t> vec;
    unsigned int size;
};

static void info_fn(png_structp png_ptr, png_infop info_ptr)
{
    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
    png_set_gray_to_rgb(png_ptr);
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
    (void)png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
}

static void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
    APNGFrame* frame = (APNGFrame*)png_get_progressive_ptr(png_ptr);
    png_progressive_combine_row(png_ptr, frame->rows[row_num], new_row);
}

static void compose_frame(std::vector<uint8_t*>& rows_dst, std::vector<uint8_t*> const& rows_src,
    uint8_t bop, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    for (size_t i = 0; i < h; i++)
    {
        uint8_t* dp = rows_dst[i + y] + x * 4;
        uint8_t const* sp = rows_src[i];

        if (bop == 0)
            std::memcpy(dp, sp, w * 4);
        else
            for (size_t i = 0; i < w; i++, sp += 4, dp += 4)
            {
                if (sp[3] == 255)
                    std::memcpy(dp, sp, 4);
                else
                    if (sp[3] != 0)
                    {
                        if (dp[3] != 0)
                        {
                            int const u = sp[3] * 255;
                            int const v = (255 - sp[3]) * dp[3];
                            int const al = u + v;
                            dp[0] = (sp[0] * u + dp[0] * v) / al;
                            dp[1] = (sp[1] * u + dp[1] * v) / al;
                            dp[2] = (sp[2] * u + dp[2] * v) / al;
                            dp[3] = al / 255;
                        }
                        else
                            std::memcpy(dp, sp, 4);
                    }
            }
    }
}

static unsigned int read_chunk(FILE* f, CHUNK* pChunk)
{
    uint8_t len[4];
    if (fread(&len, 4, 1, f) == 1)
    {
        pChunk->size = png_get_uint_32(len) + 12;
        pChunk->vec.resize(pChunk->size);
        std::memcpy(pChunk->vec.data(), len, 4);
        if (fread(pChunk->vec.data() + 4, pChunk->size - 4, 1, f) == 1)
            return *(unsigned int*)(pChunk->vec.data() + 4);
    }
    return 0;
}

static void processing_start(png_structp& png_ptr, png_infop& info_ptr, void* frame_ptr,
    bool hasInfo, CHUNK& chunkIHDR, std::vector<CHUNK>& chunksInfo)
{
    uint8_t header[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    if (!png_ptr || !info_ptr)
        return;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return;
    }

    png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
    png_set_progressive_read_fn(png_ptr, frame_ptr, info_fn, row_fn, NULL);

    png_process_data(png_ptr, info_ptr, header, 8);
    png_process_data(png_ptr, info_ptr, chunkIHDR.vec.data(), chunkIHDR.size);

    if (hasInfo)
        for (unsigned int i = 0; i < chunksInfo.size(); i++)
            png_process_data(png_ptr, info_ptr, chunksInfo[i].vec.data(), chunksInfo[i].size);
}

static void processing_data(png_structp png_ptr, png_infop info_ptr, uint8_t* p, unsigned int size)
{
    if (!png_ptr || !info_ptr)
        return;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return;
    }

    png_process_data(png_ptr, info_ptr, p, size);
}

static int processing_finish(png_structp png_ptr, png_infop info_ptr)
{
    uint8_t footer[12] = { 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130 };

    if (!png_ptr || !info_ptr)
        return 1;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return 1;
    }

    png_process_data(png_ptr, info_ptr, footer, 12);
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);

    return 0;
}

int load_apng(char const* filepath, std::vector<APNGFrame>& frames)
{
    FILE* f;
    unsigned int id, w, h, w0, h0, x0, y0;
    unsigned int delay_num, delay_den, dop, bop, rowbytes, imagesize;
    uint8_t sig[8];
    png_structp png_ptr;
    png_infop info_ptr;
    CHUNK chunk;
    CHUNK chunkIHDR;
    std::vector<CHUNK> chunksInfo;
    bool isAnimated = false;
    bool skipFirst = false;
    bool hasInfo = false;
    APNGFrame frameRaw;
    APNGFrame frameCur;
    APNGFrame frameNext;
    int res = -1;

    errno_t err = fopen_s(&f, filepath, "rb");
    if (err != 0) {
        throw_exc(CONTEXT_MSG(getErrorString(err), filepath));
    }
    if (fread_s(sig, 8, 1, 8, f) == 8 && png_sig_cmp(sig, 0, 8) == 0)
    {
        id = read_chunk(f, &chunkIHDR);

        if (id == id_IHDR && chunkIHDR.size == 25)
        {
            w0 = w = png_get_uint_32(chunkIHDR.vec.data() + 8);
            h0 = h = png_get_uint_32(chunkIHDR.vec.data() + 12);
            x0 = 0;
            y0 = 0;
            delay_num = 1;
            delay_den = 10;
            dop = 0;
            bop = 0;
            rowbytes = w * 4;
            imagesize = h * rowbytes;

            frameRaw.vec.resize(imagesize);
            frameRaw.rows.resize(h * sizeof(png_bytep));
            for (size_t i = 0; i < h; i++)
                frameRaw.rows[i] = frameRaw.vec.data() + i * rowbytes;

            frameCur.w = w;
            frameCur.h = h;
            frameCur.vec.resize(imagesize);
            frameCur.rows.resize(h * sizeof(png_bytep));
            for (size_t i = 0; i < h; i++)
                frameCur.rows[i] = frameCur.vec.data() + i * rowbytes;

            processing_start(png_ptr, info_ptr, (void*)&frameRaw, hasInfo, chunkIHDR, chunksInfo);

            while (!feof(f))
            {
                id = read_chunk(f, &chunk);

                if (id == id_acTL && !hasInfo && !isAnimated)
                {
                    isAnimated = true;
                    skipFirst = true;
                }
                else
                    if (id == id_fcTL && (!hasInfo || isAnimated))
                    {
                        if (hasInfo)
                        {
                            if (!processing_finish(png_ptr, info_ptr))
                            {
                                frameNext.vec.resize(imagesize);
                                frameNext.rows.resize(h * sizeof(png_bytep));
                                for (size_t i = 0; i < h; i++)
                                    frameNext.rows[i] = frameNext.vec.data() + i * rowbytes;

                                if (dop == 2)
                                    frameNext.vec = frameCur.vec;

                                compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
                                frameCur.delay_num = delay_num;
                                frameCur.delay_den = delay_den;

                                frames.push_back(frameCur);

                                if (dop != 2)
                                {
                                    frameNext.vec = frameCur.vec;
                                    if (dop == 1)
                                        for (size_t i = 0; i < h0; i++)
                                            std::memset(frameNext.rows[y0 + i] + x0 * 4, 0, w0 * 4);
                                }
                                frameCur.vec = std::move(frameNext.vec);
                                frameCur.rows = std::move(frameNext.rows);
                            }
                            else
                                break;
                        }

                        // At this point the old frame is done. Let's start a new one.
                        w0 = png_get_uint_32(chunk.vec.data() + 12);
                        h0 = png_get_uint_32(chunk.vec.data() + 16);
                        x0 = png_get_uint_32(chunk.vec.data() + 20);
                        y0 = png_get_uint_32(chunk.vec.data() + 24);
                        delay_num = png_get_uint_16(chunk.vec.data() + 28);
                        delay_den = png_get_uint_16(chunk.vec.data() + 30);
                        dop = chunk.vec[32];
                        bop = chunk.vec[33];

                        if (hasInfo)
                        {
                            std::memcpy(chunkIHDR.vec.data() + 8, chunk.vec.data() + 12, 8);
                            processing_start(png_ptr, info_ptr, (void*)&frameRaw, hasInfo, chunkIHDR, chunksInfo);
                        }
                        else
                            skipFirst = false;

                        if (frames.size() == (skipFirst ? 1 : 0))
                        {
                            bop = 0;
                            if (dop == 2)
                                dop = 1;
                        }
                    }
                    else
                        if (id == id_IDAT)
                        {
                            hasInfo = true;
                            processing_data(png_ptr, info_ptr, chunk.vec.data(), chunk.size);
                        }
                        else
                            if (id == id_fdAT && isAnimated)
                            {
                                png_save_uint_32(chunk.vec.data() + 4, chunk.size - 16);
                                std::memcpy(chunk.vec.data() + 8, "IDAT", 4);
                                processing_data(png_ptr, info_ptr, chunk.vec.data() + 4, chunk.size - 4);
                            }
                            else
                                if (id == id_IEND)
                                {
                                    if (hasInfo && !processing_finish(png_ptr, info_ptr))
                                    {
                                        compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
                                        frameCur.delay_num = delay_num;
                                        frameCur.delay_den = delay_den;
                                        frames.push_back(frameCur);
                                    }
                                    break;
                                }
                                else
                                    if (notabc(chunk.vec[4]) || notabc(chunk.vec[5]) || notabc(chunk.vec[6]) || notabc(chunk.vec[7]))
                                        break;
                                    else
                                        if (!hasInfo)
                                        {
                                            processing_data(png_ptr, info_ptr, chunk.vec.data(), chunk.size);
                                            chunksInfo.push_back(chunk);
                                            continue;
                                        }
            }

            if (!frames.empty())
                res = (skipFirst) ? 0 : 1;
        }
    }
    fclose(f);

    return res;
}

SSS_GL_END;
INTERNAL_END;