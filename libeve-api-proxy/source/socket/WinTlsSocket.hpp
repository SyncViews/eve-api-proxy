#pragma once
#include "Socket.hpp"
#include "TcpSocket.hpp"

class TlsSocket : public Socket
{
public:
    TlsSocket();
    ~TlsSocket();

    virtual void connect(const std::string & host, uint16_t port) override;

    virtual bool is_connected() const override;

    virtual void close() override;
    virtual void force_close() override;

    virtual std::string address_str() const override;

    virtual size_t send(const uint8_t * bytes, size_t len) override;

    virtual size_t recv(uint8_t * bytes, size_t len) override;
private:
    TcpSocket tcp;
    CtxtHandle context;
    CredHandle credentials;
    std::vector<uint8_t> recv_encrypted_buffer;
    std::vector<uint8_t> recv_decrypted_buffer;
    SecPkgContext_StreamSizes sec_sizes;
    std::unique_ptr<uint8_t[]> header_buffer;
    std::unique_ptr<uint8_t[]> trailer_buffer;

    void clear_schannel_handles();
    void init_credentials();
    void client_handshake_loop(bool initial_read);
    void client_handshake();
    void send_sec_buffers(const SecBufferDesc &buffers);
    /**recv some data into recv_encrypted_buffer*/
    bool recv_encrypted();
};
