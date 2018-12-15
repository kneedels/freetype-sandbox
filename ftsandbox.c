#include <stdio.h>
#include <string.h>
#include "qdbmp.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define MAX_GLYPHS 100

void draw_to_console(FT_Bitmap* bitmap, char printChar); 
void clear_bitmap(BMP* bitmap, UCHAR r, UCHAR g, UCHAR b);
void draw_to_bmp(FT_Bitmap* glyph_bitmap, BMP* dest_bitmap, FT_Int gbmp_left,
        FT_Int gbmp_top);
void compute_string_bbox(FT_BBox *abbox, FT_Glyph* glyphs, FT_UInt num_glyphs,
        FT_Vector* pos);

int main(int argc, char *argv[]){
   
    int output_width = 1000;
    int output_height = 1000;
    int dpi = 144;
    int pointSize = 12;

    char printChar = argc > 2 ? argv[2][0] : '#';
    FT_Library library;
    FT_Face face;
    FT_Bool use_kerning = 1;
    FT_UInt previous = 0;

    // Init FreeType library
    int error = FT_Init_FreeType(&library);
    
    if (error){
        printf("Error initializing the FreeType library: %d\n", error);
        return error;
    } 

    // Load the font file
    error = FT_New_Face(library, "/usr/share/fonts/dejavu/DejaVuSans.ttf", 
            0, &face);

    if (error == FT_Err_Unknown_File_Format) {
        printf("Unknown font file format.\n");
        return error;
    } else if (error){
        printf("Error loading the font file: %d\n", error);
        return error;
    }

    // Set the desired point size and DPI
    error = FT_Set_Char_Size(
            face,
            0,
            pointSize*64, 
            dpi,
            dpi);

    if (error){
        printf("Error setting char size: %d\n", error);
        return error;
    }

    char* text = argv[1];
    int num_chars = strlen(argv[1]);
    int pen_x, pen_y, n;
    FT_GlyphSlot slot = face->glyph;

    FT_Glyph glyphs[MAX_GLYPHS];
    FT_Vector pos[MAX_GLYPHS];
    FT_UInt num_glyphs = 0;

    pen_x = 0;
    pen_y = 0;

    // Use kerning if requested and available
    use_kerning = use_kerning && FT_HAS_KERNING(face);
  
    // Create the output bitmap and clear it with white 
    BMP* bmp;
    bmp = BMP_Create(output_width, output_height, 24);
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

        pos[num_glyphs].x = pen_x;
        pos[num_glyphs].y = pen_y;

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error){
            printf("Glyph load error! %d\n", error);
            continue;
        }

        error = FT_Get_Glyph(face->glyph, &glyphs[num_glyphs]);
        if (error){
            printf("Get glyph error! %d\n", error);
            continue;
        }

        /*
         * MOVE THIS:
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
        */
        pen_x += slot->advance.x >> 6;
        pen_y += slot->advance.y >> 6;
        previous = glyph_index;
        num_glyphs++;
    }

    FT_BBox string_bbox;
    compute_string_bbox(&string_bbox, glyphs, num_glyphs, pos);

    FT_Pos string_width = string_bbox.xMax - string_bbox.xMin;
    FT_Pos string_height = string_bbox.yMax - string_bbox.yMin;

    FT_Pos start_x = ((output_width - string_width) / 2) * 64;
    FT_Pos start_y = ((output_height - string_height) / 2) * 64;

    for (n = 0; n < num_glyphs; n++){
        FT_Glyph image;
        FT_Vector pen;

        image = glyphs[n];

        pen.x = start_x + pos[n].x * 64;
        pen.y = start_y + pos[n].y * 64;

        error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL, &pen, 0);

        if (!error){
            FT_BitmapGlyph bit = (FT_BitmapGlyph)image;

            draw_to_bmp(&bit->bitmap, bmp, bit->left, output_height - bit->top);

            FT_Done_Glyph(image);

        }
    }

    BMP_WriteFile(bmp, "out.bmp");
    BMP_Free(bmp);
    printf("Success!\n");
    return 0;
}

void compute_string_bbox(FT_BBox *abbox, FT_Glyph* glyphs, FT_UInt num_glyphs,
        FT_Vector* pos){
    FT_BBox bbox;
    FT_BBox glyph_bbox;
    int n;

    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;

    for (n = 0; n < num_glyphs; n++){
        FT_Glyph_Get_CBox(glyphs[n], ft_glyph_bbox_pixels, &glyph_bbox);

        glyph_bbox.xMin += pos[n].x;
        glyph_bbox.xMax += pos[n].x;
        glyph_bbox.yMin += pos[n].y;
        glyph_bbox.yMax += pos[n].y;

        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;

        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;
        
        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;

        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }

    if (bbox.xMin > bbox.xMax){
        bbox.xMin = 0;
        bbox.yMin = 0;
        bbox.xMax = 0;
        bbox.yMax = 0;
    }

    *abbox = bbox;
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
