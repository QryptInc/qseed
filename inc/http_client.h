#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <vector>

/// <summary>
/// HTTP request information
/// </summary>
struct HttpRequest {

    /// <summary>
    /// The Fully Qualified Domain Name (FQDN) with prepended protocol
    /// </summary>
    std::string fqdn;

    /// <summary>
    /// The HTTP method to use (ex. "GET" or "POST")
    /// </summary>
    std::string method;

    /// <summary>
    /// The HTTP path or route
    /// </summary>
    std::string path;

    /// <summary>
    /// The HTTP list of headers
    /// </summary>
    std::vector<std::string> headers;

    /// <summary>
    /// The HTTP body
    /// </summary>
    std::string body;

};

/// <summary>
/// HTTP response information
/// </summary>
struct HttpResponse {

    /// <summary>
    /// HTTP status code
    /// </summary>
    int httpCode;

    /// <summary>
    /// Response body, as a string
    /// </summary>
    std::string body;

};

/// <summary>
/// Interface to interact with HTTP clients
/// </summary>
class HttpClient {

  public:

    HttpClient() {}
    virtual ~HttpClient() {}

    /// <summary>
    /// Sends an HTTP request using a given configuration object. 
    /// HttpClientError must be raised for any error cases.
    /// </summary>
    ///
    /// <param name="HttpRequest">The HTTP request arguments</param>
    /// <returns>HTTP response</returns>
    virtual HttpResponse send(const HttpRequest &httpRequest) = 0;

};

/// <summary>
/// An implementation of the HttpClient interface using libcurl
/// </summary>
class CurlClient : public HttpClient {

  public:

    CurlClient();
    virtual ~CurlClient();
    CurlClient(const CurlClient &) = delete;
    CurlClient &operator=(const CurlClient &) = delete;

    /// <summary>
    /// Call libcurl curl_easy_perform
    /// </summary>
    ///
    /// <param name="curlRequest">The CURL request arguments</param>
    /// <returns>CURL response</returns>
    HttpResponse send(const HttpRequest &httpRequest) override;

};

#endif