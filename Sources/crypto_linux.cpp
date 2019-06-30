#ifdef KORE_LINUX

#include <openssl/sha.h> // apt-get install libssl-dev
#include <openssl/pem.h>

#include <string>
#include <string.h>


char* base64encode(const void* data, int length) {
    BIO* b64_bio = BIO_new(BIO_f_base64());
    BIO* mem_bio = BIO_new(BIO_s_mem());
    BIO_push(b64_bio, mem_bio);
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64_bio, data, length);
    BIO_flush(b64_bio);
    BUF_MEM* mem_bio_mem_ptr;
    BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);
    BIO_set_close(mem_bio, BIO_NOCLOSE);
    BIO_free_all(b64_bio);
    BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);
    (*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';
    return (*mem_bio_mem_ptr).data;
}

std::string sha1(const char* data, int length) {
    unsigned char hash[32];
    SHA1((const unsigned char*)data, length, hash);
    return base64encode(hash, 20);
}

#endif
