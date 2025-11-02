# 库函数API文档

## 概述
Vest-OS提供了丰富的标准库和专用库，简化应用程序开发。本节介绍主要库函数的使用方法。

## 目录
- [标准C库 (libc)](#标准c库-libc)
- [线程库 (pthread)](#线程库-pthread)
- [网络库 (libnet)](#网络库-libnet)
- [图形库 (libgui)](#图形库-libgui)
- [加密库 (libcrypto)](#加密库-libcrypto)

## 标准C库 (libc)

### 字符串处理

#### strlen()
计算字符串长度

```c
#include <string.h>
size_t strlen(const char *s);
```

**示例：**
```c
char *str = "Hello World";
size_t len = strlen(str);  // len = 11
```

#### strcpy()/strncpy()
复制字符串

```c
#include <string.h>
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
```

**示例：**
```c
char dest[50];
strcpy(dest, "Hello");           // dest = "Hello"
strncpy(dest, "World", 3);       // dest = "Wor"
```

#### strcat()/strncat()
连接字符串

```c
#include <string.h>
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
```

#### strcmp()/strncmp()
比较字符串

```c
#include <string.h>
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
```

**返回值：**
- 0: 字符串相等
- 负数: s1 < s2
- 正数: s1 > s2

#### strstr()
查找子串

```c
#include <string.h>
char *strstr(const char *haystack, const char *needle);
```

### 内存管理

#### malloc()/calloc()/realloc()/free()
动态内存分配

```c
#include <stdlib.h>
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
```

**示例：**
```c
// 分配内存
int *arr = (int *)malloc(10 * sizeof(int));
if (arr == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
}

// 使用内存
for (int i = 0; i < 10; i++) {
    arr[i] = i;
}

// 重新分配
int *new_arr = (int *)realloc(arr, 20 * sizeof(int));
if (new_arr == NULL) {
    free(arr);
    exit(EXIT_FAILURE);
}
arr = new_arr;

// 释放内存
free(arr);
```

#### memcpy()/memmove()
内存复制

```c
#include <string.h>
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
```

**注意：** `memmove()`可以处理重叠区域，`memcpy()`不能。

### 输入输出

#### printf()系列
格式化输出

```c
#include <stdio.h>
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
```

**格式说明符：**
- `%d`: 十进制整数
- `%f`: 浮点数
- `%s`: 字符串
- `%c`: 字符
- `%p`: 指针
- `%x`: 十六进制整数

#### scanf()系列
格式化输入

```c
#include <stdio.h>
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
```

#### fopen()/fclose()
打开/关闭文件

```c
#include <stdio.h>
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
```

**模式：**
- `"r"`: 只读
- `"w"`: 只写（创建或截断）
- `"a"`: 追加
- `"r+"`: 读写
- `"w+"`: 读写（创建或截断）
- `"a+"`: 读写（追加）

#### fread()/fwrite()
二进制读写

```c
#include <stdio.h>
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
```

### 数学函数

#### 基本数学运算

```c
#include <math.h>
double sin(double x);        // 正弦
double cos(double x);        // 余弦
double tan(double x);        // 正切
double sqrt(double x);       // 平方根
double pow(double x, double y); // x的y次方
double exp(double x);        // e的x次方
double log(double x);        // 自然对数
double log10(double x);      // 常用对数
double ceil(double x);       // 向上取整
double floor(double x);      // 向下取整
double fabs(double x);       // 绝对值
```

## 线程库 (pthread)

### pthread_create()
创建线程

```c
#include <pthread.h>
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
```

**示例：**
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
等待线程结束

```c
#include <pthread.h>
int pthread_join(pthread_t thread, void **retval);
```

### pthread_mutex_init()/lock()/unlock()
互斥锁

```c
#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

**示例：**
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
条件变量

```c
#include <pthread.h>
int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond,
                      pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
```

**示例：**
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
    // 处理数据
    pthread_mutex_unlock(&mutex);
    return NULL;
}
```

## 网络库 (libnet)

### gethostbyname()/getaddrinfo()
域名解析

```c
#include <netdb.h>
struct hostent *gethostbyname(const char *name);
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

**示例：**
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

// 使用result...

freeaddrinfo(result);
```

### libcurl
HTTP客户端库

```c
#include <curl/curl.h>

// 初始化
curl_global_init(CURL_GLOBAL_DEFAULT);
CURL *curl = curl_easy_init();

if (curl) {
    // 设置URL
    curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }

    // 清理
    curl_easy_cleanup(curl);
}

curl_global_cleanup();
```

## 图形库 (libgui)

### 基本窗口操作

```c
#include <libgui.h>

// 创建窗口
window_t *win = gui_window_create(800, 600, "My Application");

// 设置窗口属性
gui_window_set_title(win, "New Title");
gui_window_resize(win, 1024, 768);
gui_window_show(win);

// 创建按钮
button_t *btn = gui_button_create(100, 100, 200, 50, "Click Me");
gui_window_add_widget(win, btn);

// 事件循环
gui_run();

// 清理
gui_window_destroy(win);
```

### 绘图操作

```c
#include <libgui.h>

// 创建画布
canvas_t *canvas = gui_canvas_create(800, 600);

// 绘制基本图形
gui_canvas_set_color(canvas, 255, 0, 0);  // 红色
gui_canvas_draw_line(canvas, 0, 0, 100, 100);
gui_canvas_draw_rectangle(canvas, 50, 50, 200, 150);
gui_canvas_draw_circle(canvas, 400, 300, 50);

// 绘制文本
gui_canvas_set_font(canvas, "Arial", 24);
gui_canvas_draw_text(canvas, 10, 10, "Hello GUI");

// 显示画布
gui_canvas_show(canvas);
```

## 加密库 (libcrypto)

### 哈希函数

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

### 对称加密

```c
#include <libcrypto.h>

// AES加密
int aes_encrypt(const unsigned char *plaintext, int plaintext_len,
                const unsigned char *key, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // 创建并初始化上下文
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    // 初始化加密
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;

    // 加密数据
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len,
                               plaintext, plaintext_len))
        return -1;
    ciphertext_len = len;

    // 完成加密
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return -1;
    ciphertext_len += len;

    // 清理
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}
```

### 非对称加密

```c
#include <libcrypto.h>

// RSA密钥生成
RSA *rsa_generate_key(int bits) {
    RSA *rsa = RSA_new();
    BIGNUM *e = BN_new();

    BN_set_word(e, RSA_F4);
    RSA_generate_key_ex(rsa, bits, e, NULL);

    BN_free(e);
    return rsa;
}

// RSA加密
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

## 最佳实践

### 1. 内存管理
- 始终检查分配是否成功
- 配对的分配/释放函数
- 避免内存泄露

### 2. 错误处理
- 检查所有函数返回值
- 使用`errno`和`strerror()`获取错误信息
- 合理处理错误情况

### 3. 线程安全
- 了解哪些函数是线程安全的
- 正确使用同步原语
- 避免死锁

### 4. 性能优化
- 缓冲区大小优化
- 减少系统调用次数
- 使用高效的算法

---

*文档版本：1.0*
*最后更新：2024年1月*