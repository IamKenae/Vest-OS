# Library Function API Documentation

## Overview
Vest-OS provides a rich set of standard and specialized libraries to simplify application development. This section describes the usage of main library functions.

## Table of Contents
- [Standard C Library (libc)](#standard-c-library-libc)
- [Thread Library (pthread)](#thread-library-pthread)
- [Network Library (libnet)](#network-library-libnet)
- [Graphics Library (libgui)](#graphics-library-libgui)
- [Cryptography Library (libcrypto)](#cryptography-library-libcrypto)

## Standard C Library (libc)

### String Processing

#### strlen()
Calculate string length

```c
#include <string.h>
size_t strlen(const char *s);
```

**Example:**
```c
char *str = "Hello World";
size_t len = strlen(str);  // len = 11
```

#### strcpy()/strncpy()
Copy strings

```c
#include <string.h>
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
```

**Example:**
```c
char dest[50];
strcpy(dest, "Hello");           // dest = "Hello"
strncpy(dest, "World", 3);       // dest = "Wor"
```

#### strcat()/strncat()
Concatenate strings

```c
#include <string.h>
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
```

#### strcmp()/strncmp()
Compare strings

```c
#include <string.h>
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
```

**Return values:**
- 0: strings are equal
- Negative: s1 < s2
- Positive: s1 > s2

#### strstr()
Find substring

```c
#include <string.h>
char *strstr(const char *haystack, const char *needle);
```

### Memory Management

#### malloc()/calloc()/realloc()/free()
Dynamic memory allocation

```c
#include <stdlib.h>
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
```

**Example:**
```c
// Allocate memory
int *arr = (int *)malloc(10 * sizeof(int));
if (arr == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
}

// Use memory
for (int i = 0; i < 10; i++) {
    arr[i] = i;
}

// Reallocate
int *new_arr = (int *)realloc(arr, 20 * sizeof(int));
if (new_arr == NULL) {
    free(arr);
    exit(EXIT_FAILURE);
}
arr = new_arr;

// Free memory
free(arr);
```

#### memcpy()/memmove()
Memory copy

```c
#include <string.h>
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
```

**Note:** `memmove()` can handle overlapping regions, `memcpy()` cannot.

### Input/Output

#### printf() family
Formatted output

```c
#include <stdio.h>
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
```

**Format specifiers:**
- `%d`: Decimal integer
- `%f`: Floating point
- `%s`: String
- `%c`: Character
- `%p`: Pointer
- `%x`: Hexadecimal integer

#### scanf() family
Formatted input

```c
#include <stdio.h>
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
```

#### fopen()/fclose()
Open/close files

```c
#include <stdio.h>
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
```

**Modes:**
- `"r"`: Read-only
- `"w"`: Write-only (create or truncate)
- `"a"`: Append
- `"r+"`: Read-write
- `"w+"`: Read-write (create or truncate)
- `"a+"`: Read-write (append)

#### fread()/fwrite()
Binary read/write

```c
#include <stdio.h>
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
```

### Mathematical Functions

#### Basic Mathematical Operations

```c
#include <math.h>
double sin(double x);        // Sine
double cos(double x);        // Cosine
double tan(double x);        // Tangent
double sqrt(double x);       // Square root
double pow(double x, double y); // x to the power of y
double exp(double x);        // e to the power of x
double log(double x);        // Natural logarithm
double log10(double x);      // Common logarithm
double ceil(double x);       // Round up
double floor(double x);      // Round down
double fabs(double x);       // Absolute value
```

## Thread Library (pthread)

### pthread_create()
Create thread

```c
#include <pthread.h>
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
```

**Example:**
```c
#include <pthread.h>
#include <stdio.h>

void *thread_func(void *arg) {
    int *num = (int *)arg;
    printf("Thread %d running\n", *num);
    return NULL;
}

int main() {
    pthread_t tid;
    int num = 42;

    pthread_create(&tid, NULL, thread_func, &num);
    pthread_join(tid, NULL);

    return 0;
}
```

### pthread_join()
Wait for thread to finish

```c
#include <pthread.h>
int pthread_join(pthread_t thread, void **retval);
```

### pthread_mutex_init()/lock()/unlock()
Mutex

```c
#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

**Example:**
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_counter = 0;

void *increment_counter(void *arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
```

### pthread_cond_init()/wait()/signal()
Condition variables

```c
#include <pthread.h>
int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond,
                      pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
```

**Example:**
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

void *producer(void *arg) {
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *consumer(void *arg) {
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    // Process data
    pthread_mutex_unlock(&mutex);
    return NULL;
}
```

## Network Library (libnet)

### gethostbyname()/getaddrinfo()
Domain name resolution

```c
#include <netdb.h>
struct hostent *gethostbyname(const char *name);
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

**Example:**
```c
#include <sys/socket.h>
#include <netdb.h>

struct addrinfo hints, *result;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;

int ret = getaddrinfo("www.example.com", "80", &hints, &result);
if (ret != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
    return -1;
}

// Use result...

freeaddrinfo(result);
```

### libcurl
HTTP client library

```c
#include <curl/curl.h>

// Initialize
curl_global_init(CURL_GLOBAL_DEFAULT);
CURL *curl = curl_easy_init();

if (curl) {
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }

    // Cleanup
    curl_easy_cleanup(curl);
}

curl_global_cleanup();
```

## Graphics Library (libgui)

### Basic Window Operations

```c
#include <libgui.h>

// Create window
window_t *win = gui_window_create(800, 600, "My Application");

// Set window properties
gui_window_set_title(win, "New Title");
gui_window_resize(win, 1024, 768);
gui_window_show(win);

// Create button
button_t *btn = gui_button_create(100, 100, 200, 50, "Click Me");
gui_window_add_widget(win, btn);

// Event loop
gui_run();

// Cleanup
gui_window_destroy(win);
```

### Drawing Operations

```c
#include <libgui.h>

// Create canvas
canvas_t *canvas = gui_canvas_create(800, 600);

// Draw basic shapes
gui_canvas_set_color(canvas, 255, 0, 0);  // Red
gui_canvas_draw_line(canvas, 0, 0, 100, 100);
gui_canvas_draw_rectangle(canvas, 50, 50, 200, 150);
gui_canvas_draw_circle(canvas, 400, 300, 50);

// Draw text
gui_canvas_set_font(canvas, "Arial", 24);
gui_canvas_draw_text(canvas, 10, 10, "Hello GUI");

// Show canvas
gui_canvas_show(canvas);
```

## Cryptography Library (libcrypto)

### Hash Functions

```c
#include <libcrypto.h>

// MD5
char *md5_hash(const char *data) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, strlen(data));
    MD5_Final(digest, &ctx);

    char *hash = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(hash + i * 2, "%02x", digest[i]);
    }
    hash[MD5_DIGEST_LENGTH * 2] = '\0';

    return hash;
}

// SHA-256
char *sha256_hash(const char *data) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;

    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, strlen(data));
    SHA256_Final(digest, &ctx);

    char *hash = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash + i * 2, "%02x", digest[i]);
    }
    hash[SHA256_DIGEST_LENGTH * 2] = '\0';

    return hash;
}
```

### Symmetric Encryption

```c
#include <libcrypto.h>

// AES encryption
int aes_encrypt(const unsigned char *plaintext, int plaintext_len,
                const unsigned char *key, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Create and initialize context
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    // Initialize encryption
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;

    // Encrypt data
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len,
                               plaintext, plaintext_len))
        return -1;
    ciphertext_len = len;

    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return -1;
    ciphertext_len += len;

    // Cleanup
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}
```

### Asymmetric Encryption

```c
#include <libcrypto.h>

// RSA key generation
RSA *rsa_generate_key(int bits) {
    RSA *rsa = RSA_new();
    BIGNUM *e = BN_new();

    BN_set_word(e, RSA_F4);
    RSA_generate_key_ex(rsa, bits, e, NULL);

    BN_free(e);
    return rsa;
}

// RSA encryption
int rsa_encrypt(RSA *rsa, const char *msg, unsigned char **enc_msg) {
    int rsa_len = RSA_size(rsa);
    *enc_msg = malloc(rsa_len);

    return RSA_public_encrypt(strlen(msg) + 1,
                             (unsigned char *)msg,
                             *enc_msg,
                             rsa,
                             RSA_PKCS1_PADDING);
}
```

## Best Practices

### 1. Memory Management
- Always check if allocation succeeded
- Use paired allocation/deallocation functions
- Avoid memory leaks

### 2. Error Handling
- Check all function return values
- Use `errno` and `strerror()` to get error information
- Handle error situations appropriately

### 3. Thread Safety
- Understand which functions are thread-safe
- Use synchronization primitives correctly
- Avoid deadlocks

### 4. Performance Optimization
- Optimize buffer sizes
- Reduce system call frequency
- Use efficient algorithms

---

*Documentation Version: 1.0*
*Last updated: January 2024*