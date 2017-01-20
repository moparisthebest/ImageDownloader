#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>

#define TAG_IV_LENGTH 16
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
    int outlen, rv = 0, final_outlen, decrypt = 1;
    size_t read, actual_size = 0, total_size = INITIAL_BUFFER_SIZE;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <key> [enc]\n", argv[0]);
        return 1;
    }

    // this means we want to encrypt, not decrypt
    if (argc > 2 && strcmp("enc", argv[2]) == 0)
        decrypt = 0;

    hex2string(argv[1], &gcm_ivkey);

    gcm_ct = malloc(total_size);

    while ((read = fread(gcm_ct + actual_size, 1, BYTES_PER_READ, stdin)) > 0) {
        actual_size += read;
        if ((actual_size + BYTES_PER_READ) > total_size) {
            total_size = total_size * 1.5;
            gcm_ct = realloc(gcm_ct,total_size);
        }
    }

    if (actual_size < (decrypt ? 17 : 1)) {
        fprintf(stderr, "File too small for %scryption\n", decrypt ? "de" : "en");
        return 1;
    }

    if(decrypt)
        actual_size -= TAG_IV_LENGTH;

    gcm_pt = malloc(decrypt ? actual_size : (actual_size + TAG_IV_LENGTH));

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    /* Select cipher */
    if(decrypt)
        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    else
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);

    /* Set IV length, omit for 96 bits */
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, TAG_IV_LENGTH, NULL);

    if(decrypt) {
        /* Specify key and IV */
        EVP_DecryptInit_ex(ctx, NULL, NULL, gcm_ivkey + TAG_IV_LENGTH, gcm_ivkey);

        /* Set expected tag value. */
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_IV_LENGTH, gcm_ct + actual_size);

        /* Decrypt plaintext */
        EVP_DecryptUpdate(ctx, gcm_pt, &outlen, gcm_ct, actual_size);

        /* Finalise: note get no output for GCM */
        rv = EVP_DecryptFinal_ex(ctx, gcm_pt, &final_outlen);
    } else {
        /* Specify key and IV */
        EVP_EncryptInit_ex(ctx, NULL, NULL, gcm_ivkey + TAG_IV_LENGTH, gcm_ivkey);

        /* Encrypt plaintext */
        EVP_EncryptUpdate(ctx, gcm_pt, &outlen, gcm_ct, actual_size);

        /* Finalise: note get no output for GCM */
        rv = EVP_EncryptFinal_ex(ctx, gcm_pt, &final_outlen);

        /* Get expected tag value. */
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_IV_LENGTH, gcm_pt + actual_size);
    }

    EVP_CIPHER_CTX_free(ctx);
    free(gcm_ivkey);
    free(gcm_ct);

    if (rv > 0) {
        // success!
        fwrite(gcm_pt, 1, decrypt ? outlen : (outlen + TAG_IV_LENGTH), stdout);
        free(gcm_pt);
        return 0;
    } else {
        fprintf(stderr, "File integrity check failed\n");
        free(gcm_pt);
        return 1;
    }
}

// compile with: gcc aesgcm.c -lcrypto -o aesgcm
