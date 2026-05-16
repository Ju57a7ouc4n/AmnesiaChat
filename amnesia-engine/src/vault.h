#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sodium.h>

#define MAX_INDEX 20
#define UTIL_TEXT (512 - crypto_secretbox_MACBYTES)

typedef struct {
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    char data[512];
    uint8_t used;
    uint8_t is_fragmented;
} Mensaje;

typedef struct {
    uint32_t chatId; 
    unsigned char shared_secret[crypto_secretbox_KEYBYTES];
    Mensaje temp[MAX_INDEX];
    int index;
} ChatArena;
typedef ChatArena * pChat;

pChat newChat(uint32_t chatId);

void newMessage(ChatArena* chat, const char* text);

void endChat(ChatArena* chat);

void getChatHistory(ChatArena* chat, void (*on_msg_decrypted)(const char*, int));

void setSharedSecret(ChatArena* chat, const unsigned char* secret);

int encryptNetMessage(ChatArena* chat, const char* plaintext, unsigned char* out_buffer, size_t* out_len);

int decryptNetMessage(ChatArena* chat, const unsigned char* in_buffer, size_t in_len, char* out_plaintext);