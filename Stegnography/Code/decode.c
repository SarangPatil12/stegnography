#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#include <stdlib.h> 
#include <unistd.h> 
/* Read and validate decode args from argv 
 * Input: command line arguments, source image, secret file name
 * Output: validate the command line arguments
 * Description: Read & validate the decode arguments for the argv
 * Return Value: e_success or e_failure, on file errors
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo) //<--- function definition for validate the args
{
    //stego image
    if(argv[2] != NULL && strstr(argv[2], ".bmp") != NULL ) //<--- validating the 2nd argument, checking the bmp file
	{
	decInfo->stego_image_fname = argv[2];     //<---- assigning the 2nd position to the source image file
	}
    else
    {
	printf("[ERROR] Please enter the .bmp file\n");
	return e_failure;
    }
    //decoding file name
    if(argv[3] != NULL)
    {
	//ext_ptr=strstr(argv[3],".");
	   if(strstr(argv[3],".txt") != NULL)  //check if .txt 
	   {
	      decInfo->decode_fname = argv[3];    //<---- assigning the rd position to the decoded file
	   }
	   else
	   {   
	      printf("[ERROR] Output file must be of type '.txt'\n");
	      return e_failure;
	   }   
    }
    else
	{
	   decInfo->decode_fname =  "output.txt";
	}
	return e_success;    //<---- returning the success after read and validating arguments
}
Status open_files_decode(DecodeInfo *decInfo) //<---- function definition for open files
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");   //<---- opening and reading the stego image file
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "[ERROR] Unable to open file %s\n", decInfo->stego_image_fname);

    	return e_failure;
    }
	// Open the output file for writing the extracted secret
    decInfo->fptr_decoded = fopen(decInfo->decode_fname, "w");
    if (decInfo->fptr_decoded == NULL)
    {
        perror("fopen");
        fprintf(stderr, "[ERROR] Unable to open output file %s\n", decInfo->decode_fname);
        return e_failure;
    }

    // No failure return e_success
    return e_success;
}
Status decode_byte_from_lsb(char *image_buffer, char *data)
{
    *data = 0;
    for (int i = 0; i < 8; i++)
    {
        *data |= (image_buffer[i] & 1) << i; // LSB-first reconstruction
    }
    return e_success;
}
Status decode_magic_string(int magic_str_length, DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    printf("[INFO] Decoding Magic String...\n");

    for (int i = 0; i < magic_str_length; i++)
    {
        // Read 8 bytes from image
        if (fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr, "[ERROR] Failed to read image data for magic string byte %d\n", i);
            return e_failure;
        }

        // Decode one character from LSBs
        if (decode_byte_from_lsb(image_buffer, &ch) != e_success)
        {
            fprintf(stderr, "[ERROR] Failed to decode byte %d from image buffer\n", i);
            return e_failure;
        }
    
    }

    printf("[INFO] Magic String decode success\n");
	printf("[MAGIC] %c\n",ch);
    return e_success;
}
Status decode_secret_file_extn_size(FILE *fptr_stego_image, int *size)
{
    char image_buffer[32];
    *size = 0;

    // Read 32 bytes from image
    if (fread(image_buffer, sizeof(char), 32, fptr_stego_image) != 32)
    {
        fprintf(stderr, "[ERROR] Failed to read 32 bytes for size decoding\n");
        return e_failure;
    }

    // Reconstruct integer from 32 LSBs (LSB-first)
    for (int i = 0; i < 32; i++)
    {
        *size |= (image_buffer[i] & 1) << i;
    }

    return e_success;
}
Status decode_data_from_image(int size, FILE *fptr_stego_image, FILE *fptr_decoded)
{
    char image_buffer[8];
    char ch;

    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, sizeof(char), 8, fptr_stego_image) != 8)
        {
            fprintf(stderr, "[ERROR] Failed to read image data for secret byte %d\n", i);
            return e_failure;
        }

        if (decode_byte_from_lsb(image_buffer, &ch) != e_success)
        {
            fprintf(stderr, "[ERROR] Failed to decode byte from image buffer\n");
            return e_failure;
        }

        fputc(ch, fptr_decoded);
    }

    return e_success;
}
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    // Allocate memory (+1 for null-terminator)
    decInfo->secret_file_extention = malloc(decInfo->secret_file_extn_size + 1);
    if (decInfo->secret_file_extention == NULL)
    {
        fprintf(stderr, "[ERROR] Memory allocation failed for secret file extension\n");
        return e_failure;
    }

    // Decode extension from image
    for (int i = 0; i < decInfo->secret_file_extn_size; i++)
    {
        char image_buffer[8];
        char ch;

        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr, "[ERROR] Failed to read image data for extension byte %d\n", i);
            free(decInfo->secret_file_extention);
            return e_failure;
        }

        if (decode_byte_from_lsb(image_buffer, &ch) != e_success)
        {
            fprintf(stderr, "[ERROR] Failed to decode extension byte\n");
            free(decInfo->secret_file_extention);
            return e_failure;
        }

        decInfo->secret_file_extention[i] = ch;
    }

    // Null-terminate the string
    decInfo->secret_file_extention[decInfo->secret_file_extn_size] = '\0';

    printf("[INFO] Decoded secret file extension: %s\n", decInfo->secret_file_extention);
    return e_success;
}
Status decode_secret_file_size(FILE *fptr_stego_image, int *size)
{
    char image_buffer[32];
    char ch;
    *size = 0;

    // Read 32 bytes from image to decode 4-byte int
    if (fread(image_buffer, 1, 32, fptr_stego_image) != 32)
    {
        fprintf(stderr, "[ERROR] Failed to read image data for size\n");
        return e_failure;
    }

    // Decode each bit and reconstruct the integer (LSB first)
    for (int i = 0; i < 32; i++)
    {
        int bit = image_buffer[i] & 1;
        *size |= (bit << i);
    }

    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char decoded_byte;

    printf("[INFO] Starting to decode secret file data (%d bytes)...\n", decInfo->secret_file_size);

    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        // Read 8 bytes from image
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr, "[ERROR] Failed to read image data for secret byte %d\n", i);
            return e_failure;
        }

        // Decode one byte
        if (decode_byte_from_lsb(image_buffer, &decoded_byte) != e_success)
        {
            fprintf(stderr, "[ERROR] Failed to decode byte %d from image\n", i);
            return e_failure;
        }

        // Write to output file
        if (fputc(decoded_byte, decInfo->fptr_decoded) == EOF)
        {
            fprintf(stderr, "[ERROR] Failed to write decoded byte %d to output file\n", i);
            return e_failure;
        }
    }
    return e_success;
}
Status dec_close_files(DecodeInfo *decInfo)
{
    if (decInfo->fptr_stego_image)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
        printf("[INFO] Closed stego image file\n");
    }

    if (decInfo->fptr_decoded)
    {
        fclose(decInfo->fptr_decoded);
        decInfo->fptr_decoded = NULL;
        printf("[INFO] Closed decoded output file\n");
    }

    return e_success;
}
Status do_decoding(DecodeInfo *decInfo)  //<---- function definition for decoding the process
{
	if(open_files_decode(decInfo)== e_success)    //&D1;
    {
        printf("[INFO] Opening files...\n");
    }
    else
    {
        printf("[ERROR] Failed to open files\n");
        return e_failure;
    }
	// parse 54B header
	rewind(decInfo -> fptr_stego_image);
	if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
    perror("fseek");
    fprintf(stderr, "[ERROR] Failed to seek past BMP header\n");
    return e_failure;
    }
	printf("[INFO] Skipped BMP header (54 bytes)\n");
	printf("\n-------- EXTRACTING METADATA --------\n");
	// Decode and print magic string
    if (decode_magic_string(MAGIC_STRING_LENGTH, decInfo) != e_success)
    {
		printf("[ERROR] failed to decode magic string\n");
        return e_failure;
    }
	if (decode_secret_file_extn_size(decInfo->fptr_stego_image, &decInfo->secret_file_extn_size) == e_success)
    {
       printf("[INFO] Decoded secret file extension size: %d bytes\n", decInfo->secret_file_extn_size);
    }
    else
    {
      fprintf(stderr, "[ERROR] Failed to decode secret file extension size\n");
      return e_failure;
    }
	printf("[INFO] Decoding secret file extension...\n");
	if (decode_secret_file_extn(decInfo) == e_success)
    {
       // Check if it's ".txt"
       if(strcmp(decInfo->secret_file_extention, ".txt") != 0)
       {
          fprintf(stderr, "[ERROR] Only .txt files are supported. Found: %s\n", decInfo->secret_file_extention);
          return e_failure;
       }
       else
       {
          printf("[INFO] Secret file extension is valid: .txt\n");
       }
    }  
    else
    {
       fprintf(stderr, "[ERROR] Failed to decode secret file extension\n");
       return e_failure;
    }   
	if (decode_secret_file_size(decInfo->fptr_stego_image, &decInfo->secret_file_size) == e_success)
    {
       printf("[INFO] Decoded secret file size: %d bytes\n", decInfo->secret_file_size);
    }
    else
    {
       fprintf(stderr, "[ERROR] Failed to decode secret file size\n");
       return e_failure;
    }
	printf("\n-------- EXTRACTING SECRET DATA --------\n");
	if (decode_secret_file_data(decInfo) == e_success)
    {
	   printf("[INFO] Secret file data decoded and written to %s successfully.\n", decInfo->decode_fname);
	}
    else
    {
       fprintf(stderr, "[ERROR] Failed to decode secret file data\n");
       return e_failure;
    }
	printf("\n-------- FINALIZING --------\n");
    dec_close_files(decInfo);
    return e_success;



	//return e_success;
}