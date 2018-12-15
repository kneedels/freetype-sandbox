#include <stdio.h>
#include <string.h>
#include "qdbmp.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

void draw_to_console(FT_Bitmap* bitmap, char printChar); 
void clear_bitmap(BMP* bitmap, UCHAR r, UCHAR g, UCHAR b);
void draw_to_bmp(FT_Bitmap* glyph_bitmap, BMP* dest_bitmap, FT_Int gbmp_left,
        FT_Int gbmp_top);


int main(int argc, char *argv[]){
    
    char printChar = argc > 2 ? argv[2][0] : '#';
    FT_Library library;
    FT_Face face;
    FT_Bool use_kerning = 1;
    FT_UInt previous;

    int error = FT_Init_FreeType(&library);
    
    if (error){
        printf("There was an error: %d\n", error);
    } else {
        printf("Successfully loaded library.\n");
    }

    error = FT_New_Face(library, "/usr/share/fonts/dejavu/DejaVuSans.ttf", 0, &face);

    if (error == FT_Err_Unknown_File_Format) {
        printf("Unknown file format.\n");
    } else if (error){
        printf("There was an error: %d\n", error);
    } else {
        printf("Successfully loaded font file.\n");
    }

    printf("Num glyphs: %ld\n", face->num_glyphs);
    printf("Units per EM:: %u\n", face->units_per_EM);
    printf("Fixed strikes: %d\n", face->num_fixed_sizes);

    error = FT_Set_Char_Size(
            face,
            0,
            16*64, 
            300,
            300);

    if (error){
        printf("There was an error: %d\n", error);
    } else {
        printf("Successfully set char size..\n");
    }

    char* text = argv[1];
    int num_chars = strlen(argv[1]);
    int pen_x, pen_y, n;
    FT_GlyphSlot slot = face->glyph;

    pen_x = 300;
    pen_y = 300;

    use_kerning = use_kerning && FT_HAS_KERNING(face);
   
    BMP* bmp;
    bmp = BMP_Create(1000, 1000, 24);
    clear_bitmap(bmp, 255, 255, 255);

    for (n = 0; n < num_chars; n++){
        FT_UInt glyph_index;

        glyph_index = FT_Get_Char_Index(face, text[n]);
        //glyph_index=49;

        if (use_kerning && previous && glyph_index){
            FT_Vector delta;
            FT_Get_Kerning(face, previous, glyph_index, 
                    FT_KERNING_DEFAULT, &delta);
            pen_x += delta.x >> 6;
        }

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error){
            printf("Glyph load error! %d\n", error);
            continue;
        }

        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (error){
            printf("Glyph render error! %d\n", error);
            continue;
        }


        //draw_to_console(&slot->bitmap, printChar);
        draw_to_bmp(&slot->bitmap,
                bmp,
                pen_x + slot->bitmap_left,
                pen_y - slot->bitmap_top);

        pen_x += slot->advance.x >> 6;
        pen_y += slot->advance.y >> 6;
        previous = glyph_index;
    }

    BMP_WriteFile(bmp, "out.bmp");
    BMP_Free(bmp);
    printf("Done!\n");
    return 0;
}

void clear_bitmap(BMP* bitmap, UCHAR r, UCHAR g, UCHAR b) {
    UINT height = BMP_GetHeight(bitmap);
    UINT width = BMP_GetWidth(bitmap);
    int x, y;

    for (y = 0; y < height; y++){
        for (x = 0; x < width; x++){
            BMP_SetPixelRGB(bitmap, x, y, r, g, b);
        }
    }
}

void draw_to_bmp(FT_Bitmap* glyph_bitmap, BMP* dest_bitmap, FT_Int gbmp_left,
        FT_Int gbmp_top) {

    int row, col;

    for (row = 0; row < glyph_bitmap->rows; row++){
        for (col = 0; col < glyph_bitmap->width; col++) {
            // Needs gamma correction and proper alpha blending
            int v = 255 - glyph_bitmap->buffer[row * glyph_bitmap->width + col];
            if (v == 255) continue;
            BMP_SetPixelRGB(
                    dest_bitmap,
                    gbmp_left + col,
                    gbmp_top + row,
                    v, v, v);

        }
    }
}

void draw_to_console(FT_Bitmap* bitmap, char printChar) {
    printf("Dimensions: %d x %d\n", bitmap->rows, bitmap->width);

    int row, col;

    for (row = 0; row < bitmap->rows; row++){
        for (col = 0; col < bitmap->width; col++) {
            char toPrint = ' ';
            if (bitmap->buffer[row*bitmap->width + col] > 0){
               toPrint = printChar;
            } 
            printf("%c", toPrint);
        }
        printf("\n");
    }
}
