

/* This function is called when there is a warning, but the library thinks
 * it can continue anyway.  Replacement functions don't have to do anything
 * here if you don't want to.  In the default configuration, png_ptr is
 * not used, but it is passed in case it may be useful.
 */
static void
pngtest_warning(png_structp png_ptr, png_const_charp message)
{
   PNG_CONST char *name = "UNKNOWN (ERROR!)";
   if (png_ptr != NULL && png_ptr->error_ptr != NULL)
      name = png_ptr->error_ptr;
   fprintf(STDERR, "%s: libpng warning: %s\n", name, message);
}

/* This is the default error handling function.  Note that replacements for
 * this function MUST NOT RETURN, or the program will likely crash.  This
 * function is used by default, or if the program supplies NULL for the
 * error function pointer in png_set_error_fn().
 */
static void
pngtest_error(png_structp png_ptr, png_const_charp message)
{
   pngtest_warning(png_ptr, message);
   /* We can return because png_error calls the default handler, which is
    * actually OK in this case.
    */
}


int
test_one_file(PNG_CONST char *inname, PNG_CONST char *outname)
{
   static png_FILE_p fpin;
   static png_FILE_p fpout;  /* "static" prevents setjmp corruption */
   png_structp read_ptr;
   png_infop read_info_ptr, end_info_ptr;
   png_structp write_ptr = NULL;
   png_infop write_info_ptr = NULL;
   png_infop write_end_info_ptr = NULL;
   png_bytep row_buf;
   png_uint_32 y;
   png_uint_32 width, height;
   int num_pass, pass;
   int bit_depth, color_type;
#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   jmp_buf jmpbuf;
#endif
#endif

#if defined(_WIN32_WCE)
   TCHAR path[MAX_PATH];
#endif
   char inbuf[256], outbuf[256];

   row_buf = NULL;

#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, inname, -1, path, MAX_PATH);
   if ((fpin = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpin = fopen(inname, "rb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not find input file %s\n", inname);
      return (1);
   }

#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, outname, -1, path, MAX_PATH);
   if ((fpout = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpout = fopen(outname, "wb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not open output file %s\n", outname);
      FCLOSE(fpin);
      return (1);
   }


   read_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL,
      png_error_ptr_NULL, png_error_ptr_NULL);

      
   png_set_error_fn(read_ptr, (png_voidp)inname, pngtest_error,
       pngtest_warning);

   read_info_ptr = png_create_info_struct(read_ptr);
   end_info_ptr = png_create_info_struct(read_ptr);

   
#ifdef PNG_SETJMP_SUPPORTED
   png_debug(0, "Setting jmpbuf for read struct");
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(png_jmpbuf(read_ptr)))
#endif
   {
      FPRINTF(STDERR, "%s -> %s: libpng read error\n", inname, outname);
      png_free(read_ptr, row_buf);
      row_buf = NULL;
      png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
      FCLOSE(fpin);
      FCLOSE(fpout);
      return (1);
   }
#ifdef USE_FAR_KEYWORD
   png_memcpy(png_jmpbuf(read_ptr), jmpbuf, png_sizeof(jmp_buf));
#endif


   png_debug(0, "Initializing input and output streams");
#if !defined(PNG_NO_STDIO)
   png_init_io(read_ptr, fpin);
#else
   png_set_read_fn(read_ptr, (png_voidp)fpin, pngtest_read_data);
#endif
   if (status_dots_requested == 1)
   {
      png_set_read_status_fn(read_ptr, read_row_callback);
   }
   else
   {
      png_set_read_status_fn(read_ptr, png_read_status_ptr_NULL);
   }



   png_debug(0, "Reading info struct");
   png_read_info(read_ptr, read_info_ptr);

   png_debug(0, "Transferring info struct");
   {
      int interlace_type, compression_type, filter_type;

      if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,
          &color_type, &interlace_type, &compression_type, &filter_type))
      {
         png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,
            color_type, PNG_INTERLACE_NONE, compression_type, filter_type);
      }
   }
#if defined(PNG_FIXED_POINT_SUPPORTED)
#if defined(PNG_cHRM_SUPPORTED)
   {
      png_fixed_point white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
         blue_y;
      if (png_get_cHRM_fixed(read_ptr, read_info_ptr, &white_x, &white_y, &red_x,
         &red_y, &green_x, &green_y, &blue_x, &blue_y))
      {
         png_set_cHRM_fixed(write_ptr, write_info_ptr, white_x, white_y, red_x,
            red_y, green_x, green_y, blue_x, blue_y);
      }
   }
