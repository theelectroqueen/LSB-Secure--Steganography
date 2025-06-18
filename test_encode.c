#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"

void decode_message_from_image(const char *stego_image, const char *output_file);
void xor_encrypt_decrypt(char *data, int size, const char *password);  // declare here too

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Usage:\n");
        printf("  To Encode: %s -e <input.bmp> <secret.txt> <output.bmp>\n", argv[0]);
        printf("  To Decode: %s -d <stego.bmp> <extracted.txt>\n", argv[0]);
        return 1;
    }

    // Encoding mode
    if (strcmp(argv[1], "-e") == 0)
    {
        if (argc != 5)
        {
            fprintf(stderr, "ERROR: Incorrect arguments for encoding.\n");
            printf("Usage: %s -e <input.bmp> <secret.txt> <output.bmp>\n", argv[0]);
            return 1;
        }

        EncodeInfo encInfo;
        encInfo.src_image_fname = argv[2];
        encInfo.secret_fname = argv[3];
        encInfo.stego_image_fname = argv[4];

        if (open_files(&encInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: open_files failed\n");
            return 1;
        }

        printf("SUCCESS: open_files completed\n");

        uint img_size = get_image_size_for_bmp(encInfo.fptr_src_image);
        printf("INFO: Image size = %u\n", img_size);

        // Read the secret message into buffer
        fseek(encInfo.fptr_secret, 0, SEEK_END);
        int secret_size = ftell(encInfo.fptr_secret);
        rewind(encInfo.fptr_secret);

        char *buffer = malloc(secret_size + 1);
        fread(buffer, 1, secret_size, encInfo.fptr_secret);
        buffer[secret_size] = '\0';

        // Ask for password
        char password[100];
        printf("Enter password to encrypt: ");
        scanf("%s", password);

        // Encrypt the message before embedding
        xor_encrypt_decrypt(buffer, secret_size, password);

        // Copy BMP header
        fseek(encInfo.fptr_src_image, 0, SEEK_SET);
        fseek(encInfo.fptr_stego_image, 0, SEEK_SET);
        char header[54];
        fread(header, 1, 54, encInfo.fptr_src_image);
        fwrite(header, 1, 54, encInfo.fptr_stego_image);

        // Embed the encrypted message in image
        for (int i = 0; i < secret_size; i++)
        {
            char ch = buffer[i];
            for (int bit = 7; bit >= 0; bit--)
            {
                char byte;
                fread(&byte, 1, 1, encInfo.fptr_src_image);
                byte = (byte & 0xFE) | ((ch >> bit) & 1);  // ✅ FIXED: = instead of ==
                fwrite(&byte, 1, 1, encInfo.fptr_stego_image);
            }
        }

        // Embed null terminator to mark end
        char null_char = '\0';
        for (int bit = 7; bit >= 0; bit--)
        {
            char byte;
            fread(&byte, 1, 1, encInfo.fptr_src_image);
            byte = (byte & 0xFE) | ((null_char >> bit) & 1);
            fwrite(&byte, 1, 1, encInfo.fptr_stego_image);
        }

        // Copy rest of the image
        char ch;
        while (fread(&ch, 1, 1, encInfo.fptr_src_image))
        {
            fwrite(&ch, 1, 1, encInfo.fptr_stego_image);
        }

        printf("Encoding complete. Encrypted message hidden in %s\n", encInfo.stego_image_fname);
        free(buffer);
    }

    // Decoding mode
    else if (strcmp(argv[1], "-d") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "ERROR: Incorrect arguments for decoding.\n");
            printf("Usage: %s -d <stego.bmp> <extracted.txt>\n", argv[0]);
            return 1;
        }

        decode_message_from_image(argv[2], argv[3]);
    }
    else
    {
        fprintf(stderr, "ERROR: Unknown option '%s'\n", argv[1]);
        return 1;
    }

    return 0;
}
