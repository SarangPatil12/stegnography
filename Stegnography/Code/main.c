#include <stdio.h>
#include <string.h>
#include "encode.h"
#include"decode.h"
#include "types.h"

int main(int argc, char* argv[])
{
    // Print project banner
    printf("===========================================\n");
    printf("           STEGANOGRAPHY PROJECT\n");
    printf("===========================================\n\n");
    //validate command line argument
    if(argc<3){
        printf("[ERROR] Pass valid command line arguments\n");
        printf("Sample Usage : For Encoding ./a.out -e beautiful.bmp secret.txt [stegno.bmp]\n");
        printf("               For Decoding ./a.out -d stegno.bmp [data.txt]\n"); 
        return 1;                                //checking how many arguments passed
    }
    EncodeInfo E1;
    int result = check_operation_type(argv);
    if(result == e_encode)
    {
        if (argc < 4) {
            printf("[Error] Not enough arguments\n");
            return e_failure;
        }
        printf("[INFO] Selected Operation: Encoding\n");
        if(read_and_validate_encode_args(argv,&E1)== e_success)
        {
            printf("[INFO] Validating command-line arguments...\n");
            if (do_encoding(&E1) == e_success){
               printf("[INFO] Encoded stego image saved as: %s\n", E1.stego_image_fname);
               printf("[SUCCESS] Steganography encoding completed successfully!\n"); 
            }
            else{
                printf("[ERROR] Failed to Encode\n");
                return 1;
            }
        }
        else{
            printf("[ERROR] failed to read/validate encode args\n");
            return 1;
        }
    }
    else if (result == e_decode)
    {
        if (argc < 3) {
            printf("[ERROR] Not enough arguments for decoding\n");
            return e_failure;
        }

        DecodeInfo D1;
        printf("[INFO] Selected Operation: Decoding\n");

        if (read_and_validate_decode_args(argv, &D1) == e_success)
        {
            printf("[INFO] Validating decode arguments...\n");
            if (do_decoding(&D1) == e_success) {
                printf("[INFO] Decoded secret file saved as: %s\n", D1.decode_fname);
                printf("[SUCCESS] Decoding completed successfully!\n");
            } else {
                printf("[ERROR] Failed to decode\n");
                return 1;
            }
        } else {
            printf("[ERROR] Failed to read/validate decode args\n");
            return 1;
        }
    }
    else if((result == e_unsupported)){
    {
        printf("[Error] Invalid option\n");
        printf("Usage : For Encoding ./a.out -e beautiful.bmp secret.txt [stegno.bmp]\n");
        printf("        For Decoding ./a.out -d stegno.bmp [data.txt]\n");
    }

    return 0;
}
}
OperationType check_operation_type(char *argv[])
{
     if(strcmp(argv[1], "-e")==0){
        return e_encode;
     }
     else if(strcmp(argv[1], "-d")==0){
        return e_decode;
     }
     else{
        return e_unsupported;
     }
}