#endif
#if defined(PNG_gAMA_SUPPORTED)
   {
      png_fixed_point gamma;

      if (png_get_gAMA_fixed(read_ptr, read_info_ptr, &gamma))
         png_set_gAMA_fixed(write_ptr, write_info_ptr, gamma);
   }
#endif
#else /* Use floating point versions */
#if defined(PNG_FLOATING_POINT_SUPPORTED)
#if defined(PNG_cHRM_SUPPORTED)
   {
      double white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
         blue_y;
      if (png_get_cHRM(read_ptr, read_info_ptr, &white_x, &white_y, &red_x,
         &red_y, &green_x, &green_y, &blue_x, &blue_y))
      {
         png_set_cHRM(write_ptr, write_info_ptr, white_x, white_y, red_x,
            red_y, green_x, green_y, blue_x, blue_y);
      }
   }
#endif
#if defined(PNG_gAMA_SUPPORTED)
   {
      double gamma;

      if (png_get_gAMA(read_ptr, read_info_ptr, &gamma))
         png_set_gAMA(write_ptr, write_info_ptr, gamma);
   }
#endif
#endif /* Floating point */
#endif /* Fixed point */
#if defined(PNG_iCCP_SUPPORTED)
   {
      png_charp name;
      png_charp profile;
      png_uint_32 proflen;
      int compression_type;

      if (png_get_iCCP(read_ptr, read_info_ptr, &name, &compression_type,
                      &profile, &proflen))
      {
         png_set_iCCP(write_ptr, write_info_ptr, name, compression_type,
                      profile, proflen);
      }
   }
#endif

//#if defined(PNG_sRGB_SUPPORTED)
   {
      int intent;

      if (png_get_sRGB(read_ptr, read_info_ptr, &intent))
         png_set_sRGB(write_ptr, write_info_ptr, intent);
   }
//#endif
   {
      png_colorp palette;
      int num_palette;

      if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette))
         png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);
   }
#if defined(PNG_bKGD_SUPPORTED)
   {
      png_color_16p background;

      if (png_get_bKGD(read_ptr, read_info_ptr, &background))
      {
         png_set_bKGD(write_ptr, write_info_ptr, background);
      }
   }
#endif
#if defined(PNG_hIST_SUPPORTED)
   {
      png_uint_16p hist;

      if (png_get_hIST(read_ptr, read_info_ptr, &hist))
         png_set_hIST(write_ptr, write_info_ptr, hist);
   }
#endif
#if defined(PNG_oFFs_SUPPORTED)
   {
      png_int_32 offset_x, offset_y;
      int unit_type;

      if (png_get_oFFs(read_ptr, read_info_ptr, &offset_x, &offset_y,
          &unit_type))
      {
         png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);
      }
   }
#endif
#if defined(PNG_pCAL_SUPPORTED)
   {
      png_charp purpose, units;
      png_charpp params;
      png_int_32 X0, X1;
      int type, nparams;

      if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,
         &nparams, &units, &params))
      {
         png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,
            nparams, units, params);
      }
   }
#endif
#if defined(PNG_pHYs_SUPPORTED)
   {
      png_uint_32 res_x, res_y;
      int unit_type;

      if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y, &unit_type))
         png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);
   }
#endif
#if defined(PNG_sBIT_SUPPORTED)
   {
      png_color_8p sig_bit;

      if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit))
         png_set_sBIT(write_ptr, write_info_ptr, sig_bit);
   }
#endif
#if defined(PNG_sCAL_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
   {
      int unit;
      double scal_width, scal_height;

      if (png_get_sCAL(read_ptr, read_info_ptr, &unit, &scal_width,
         &scal_height))
      {
         png_set_sCAL(write_ptr, write_info_ptr, unit, scal_width, scal_height);
      }
   }
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
   {
      int unit;
      png_charp scal_width, scal_height;

      if (png_get_sCAL_s(read_ptr, read_info_ptr, &unit, &scal_width,
          &scal_height))
      {
         png_set_sCAL_s(write_ptr, write_info_ptr, unit, scal_width, scal_height);
      }
   }
