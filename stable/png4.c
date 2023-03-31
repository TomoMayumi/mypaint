#include<png.h>
#include<zlib.h>

void write_png(char *file_name, unsigned char **image, int width, int height)
{
  FILE*fp;
  png_structp png_ptr;
  png_infop info_ptr;
  void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass);

  if ((fp = fopen(file_name, "wb")) == NULL) return;
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fclose(fp);
    return;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    fclose(fp);
    return;
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr,  &info_ptr);
    fclose(fp);
    return;
  }
  png_init_io(png_ptr, fp);
  png_set_write_status_fn(png_ptr, write_row_callback);
  png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_gAMA(png_ptr, info_ptr, 1.0);

  {
    time_t gmt;// G.M.T.
    png_time mod_time;
    png_text text_ptr[5];

    time(&gmt);
    png_convert_from_time_t(&mod_time, gmt);
    png_set_tIME(png_ptr, info_ptr, &mod_time);
    
    text_ptr[0].key = "Creation Time";
    text_ptr[0].text = png_convert_to_rfc1123(png_ptr, &mod_time);
    text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[1].key = "Software";
    text_ptr[1].text = "Paintb.exe";
    text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text(png_ptr, info_ptr, text_ptr, 2);
  }
  
  png_write_info(png_ptr, info_ptr);

  {
    int k; 
    png_bytep *row_pointers;
    row_pointers = (png_bytep *)malloc(sizeof(png_bytep *) * height);
    for (k = 0; k < height; k++)
      row_pointers[k] = image[k];
    
    png_write_image(png_ptr, row_pointers);
    free(row_pointers);
    png_write_end(png_ptr, info_ptr);


  }
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fflush(fp);
  fclose(fp);
  return;
}

void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
  //printf("\r%3d%% saved", (row * 100) / png_ptr->height);
  //if(((row * 100) / png_ptr->height)==100)printf("\n");
}
