#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"

// Function to calculate BMP image size
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);  
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    return width * height * 3;
}

// Function to open input/output files for encoding
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

// XOR-based encryption/decryption function 
void xor_encrypt_decrypt(char *data, int size, const char *password)
{
    int pass_len = strlen(password);
    for (int i = 0; i < size; i++)
    {
        data[i] ^= password[i % pass_len];
    }
}

// Decoding function with password-based decryptionss
void decode_message_from_image(const char *stego_image, const char *output_file)
{
    FILE *fptr_image = fopen(stego_image, "r");
    if (fptr_image == NULL)
    {
        perror("fopen");
        return;
    }

    FILE *fptr_output = fopen(output_file, "w");
    if (fptr_output == NULL)
    {
        perror("fopen");
        fclose(fptr_image);
        return;
    }

    fseek(fptr_image, 54, SEEK_SET);

    #define MAX_MSG_SIZE 10000
    char message[MAX_MSG_SIZE];
    int msg_index = 0;
    char ch = 0;
    int bit_index = 0;

    // Rebuild characters from LSBs
    while (1)
    {
        char byte;
        if (fread(&byte, 1, 1, fptr_image) != 1)
            break;

        ch = (ch << 1) | (byte & 1);
        bit_index++;

        if (bit_index == 8)
        {
            if (ch == '\0')
                break;

            message[msg_index++] = ch;
            ch = 0;
            bit_index = 0;

            if (msg_index >= MAX_MSG_SIZE)
            {
                printf("ERROR: Message too long\n");
                break;
            }
        }
    }

    // Ask user for password
    char password[100];
    printf("Enter password to decrypt: ");
    scanf("%s", password);

    // Decrypt and null-terminate
    xor_encrypt_decrypt(message, msg_index, password);
    message[msg_index] = '\0';

    // Write decrypted message to file
    fprintf(fptr_output, "%s", message);
    printf("Decoded message written to %s\n", output_file);

    fclose(fptr_image);
    fclose(fptr_output);
}
