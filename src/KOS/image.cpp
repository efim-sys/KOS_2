#include "KOS/KOS.h"
#include "bmp_headers.h"

namespace KOS {
    uint16_t *rotateImageRight(uint16_t *from, int w, int h)
    {
        uint16_t *addr = (uint16_t *)ps_malloc(w * h * 2);
        for (uint32_t i = 0; i < w * h; i++)
        {
            addr[i] = from[w * h - w - w * (i % h) + i / w];
        }
        return addr;
    }

    void flipImageV(uint16_t *img, int w, int h)
    {
        for (uint32_t x = 0; x < w; x++)
        {
            for (uint32_t y = 0; y < h >> 1; y++)
            {
                std::swap(img[x + y * w], img[x + (h - y - 1) * w]);
            }
        }
    }

    void flipImageV(uint16_t *img, uint16_t **destination, int w, int h)
    {
        *destination = (uint16_t *)ps_malloc(w * h * 2);
        memcpy(destination, img, w * h * 2);
        flipImageV(*destination, w, h);
    }

    void flipImageH(uint16_t *img, int w, int h)
    {
        for (uint32_t y = 0; y < h; y++)
        {
            for (uint32_t x = 0; x < w >> 1; x++)
            {
                std::swap(img[x + y * w], img[(w - 1 - x) + y * w]);
            }
        }
    }

    void flipImageH(uint16_t *img, uint16_t **destination, int w, int h)
    {
        USBSerial.printf("Allocating %d bytes\n", w * h * 2);
        *destination = (uint16_t *)ps_malloc(w * h * 2);
        memcpy(*destination, img, w * h * 2);
        flipImageH(*destination, w, h);
    }

    uint16_t *readImageBmp(fs::FS &fs, const char *filename, uint32_t *width, uint32_t *height)
    {
        uint16_t *addr;
        char header[0x80];

        File image = fs.open(filename);
        image.readBytes(header, 0xB);
        USBSerial.printf("Data offset = %d\n", header[0xA]);
        image.readBytes(header + 0xB, header[0xA] - 0xA);
        USBSerial.printf("Data offset = %d\n", header[0xA]);

        uint8_t bpp = header[0x1C];

        USBSerial.printf("BPP=%d\n", bpp);

        std::swap(header[15], header[18]);
        std::swap(header[16], header[17]);
        std::swap(header[19], header[22]);
        std::swap(header[20], header[21]);

        uint32_t w = *(uint32_t *)(header + 15);
        uint32_t h = *(uint32_t *)(header + 15 + 4);

        USBSerial.printf("Filename=%s; w=%d, h=%d\n", filename, w, h);

        addr = (uint16_t *)ps_malloc(w * h * (bpp / 8));

        USBSerial.printf("Allocated %d pixels\n", w * h);

        image.readBytes((char *)addr, w * h * (bpp / 8));

        USBSerial.print("Read OK!\n");

        image.close();

        if (bpp == 24)
        {
            USBSerial.println("BPP=24 -> correcting bpp...");
            uint32_t totalPixels = w * h;
            lgfx::rgb888_t *a24 = (lgfx::rgb888_t *)addr;
            lgfx::rgb565_t *a16 = (lgfx::rgb565_t *)addr;

            for (uint32_t i = 0; i < totalPixels; i++)
            {
                a16[i] = a24[i];
            }
        }
        USBSerial.println("Flipping image now");
        
        flipImageV(addr, w, h);

        if (width != NULL)
            *width = w;
        if (height != NULL)
            *height = h;

        return addr;
    }

    void readImageBmp(fs::FS &fs, const char *filename, uint32_t *width, uint32_t *height, uint16_t *addr)
    {
        // uint64_t tmr = micros();
        File image = fs.open(filename);
        char header[0x80];

        // USBSerial.printf("File opening time=%d\n", micros()-tmr);
        image.readBytes(header, 0x45);

        std::swap(header[15], header[18]);
        std::swap(header[16], header[17]);
        std::swap(header[19], header[22]);
        std::swap(header[20], header[21]);

        uint32_t w = *(uint32_t *)(header + 15);
        uint32_t h = *(uint32_t *)(header + 15 + 4);

        uint8_t bpp = header[0x1C];

        image.readBytes((char *)addr, w * h * (bpp / 8));

        if (bpp == 24)
        {
            uint32_t totalPixels = w * h;
            lgfx::rgb888_t *a24 = (lgfx::rgb888_t *)addr;
            lgfx::rgb565_t *a16 = (lgfx::rgb565_t *)addr;

            for (uint32_t i = 0; i < totalPixels; i++)
            {
                a16[i] = a24[i];
                addr[i] = ntohs(addr[i]);
            }
        }

        image.close();

        flipImageV(addr, w, h);

        *width = w;
        *height = h;
    }
    
    int saveFramebuffer(uint16_t *framebuffer, uint32_t w, uint32_t h, const char* filename) {
        // В файле строки должны быть выровнены по 4 байта (padding)
        int row_size = (w * sizeof(uint16_t) + 0b11) & ~0b11;

        const uint32_t masks[3] = {0xF800, 0x07E0, 0x001F};

        BMP_file_header_t file_hdr = {
            .bfType = 0x4d42, // ASCII "BM"
            .bfSize = row_size*h*sizeof(uint16_t) + sizeof(BMP_file_header_t) + sizeof(BMP_info_header_t) + 12, 
                // bitmap size + header size + size of masks
            .bfReserved1 = 0,
            .bfReserved2 = 0,
            .bfOffBits = sizeof(BMP_file_header_t) + sizeof(BMP_info_header_t) + 12 // Header size
        };

        BMP_info_header_t info_hdr = {
            .biSize = 40,   // Size of this header
            .biWidth = w,
            .biHeight = h,
            .biPlanes = 1,      // must be 1
            .biBitCount = 16,   // 16 bpp
            .biCompression = 3, // BI_BITFIELDS compression
            .biSizeImage = row_size*h*sizeof(uint16_t),
            .biXPelsPerMeter = 0,   // pixels per meter (don't use)
            .biYPelsPerMeter = 0,   // pixels per meter (don't use)
            .biClrUsed = 0,     // For color tables (don't use)
            .biClrImportant = 0 // For color tables (don't use)
        };

        USBSerial.printf("row_size = %d\n", row_size);
        USBSerial.printf(".bfsize = %d\n", file_hdr.bfSize);
        USBSerial.printf("sizeof(file_hdr) = %d\n", sizeof(file_hdr));
        USBSerial.printf("sizeof(info_hdr) = %d\n", sizeof(info_hdr));

        FILE* bmp_file = fopen(filename, "wb");

        if(!bmp_file) {
            return -USBSerial.printf("Cannot open file [%s] for saving image\n", filename);
        }

        

        fwrite(&file_hdr, sizeof(file_hdr), 1, bmp_file);
        fwrite(&info_hdr, sizeof(info_hdr), 1, bmp_file);
        fwrite(masks, sizeof(uint32_t), 3, bmp_file);

        // fputc(0, bmp_file);
        // fputc(0, bmp_file);
        // fputc(0, bmp_file);

        uint16_t* row_buffer = (uint16_t*) calloc(row_size, 1);

        for(int32_t y = h-1; y >= 0; y--) {
            for(int32_t x = 0; x < w; x++) {
                row_buffer[x] = ntohs(framebuffer[y * w + x]);
                
            }

            fwrite(row_buffer, row_size, 1, bmp_file);
        }


        free(row_buffer);
        fclose(bmp_file);

        return 0;
    }
}