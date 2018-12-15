#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

void my_draw_bitmap(FT_Bitmap* bitmap, FT_Int bitmap_left, 
        FT_Int bitmap_top);

int main(){
    
    FT_Library library;
    FT_Face face;

    int error = FT_Init_FreeType(&library);
    
    if (error){
        printf("There was an error: %d\n", error);
    } else {
        printf("Successfully loaded library.\n");
    }

    error = FT_New_Face(library, "/Library/Fonts/Wingdings.ttf", 0, &face);

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
            1200,
            400);

    if (error){
        printf("There was an error: %d\n", error);
    } else {
        printf("Successfully set char size..\n");
    }

    char* text = "a";
    int num_chars = 1;
    int pen_x, pen_y, n;
    FT_GlyphSlot slot = face->glyph;

    pen_x = 300;
    pen_y = 300;
    
    for (n = 0; n < num_chars; n++){
        FT_UInt glyph_index;

        glyph_index = FT_Get_Char_Index(face, text[n]);
        glyph_index=49;
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error){
            printf("Glyph load error! %d\n", error);
            continue;
        }

        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (error){
            printf("Glyph render error! %d\n", error);
        }

        //printf("bitmap left: %d bitmap top: %d", slot->bitmap_left,
        //        slot->bitmap_top);

        my_draw_bitmap(&slot->bitmap,
                pen_x + slot->bitmap_left,
                pen_y + slot->bitmap_top);

        pen_x += slot->advance.x >> 6;
        pen_y += slot->advance.y >> 6;
    }

    printf("Done!\n");
    return 0;
}
    
void my_draw_bitmap(FT_Bitmap* bitmap, FT_Int bitmap_left, 
        FT_Int bitmap_top) {
    printf("Dimensions: %d x %d\n", bitmap->rows, bitmap->width);

    int row, col;

    for (row = 0; row < bitmap->rows; row++){
        for (col = 0; col < bitmap->width; col++) {
            char toPrint = '.';
            if (bitmap->buffer[row*bitmap->width + col] > 0){
               toPrint = '#';
            } 
            printf("%c", toPrint);
        }
        printf("\n");
    }
}
