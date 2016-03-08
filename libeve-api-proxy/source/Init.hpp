#pragma once
#include "Precompiled.hpp"

#ifdef _WIN32
extern SecurityFunctionTableW *sspi;
#else
extern SSL_CTX* openssl_ctx;
extern const SSL_METHOD *openssl_method;
#endif

void init();