#endif
#endif
#endif
#if defined(PNG_TEXT_SUPPORTED)
   {
      png_textp text_ptr;
      int num_text;

      if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)
      {
         png_debug1(0, "Handling %d iTXt/tEXt/zTXt chunks", num_text);
         png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);
      }
   }
#endif
#if defined(PNG_tIME_SUPPORTED)
   {
      png_timep mod_time;

      if (png_get_tIME(read_ptr, read_info_ptr, &mod_time))
      {
         png_set_tIME(write_ptr, write_info_ptr, mod_time);
#if defined(PNG_TIME_RFC1123_SUPPORTED)
         /* We have to use png_memcpy instead of "=" because the string
          * pointed to by png_convert_to_rfc1123() gets free'ed before
          * we use it.
          */
         png_memcpy(tIME_string,
                    png_convert_to_rfc1123(read_ptr, mod_time),
                    png_sizeof(tIME_string));
         tIME_string[png_sizeof(tIME_string) - 1] = '\0';
         tIME_chunk_present++;
#endif /* PNG_TIME_RFC1123_SUPPORTED */
      }
   }
#endif
#if defined(PNG_tRNS_SUPPORTED)
   {
      png_bytep trans;
      int num_trans;
      png_color_16p trans_values;

      if (png_get_tRNS(read_ptr, read_info_ptr, &trans, &num_trans,
         &trans_values))
      {
         int sample_max = (1 << read_info_ptr->bit_depth);
         /* libpng doesn't reject a tRNS chunk with out-of-range samples */
         if (!((read_info_ptr->color_type == PNG_COLOR_TYPE_GRAY &&
             (int)trans_values->gray > sample_max) ||
             (read_info_ptr->color_type == PNG_COLOR_TYPE_RGB &&
             ((int)trans_values->red > sample_max ||
             (int)trans_values->green > sample_max ||
             (int)trans_values->blue > sample_max))))
            png_set_tRNS(write_ptr, write_info_ptr, trans, num_trans,
               trans_values);
      }
   }
#endif


#ifdef SINGLE_ROWBUF_ALLOC
   png_debug(0, "Allocating row buffer...");
   row_buf = (png_bytep)png_malloc(read_ptr,
      png_get_rowbytes(read_ptr, read_info_ptr));
   png_debug1(0, "0x%08lx", (unsigned long)row_buf);
#endif /* SINGLE_ROWBUF_ALLOC */
   png_debug(0, "Writing row data");

#if defined(PNG_READ_INTERLACING_SUPPORTED) || \
  defined(PNG_WRITE_INTERLACING_SUPPORTED)
   num_pass = png_set_interlace_handling(read_ptr);
#  ifdef PNG_WRITE_SUPPORTED
   png_set_interlace_handling(write_ptr);
#  endif
#else
   num_pass = 1;
#endif

#ifdef PNGTEST_TIMING
   t_stop = (float)clock();
   t_misc += (t_stop - t_start);
   t_start = t_stop;
#endif
   for (pass = 0; pass < num_pass; pass++)
   {
      png_debug1(0, "Writing row data for pass %d", pass);
      for (y = 0; y < height; y++)
      {
#ifndef SINGLE_ROWBUF_ALLOC
         png_debug2(0, "Allocating row buffer (pass %d, y = %ld)...", pass, y);
         row_buf = (png_bytep)png_malloc(read_ptr,
            png_get_rowbytes(read_ptr, read_info_ptr));
         png_debug2(0, "0x%08lx (%ld bytes)", (unsigned long)row_buf,
            png_get_rowbytes(read_ptr, read_info_ptr));
#endif /* !SINGLE_ROWBUF_ALLOC */
         png_read_rows(read_ptr, (png_bytepp)&row_buf, png_bytepp_NULL, 1);


#ifndef SINGLE_ROWBUF_ALLOC
         png_debug2(0, "Freeing row buffer (pass %d, y = %ld)", pass, y);
         png_free(read_ptr, row_buf);
         row_buf = NULL;
#endif /* !SINGLE_ROWBUF_ALLOC */
      }
   }


   png_debug(0, "Reading and writing end_info data");

   png_read_end(read_ptr, end_info_ptr);
