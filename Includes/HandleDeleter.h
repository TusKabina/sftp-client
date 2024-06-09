
#ifndef SFTP_INTERFACE_COMMONS_H
#define SFTP_INTERFACE_COMMONS_H
#include <curl/curl.h>
#include <memory>


struct CurlHandleDeleter {
    void operator()(CURL *handle) const {
        if (handle) {
            curl_easy_cleanup(handle);
        }
    }
};

struct CurlMHandleDeleter {
    void operator()(CURLM *handle) const {
        if (handle) {
            curl_multi_cleanup(handle);
        }
    }
};

struct CurlSlistsDeleter {
    void operator()(curl_slist* header) const {
        if (header) {
            curl_slist_free_all(header);
        }
    }
};
using CurlUniquePtr = std::unique_ptr<CURL, CurlHandleDeleter>;
using CurlMUniquePtr = std::unique_ptr<CURLM, CurlMHandleDeleter>;

#endif //SFTP_INTERFACE_COMMONS_H
