#ifndef __HTTP_CLIENT_H_
#define __HTTP_CLIENT_H_

#include "spark_wiring_string.h"
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_usbserial.h"
#include "spark_wiring_printable.h"

/**
 * Defines for the HTTP methods.
 */
static const char* HTTP_METHOD_GET    = "GET";
static const char* HTTP_METHOD_POST   = "POST";
static const char* HTTP_METHOD_PUT    = "PUT";
static const char* HTTP_METHOD_DELETE = "DELETE";

static const size_t HTTP_CLIENT_BUFFER_SIZE = 2048;

/**
 * This struct is used to pass additional HTTP headers such as API-keys.
 * Normally you pass this as an array. The last entry must have NULL as key.
 */
typedef struct
{
  const char* header;
  const char* value;
} http_header_t;

/**
 * HTTP Request struct.
 * hostname request host
 * path	 request path
 * port     request port
 * body	 request body
 */
typedef struct
{
  IPAddress* hostname;
  char* path;
  uint16_t port;
  char* body;
} http_request_t;

/**
 * HTTP Response struct.
 * status  response status code.
 * body	response body
 */
typedef struct
{
  int status;
  char* body;
} http_response_t;

class HttpClient {

  public:

    HttpClient(void);

    TCPClient client;

    char buffer[HTTP_CLIENT_BUFFER_SIZE];

    /**
    * HTTP request methods.
    * Can't use 'delete' as name since it's a C++ keyword.
    */
    void get(http_request_t &aRequest, http_response_t &aResponse)
    {
        request(aRequest, aResponse, NULL, HTTP_METHOD_GET);
    }

    void post(http_request_t &aRequest, http_response_t &aResponse)
    {
        request(aRequest, aResponse, NULL, HTTP_METHOD_POST);
    }

    void put(http_request_t &aRequest, http_response_t &aResponse)
    {
        request(aRequest, aResponse, NULL, HTTP_METHOD_PUT);
    }

    void del(http_request_t &aRequest, http_response_t &aResponse)
    {
        request(aRequest, aResponse, NULL, HTTP_METHOD_DELETE);
    }

    void get(http_request_t &aRequest, http_response_t &aResponse, http_header_t headers[])
    {
        request(aRequest, aResponse, headers, HTTP_METHOD_GET);
    }

    void post(http_request_t &aRequest, http_response_t &aResponse, http_header_t headers[])
    {
        request(aRequest, aResponse, headers, HTTP_METHOD_POST);
    }

    void put(http_request_t &aRequest, http_response_t &aResponse, http_header_t headers[])
    {
        request(aRequest, aResponse, headers, HTTP_METHOD_PUT);
    }

    void del(http_request_t &aRequest, http_response_t &aResponse, http_header_t headers[])
    {
        request(aRequest, aResponse, headers, HTTP_METHOD_DELETE);
    }

  private:
    void request(http_request_t &aRequest, http_response_t &aResponse, http_header_t headers[], const char* aHttpMethod);
    void sendHeader(const char* aHeaderName, const char* aHeaderValue);
    void sendHeader(const char* aHeaderName, const int aHeaderValue);
    void sendHeader(const char* aHeaderName);
    void sendHeader(const char* aHeaderName, const Printable& aHeaderValue);
};

#endif /* __HTTP_CLIENT_H_ */