#if defined(PNG_TEXT_SUPPORTED)
   {
      png_textp text_ptr;
      int num_text;

      if (png_get_text(read_ptr, end_info_ptr, &text_ptr, &num_text) > 0)
      {
         png_debug1(0, "Handling %d iTXt/tEXt/zTXt chunks", num_text);
         png_set_text(write_ptr, write_end_info_ptr, text_ptr, num_text);
      }
   }
#endif
#if defined(PNG_tIME_SUPPORTED)
   {
      png_timep mod_time;

      if (png_get_tIME(read_ptr, end_info_ptr, &mod_time))
      {
         png_set_tIME(write_ptr, write_end_info_ptr, mod_time);
#if defined(PNG_TIME_RFC1123_SUPPORTED)
         /* We have to use png_memcpy instead of "=" because the string
            pointed to by png_convert_to_rfc1123() gets free'ed before
            we use it */
         png_memcpy(tIME_string,
                    png_convert_to_rfc1123(read_ptr, mod_time),
                    png_sizeof(tIME_string));
         tIME_string[png_sizeof(tIME_string) - 1] = '\0';
         tIME_chunk_present++;
#endif /* PNG_TIME_RFC1123_SUPPORTED */
      }
   }
#endif

#ifdef PNG_EASY_ACCESS_SUPPORTED
   if (verbose)
   {
      png_uint_32 iwidth, iheight;
      iwidth = png_get_image_width(write_ptr, write_info_ptr);
      iheight = png_get_image_height(write_ptr, write_info_ptr);
      fprintf(STDERR, "\n Image width = %lu, height = %lu\n",
         (unsigned long)iwidth, (unsigned long)iheight);
   }
#endif

   png_debug(0, "Destroying data structs");
#ifdef SINGLE_ROWBUF_ALLOC
   png_debug(1, "destroying row_buf for read_ptr");
   png_free(read_ptr, row_buf);
   row_buf = NULL;
#endif /* SINGLE_ROWBUF_ALLOC */
   png_debug(1, "destroying read_ptr, read_info_ptr, end_info_ptr");
   png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
   png_debug(0, "Destruction complete.");

   FCLOSE(fpin);
   FCLOSE(fpout);

   png_debug(0, "Opening files for comparison");
#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, inname, -1, path, MAX_PATH);
   if ((fpin = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpin = fopen(inname, "rb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not find file %s\n", inname);
      return (1);
   }

#if defined(_WIN32_WCE)
   MultiByteToWideChar(CP_ACP, 0, outname, -1, path, MAX_PATH);
   if ((fpout = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
#else
   if ((fpout = fopen(outname, "rb")) == NULL)
#endif
   {
      fprintf(STDERR, "Could not find file %s\n", outname);
      FCLOSE(fpin);
      return (1);
   }

   for (;;)
   {
      png_size_t num_in, num_out;

         READFILE(fpin, inbuf, 1, num_in);
         READFILE(fpout, outbuf, 1, num_out);

      if (num_in != num_out)
      {
         fprintf(STDERR, "\nFiles %s and %s are of a different size\n",
                 inname, outname);
         if (wrote_question == 0)
         {
            fprintf(STDERR,
         "   Was %s written with the same maximum IDAT chunk size (%d bytes),",
              inname, PNG_ZBUF_SIZE);
            fprintf(STDERR,
              "\n   filtering heuristic (libpng default), compression");
            fprintf(STDERR,
              " level (zlib default),\n   and zlib version (%s)?\n\n",
              ZLIB_VERSION);
            wrote_question = 1;
         }
         FCLOSE(fpin);
         FCLOSE(fpout);
         return (0);
      }

      if (!num_in)
         break;

      if (png_memcmp(inbuf, outbuf, num_in))
      {
         fprintf(STDERR, "\nFiles %s and %s are different\n", inname, outname);
         if (wrote_question == 0)
         {
            fprintf(STDERR,
         "   Was %s written with the same maximum IDAT chunk size (%d bytes),",
                 inname, PNG_ZBUF_SIZE);
            fprintf(STDERR,
              "\n   filtering heuristic (libpng default), compression");
            fprintf(STDERR,
              " level (zlib default),\n   and zlib version (%s)?\n\n",
              ZLIB_VERSION);
            wrote_question = 1;
         }
         FCLOSE(fpin);
         FCLOSE(fpout);
         return (0);
      }
   }

   FCLOSE(fpin);
   FCLOSE(fpout);

   return (0);
}
