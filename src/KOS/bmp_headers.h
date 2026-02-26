#include <stdint.h>

// Compiler-specific packing directives (GCC example)
#pragma pack(push, 1)

typedef struct {
    uint16_t bfType;       /* Magic identifier: 0x4D42 ('BM') */
    uint32_t bfSize;       /* Size of file in bytes */
    uint16_t bfReserved1;  /* Reserved; must be 0 */
    uint16_t bfReserved2;  /* Reserved; must be 0 */
    uint32_t bfOffBits;    /* Offset to image data in bytes from beginning of file (usually 54 bytes) */
} BMP_file_header_t;        /* 14 bytes total */

typedef struct {
    uint32_t biSize;           /* Size of this header in bytes (40 bytes for BITMAPINFOHEADER) */
    int32_t  biWidth;          /* Width of image in pixels */
    int32_t  biHeight;         /* Height of image in pixels (positive for bottom-up, negative for top-down) */
    uint16_t biPlanes;         /* Number of color planes (must be 1) */
    uint16_t biBitCount;       /* Bits per pixel (e.g., 24 for 24-bit RGB) */
    uint32_t biCompression;    /* Compression type (0 for uncompressed RGB) */
    uint32_t biSizeImage;      /* Image size in bytes (can be 0 for uncompressed images) */
    int32_t  biXPelsPerMeter;  /* Horizontal resolution in pixels per meter */
    int32_t  biYPelsPerMeter;  /* Vertical resolution in pixels per meter */
    uint32_t biClrUsed;        /* Number of colors used in the color table (0 means max colors for bpp) */
    uint32_t biClrImportant;   /* Number of important colors (0 means all colors are important) */
} BMP_info_header_t;        /* 40 bytes total */

// Restore default packing
#pragma pack(pop)
