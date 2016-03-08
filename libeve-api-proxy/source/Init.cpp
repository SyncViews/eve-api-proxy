#include "Precompiled.hpp"
#include "Init.hpp"

#ifdef _WIN32
SecurityFunctionTableW *sspi;

void init()
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    sspi = InitSecurityInterfaceW();
}
#else

SSL_CTX* openssl_ctx;
const SSL_METHOD *openssl_method;
std::vector<pthread_mutex_t> openssl_mutexes;
static unsigned long openssl_thread_id_function(void)
{
    return (unsigned long)pthread_self();
}
void openssl_thread_id(CRYPTO_THREADID *tid)
{
    CRYPTO_THREADID_set_numeric(tid, (unsigned long)pthread_self());
}
void openssl_locking_callback(int mode, int type, const char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(openssl_mutexes[type]));
    }
    else {
        pthread_mutex_unlock(&(openssl_mutexes[type]));
    }
}

void init()
{
    OpenSSL_add_ssl_algorithms();
    openssl_method = SSLv23_client_method();//TLS_client_method();
    openssl_ctx = SSL_CTX_new(openssl_method);
    if (!openssl_ctx) throw std::runtime_error("SSL_CTX_new failed");

    openssl_mutexes.resize(CRYPTO_num_locks(), PTHREAD_MUTEX_INITIALIZER);
    CRYPTO_THREADID_set_callback(openssl_thread_id);
    CRYPTO_set_locking_callback(openssl_locking_callback);

    //Linux sends a sigpipe when a socket or pipe has an error. better to handle the error where it
    //happens at the read/write site
    signal(SIGPIPE, SIG_IGN);
}

#endif
