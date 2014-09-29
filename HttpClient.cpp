#include "HttpClient.h"

#define LOG_REQUESTS
//#define LOG_HEADERS
//#define LOG_RESPONSE
#define LOG_ERRORS

static const uint16_t TIMEOUT = 5000; // Allow maximum 5s between data packets.

HttpClient::HttpClient() {}

/**
* Method to send a header, should only be called from within the class.
*/
void HttpClient::sendHeader(const char* aHeaderName, const Printable& aHeaderValue)
{
    client.print(aHeaderName);
    client.print(": ");
    client.println(aHeaderValue);

    #ifdef LOG_HEADERS
    Serial.print(aHeaderName);
    Serial.print(": ");
    Serial.println(aHeaderValue);
    #endif
}

void HttpClient::sendHeader(const char* aHeaderName, const char* aHeaderValue)
{
    client.print(aHeaderName);
    client.print(": ");
    client.println(aHeaderValue);

    #ifdef LOG_HEADERS
    Serial.print(aHeaderName);
    Serial.print(": ");
    Serial.println(aHeaderValue);
    #endif
}

void HttpClient::sendHeader(const char* aHeaderName, const int aHeaderValue)
{
    client.print(aHeaderName);
    client.print(": ");
    client.println(aHeaderValue);

    #ifdef LOG_HEADERS
    Serial.print(aHeaderName);
    Serial.print(": ");
    Serial.println(aHeaderValue);
    #endif
}

void HttpClient::sendHeader(const char* aHeaderName)
{
    client.println(aHeaderName);

    #ifdef LOG_HEADERS
    Serial.println(aHeaderName);
    #endif

}

/**
* Method to send an HTTP Request. Allocate variables in your application code
* in the aResponse struct and set the headers and the options in the aRequest
* struct.
*/
void HttpClient::request(http_request_t &aRequest, http_response_t &aResponse, http_header_t headers[], const char* aHttpMethod)
{
    size_t content_length = strlen(aRequest.body);

    #ifdef LOG_REQUESTS
    Serial.println();
    Serial.print(aHttpMethod);
    Serial.print(" | ");
    Serial.print(*(aRequest.hostname));
    Serial.print(":");
    Serial.print(aRequest.port);
    Serial.print(aRequest.path);
    Serial.print(" | ");
    Serial.print(headers[0].value);
    Serial.print(" | ");
    Serial.print(content_length);
    Serial.print(" | ");
    Serial.print(aRequest.body);
    #endif

    // If a proper response code isn't received it will be set to 0.
    aResponse.status = 0;

    // NOTE: The default port tertiary statement is unpredictable if the request structure is not initialised
    // http_request_t request = {0} or memset(&request, 0, sizeof(http_request_t)) should be used 
    // to ensure all fields are zero
    bool connected = client.connect(*(aRequest.hostname), (aRequest.port) ? aRequest.port : 80 );

    if (!connected)
    {
        client.stop();

#ifdef LOG_ERRORS
        Serial.println("HttpClient>\tConnection failed.");
#endif
        // If TCP Client can't connect to host, exit here.
        return;
    }

    //
    // Send HTTP Headers
    //

    // Send initial headers (only HTTP 1.0 is supported for now).
    client.print(aHttpMethod);
    client.print(" ");
    client.print(aRequest.path);
    client.print(" HTTP/1.0\r\n");

    // Send General and Request Headers.
    sendHeader("Connection", "close"); // Not supporting keep-alive for now.

    sendHeader("HOST", *(aRequest.hostname));

    //Send Entity Headers
    // TODO: Check the standard, currently sending Content-Length : 0 for empty
    // POST requests, and no content-length for other types.
    if (aRequest.body != NULL) {
        sendHeader("Content-Length", content_length);
    } else if (strcmp(aHttpMethod, HTTP_METHOD_POST) == 0) { //Check to see if its a Post method.
        sendHeader("Content-Length", 0);
    }

    if (headers != NULL)
    {
        int i = 0;
        while (headers[i].header != NULL)
        {
            if (headers[i].value != NULL) {
                sendHeader(headers[i].header, headers[i].value);
            } else {
                sendHeader(headers[i].header);
            }
            i++;
        }
    }

    // Empty line to finish headers
    client.println();
    client.flush();

    //
    // Send HTTP Request Body
    //

    if (aRequest.body != NULL) {
        client.println(aRequest.body);
        client.flush();
    }

    //
    // Receive HTTP Response
    //
    // The first value of client.available() might not represent the
    // whole response, so after the first chunk of data is received instead
    // of terminating the connection there is a delay and another attempt
    // to read data.
    // The loop exits when the connection is closed, or if there is a
    // timeout or an error.

    unsigned int bufferPosition = 0;
    unsigned long lastRead = millis();
    unsigned long firstRead = millis();
    bool error = false;
    bool timeout = false;

    // Zero out the buffer before we write to it. This ensures the response
    // is null terminated.
    memset(buffer, '\0', sizeof(buffer));

    #ifdef LOG_REQUESTS
      Serial.print(" | ");
    #endif

    do {

        #ifdef LOG_REQUESTS
          Serial.print("> ");
        #endif

        while (client.available())
        {
            char c = client.read();

            lastRead = millis();

            #ifdef LOG_RESPONSE
            Serial.print(c);
            #endif

            if (c == -1) {
                error = true;

                #ifdef LOG_ERRORS
                Serial.println("\nHttpClient>\tError: No data available.");
                #endif

                break;
            }

            // Check that received character fits in buffer before storing.
            if (bufferPosition < sizeof(buffer)-1)
            {
                buffer[bufferPosition] = c;
            } 
            else
            {
                error = true;

                #ifdef LOG_ERRORS
                Serial.println("\nHttpClient>\tError: Response body larger than buffer.");
                #endif
            }
            bufferPosition++;
        }

        // Check that there hasn't been more than 5s since last read.
        timeout = (millis() - lastRead) > TIMEOUT;

    } while (client.connected() && !timeout && !error);

    client.stop();

    #ifdef LOG_ERRORS
    if (timeout)
    {
        Serial.println("\nHttpClient>\tError: Timeout while reading response.");
    }

    Serial.print("| ");
    Serial.print(millis() - firstRead);
    Serial.print("ms");

    #endif

    char status_code_buffer[4] = {0};
    memcpy(&status_code_buffer, buffer + 9, 3);
    aResponse.status = atoi(status_code_buffer);

    #ifdef LOG_REQUESTS
    Serial.print(" | ");
    Serial.print(aResponse.status);
    Serial.print(aResponse.status == 200 ? " OK" : " !");
    #endif

    char* bodyPos = strstr(buffer, "\r\n\r\n");

    aResponse.body = bodyPos + strlen("\r\n\r\n");

}
