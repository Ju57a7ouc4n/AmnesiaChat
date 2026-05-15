#include "vault.h"

pChat newChat(uint32_t chatId){
    pChat aux = (ChatArena*)malloc(sizeof(ChatArena));
    
    if(aux == NULL)
        return NULL;

    if(mlock(aux, sizeof(ChatArena)) != 0) {
        free(aux);
        return NULL;
    }

    memset(aux, 0, sizeof(ChatArena));

    aux->chatId = chatId;
    aux->index = -1; 

    return aux;
}

void setSharedSecret(ChatArena* chat, const unsigned char* secret) {
    if (chat != NULL && secret != NULL) {
        memcpy(chat->shared_secret, secret, crypto_secretbox_KEYBYTES);
    }
}

void newMessage(ChatArena* chat, const char* text){
    size_t total_length = strlen(text);
    size_t procesed_bytes = 0;

    if(chat!=NULL && text!=NULL){

        if(chat->index == MAX_INDEX-1 || chat->index == -1)
            chat->index=0;
        else
            chat->index+=1;

        while(procesed_bytes<total_length){

            memset(&chat->temp[chat->index], 0, sizeof(chat->temp[chat->index]));

            size_t bytes_to_cpy = total_length - procesed_bytes;

            if (bytes_to_cpy > UTIL_TEXT) {
                bytes_to_cpy = UTIL_TEXT;
                chat->temp[chat->index].is_fragmented = 1;
            } 
            else{
                chat->temp[chat->index].is_fragmented = 0;
            }

            randombytes_buf(chat->temp[chat->index].nonce, crypto_secretbox_NONCEBYTES);

            crypto_secretbox_easy((unsigned char*)chat->temp[chat->index].data, 
                                  (const unsigned char*)(text+procesed_bytes), 
                                  bytes_to_cpy, 
                                  chat->temp[chat->index].nonce, 
                                  chat->shared_secret);

            chat->temp[chat->index].used = 1;
            procesed_bytes+=bytes_to_cpy;
            
            if (procesed_bytes < total_length) {
                if(chat->index==MAX_INDEX-1)
                    chat->index=0;
                else
                    chat->index+=1;
            }
        }
    }
}

void endChat(ChatArena* chat){
    if(chat != NULL){
        memset_explicit(chat, 0, sizeof(ChatArena));
        munlock(chat, sizeof(ChatArena));
        free(chat);
    }
}

void getChatHistory(ChatArena* chat, void (*on_msg_decrypted)(const char*, int)) {
    
    if (chat != NULL && on_msg_decrypted != NULL){

        unsigned char texto_desencriptado[UTIL_TEXT + 1];

        for (int i = 0; i < MAX_INDEX; i++) {
        
            int pos = (chat->index + 1 + i) % MAX_INDEX;

            if (pos >= 0 && chat->temp[pos].used == 1) {
                if (crypto_secretbox_open_easy(
                    texto_desencriptado, 
                    (const unsigned char*)chat->temp[pos].data, 
                    512,
                    chat->temp[pos].nonce, 
                    chat->shared_secret) == 0) {

                    texto_desencriptado[UTIL_TEXT] = '\0'; 

                    on_msg_decrypted((const char*)texto_desencriptado, chat->temp[pos].is_fragmented);
                } 
            }
        }
    }
}