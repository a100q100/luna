//
//      _
//  ___/__)
// (, /      __   _
//   /   (_(_/ (_(_(_
//  (________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

#pragma once

#include "luna/private/safer_times.h"
#include "luna/private/response_generator.h"
#include "server.h"
#include <microhttpd.h>
#include <cstring>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace luna
{

const auto GET = "GET";
const auto POST = "POST";
const auto PUT = "PUT";
const auto PATCH = "PATCH";
const auto DELETE = "DELETE";
const auto OPTIONS = "OPTIONS";


class server::server_impl
{
public:

    server_impl();

    ~server_impl();

    void start();

    bool is_running();

    void stop();

    void await();

    server::port get_port();

    using request_handler = std::map<request_method, std::vector<std::pair<std::regex, endpoint_handler_cb>>>;

    server::request_handler_handle handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback, parameter::validators &&validators={});
    server::request_handler_handle handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback, parameter::validators &&validators={});
    server::request_handler_handle handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback, const parameter::validators &validators);
    server::request_handler_handle handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback, const parameter::validators &validators);

    server::request_handler_handle serve_files(const std::string &mount_point, const std::string &path_to_files);
    server::request_handler_handle serve_files(std::string &&mount_point, std::string &&path_to_files);

    void remove_request_handler(request_handler_handle item);

    server::error_handler_handle handle_404(server::error_handler_cb callback);
    server::error_handler_handle handle_error(status_code code, server::error_handler_cb callback);

    void remove_error_handler(error_handler_handle item);

    template<class H, class V>
    void add_global_header(H &&header, V &&value)
    {
        response_generator_.add_global_header(std::forward<H>(header), std::forward<V>(value));
    };

    void set_option(debug_output value);

    void set_option(use_thread_per_connection value);

    void set_option(use_epoll_if_available value);

    void set_option(const mime_type &mime_type);

    void set_option(error_handler_cb handler);

    void set_option(class port port);

    void set_option(accept_policy_cb handler);

    // MHD specific options

    void set_option(connection_memory_limit value);

    void set_option(connection_limit value);

    void set_option(connection_timeout value);

//    void set_option(notify_completed value);

    void set_option(per_ip_connection_limit value);

    void set_option(const sockaddr *value);

    void set_option(const https_mem_key &value);

    void set_option(const https_mem_cert &value);

//    void set_option(https_cred_type value);

    void set_option(const https_priorities &value);

    void set_option(listen_socket value);

    void set_option(thread_pool_size value);

    void set_option(unescaper_cb value);

//    void set_option(digest_auth_random value);

    void set_option(nonce_nc_size value);

    void set_option(thread_stack_size value);

    void set_option(const https_mem_trust &value);

    void set_option(connection_memory_increment value);

//    void set_option(https_cert_callback value);

//    void set_option(tcp_fastopen_queue_size value);

    void set_option(const https_mem_dhparams &value);

//    void set_option(listening_address_reuse value);

    void set_option(const https_key_password &value);

//    void set_option(notify_connection value);

    void set_option(const server_identifier &value);
    void set_option(const append_to_server_identifier &value);

    //middleware
    void set_option(middleware::before_request_handler value);
    void set_option(middleware::after_request_handler value);
    void set_option(middleware::after_error value);

    //static asset cacheing
    void set_option(std::pair<cache::read, cache::write> value);
    void set_option(server::enable_internal_file_cache value);
    void set_option(internal_file_cache_keep_alive value);



private:
    std::mutex lock_;

    std::map<request_method, server::request_handlers> request_handlers_;

    luna::headers global_headers_;

    bool debug_output_;

    bool ssl_mem_key_set_;
    bool ssl_mem_cert_set_;

    bool use_thread_per_connection_;

    bool use_epoll_if_available_;

    uint16_t port_;

    // string copies of options
    std::vector<std::string> https_mem_key_;
    std::vector<std::string> https_mem_cert_;
    std::vector<std::string> https_priorities_;
    std::vector<std::string> https_mem_trust_;
    std::vector<std::string> https_mem_dhparams_;
    std::vector<std::string> https_key_password_;

    // middlewares
    middleware::before_request_handler middleware_before_request_handler_;
    middleware::after_request_handler middleware_after_request_handler_;

    //options
    std::vector<MHD_OptionItem> options_;

    struct MHD_Daemon *daemon_;

    std::condition_variable running_cv_;

    ///// internal use-only callbacks

    int access_handler_callback_(struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method,
                                 const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size,
                                 void **con_cls);


    ////// external-use callbacks that can be set with options
    accept_policy_cb accept_policy_callback_; //has a default value

    unescaper_cb unescaper_callback_;

    ///// callback shims

    static int access_policy_callback_shim_(void *cls,
                                            const struct sockaddr *addr,
                                            socklen_t addrlen);

    static int access_handler_callback_shim_(void *cls,
                                             struct MHD_Connection *connection,
                                             const char *url,
                                             const char *method,
                                             const char *version,
                                             const char *upload_data,
                                             size_t *upload_data_size,
                                             void **con_cls);

    static void request_completed_callback_shim_(void *cls,
                                                 struct MHD_Connection *connection,
                                                 void **con_cls,
                                                 enum MHD_RequestTerminationCode toe);

    static void *uri_logger_callback_shim_(void *cls, const char *uri, struct MHD_Connection *con);

    static void logger_callback_shim_(void *cls, const char *fm, va_list ap);

    static size_t unescaper_callback_shim_(void *cls, struct MHD_Connection *c, char *s);

    static int iterate_postdata_shim_(void *cls,
                                      enum MHD_ValueKind kind,
                                      const char *key,
                                      const char *filename,
                                      const char *content_type,
                                      const char *transfer_encoding,
                                      const char *data,
                                      uint64_t off,
                                      size_t size);

    //TODO MHD_OPTION_HTTPS_CERT_CALLBACK callback_shim_

    //TODO I don't know what to do with this one yet.
//    static void notify_connection_callback_shim_(void *cls,
//                                                 struct MHD_Connection *connection,
//                                                 void **socket_context,
//                                                 enum MHD_ConnectionNotificationCode toe);


    ///// helpers
    response_generator response_generator_;
};

} //namespace luna
