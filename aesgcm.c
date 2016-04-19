#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>

#define BYTES_PER_READ 32 * 1024 // 32kb
#define INITIAL_BUFFER_SIZE 256 * 1024 // 256kb, must be at least 2*BYTES_PER_READ

void hex2string(char *src, unsigned char **dst_p)
{
    *dst_p = malloc(strlen(src)/2);
    unsigned char *dst = *dst_p;
    unsigned char *end = dst + (strlen(src)/2);
    unsigned int u;

    while (dst < end && sscanf(src, "%2x", &u) == 1) {
        *dst++ = u;
        src += 2;
    }
}

int main(int argc, char **argv)
{
    unsigned char *gcm_ivkey, *gcm_ct, *gcm_pt;
    int outlen, rv, final_outlen;
    size_t read, actual_size = 0, total_size = INITIAL_BUFFER_SIZE;

    if (argc < 2) {
        printf("Usage: %s <key>\n", argv[0]);
        return 1;
    }

    hex2string(argv[1], &gcm_ivkey);

    gcm_ct = malloc(total_size);

    while ((read = fread(gcm_ct + actual_size, 1, BYTES_PER_READ, stdin)) > 0) {
        actual_size += read;
        if ((actual_size + BYTES_PER_READ) > total_size) {
            total_size = total_size * 1.5;
            gcm_ct = realloc(gcm_ct,total_size);
        }
    }

    if (actual_size < 32) {
        fprintf(stderr, "File too small for decryption\n");
        return 1;
    }

    actual_size -= 16;

    gcm_pt = malloc(actual_size);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    /* Select cipher */
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);

    /* Set IV length, omit for 96 bits */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL);

    /* Specify key and IV */
    EVP_DecryptInit_ex(ctx, NULL, NULL, gcm_ivkey+16, gcm_ivkey);

    /* Decrypt plaintext */
    EVP_DecryptUpdate(ctx, gcm_pt, &outlen, gcm_ct, actual_size);

    /* Set expected tag value. */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, gcm_ct + actual_size);

    /* Finalise: note get no output for GCM */
    rv = EVP_DecryptFinal_ex(ctx, gcm_pt, &final_outlen);

    EVP_CIPHER_CTX_free(ctx);
    free(gcm_ivkey);
    free(gcm_ct);

    if (rv > 0) {
        // success!
        fwrite(gcm_pt, 1, outlen, stdout);
        free(gcm_pt);
        return 0;
    } else {
        fprintf(stderr, "File integrity check failed\n");
        free(gcm_pt);
        return 1;
    }
}

// compile with: gcc aesgcm.c -lcrypto -o aesgcm
