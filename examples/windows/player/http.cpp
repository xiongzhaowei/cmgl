#include "http.h"

#pragma comment(lib, "winhttp.lib")

WinHTTPResponse::WinHTTPResponse(RefPtr<WinHTTPConnection> connection, std::wstring protocol, std::wstring status, std::wstring message) : _connection(connection), _protocol(protocol), _status(status), _message(message), _controller(StreamController<std::initializer_list<uint8_t>>::sync()) {

}
WinHTTPResponse::~WinHTTPResponse() {

}

std::wstring WinHTTPResponse::protocol() const {
    return _protocol;
}
std::wstring WinHTTPResponse::status() const {
    return _status;
}
std::wstring WinHTTPResponse::message() const {
    return _message;
}
std::wstring WinHTTPResponse::header(const std::wstring& name) const {
    auto it = _headers.find(name);
    return it != _headers.end() ? it->second : L"";
}
void WinHTTPResponse::setHeader(const std::wstring& name, const std::wstring& value) {
    _headers[name] = value;
}
RefPtr<StreamSubscription> WinHTTPResponse::listen(RefPtr<StreamConsumer<std::initializer_list<uint8_t>>> consumer) {
    return _controller->stream()->listen(consumer);
}

void WinHTTPResponse::close() {
    _connection->cancel();
}

bool WinHTTPRequest::secure() const {
    return _secure;
}
std::wstring WinHTTPRequest::method() const {
    return _method;
}
std::wstring WinHTTPRequest::host() const {
    return _method;
}
uint16_t WinHTTPRequest::port() const {
    return _port;
}
std::wstring WinHTTPRequest::path() const {
    return _path;
}
std::wstring WinHTTPRequest::headers() const {
    std::wstring headers;
    for (auto it : _headers) {
        headers += it.first + L": " + it.second + L"\r\n";
    }
    return headers;
}
std::wstring WinHTTPRequest::header(const std::wstring& name) const {
    auto it = _headers.find(name);
    return it != _headers.end() ? it->second : L"";
}
std::vector<uint8_t> WinHTTPRequest::body() const {
    return _body;
}
void WinHTTPRequest::setSecure(bool secure) {
    _secure = secure;
}
void WinHTTPRequest::setMethod(const std::wstring& method) {
    _method = method;
}
void WinHTTPRequest::setHost(const std::wstring& host) {
    _host = host;
}
void WinHTTPRequest::setPort(std::uint16_t port) {
    _port = port;
}
void WinHTTPRequest::setPath(const std::wstring& path) {
    _path = path;
}
void WinHTTPRequest::setHeader(const std::wstring& name, const std::wstring& value) {
    _headers[name] = value;
}
void WinHTTPRequest::setBody(const std::vector<uint8_t>& body) {
    _body = body;
}
RefPtr<Future<WinHTTPResponse>> WinHTTPRequest::open(RefPtr<WinHTTPSession> session, RefPtr<Thread> thread) {
    return open(session->open(host().c_str(), port()), thread);
}
RefPtr<Future<WinHTTPResponse>> WinHTTPRequest::open(RefPtr<WinHTTPWebsite> website, RefPtr<Thread> thread) {
    std::vector<uint8_t> body = this->body();
    return website->open(secure(), method().c_str(), path().c_str(), headers().c_str(), body.data(), (DWORD)body.size(), thread);
}

WinHTTPSession::WinHTTPSession() {
    m_hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
}

WinHTTPSession::~WinHTTPSession() {
    WinHttpCloseHandle(m_hSession);
    m_hSession = nullptr;
}

bool WinHTTPSession::setProxy(std::wstring host, uint16_t port) {
    std::wstring uri = L"http://" + host + L":" + std::to_wstring(port);

    WINHTTP_PROXY_INFO proxy = { 0 };
    proxy.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
    proxy.lpszProxy = uri.data();

    return WinHttpSetOption(m_hSession, WINHTTP_OPTION_PROXY, &proxy, sizeof(proxy));
}

RefPtr<WinHTTPWebsite> WinHTTPSession::open(std::wstring host, uint16_t port) {
    return new WinHTTPWebsite(WinHttpConnect(m_hSession, host.c_str(), port, 0));
}

WinHTTPWebsite::WinHTTPWebsite(HINTERNET hConnect) : m_hConnect(hConnect) {
}

WinHTTPWebsite::~WinHTTPWebsite() {
    WinHttpCloseHandle(m_hConnect);
    m_hConnect = nullptr;
}

RefPtr<Future<WinHTTPResponse>> WinHTTPWebsite::open(bool bSecure, LPCWSTR lpszMethod, LPCWSTR lpszPath, LPCWSTR lpszHeaders, LPVOID lpBody, DWORD dwBodyLength, RefPtr<Thread> thread) {
    HINTERNET hRequest = WinHttpOpenRequest(m_hConnect, lpszMethod, lpszPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, bSecure ? WINHTTP_FLAG_SECURE : 0);
    if (hRequest == nullptr) return nullptr;

    RefPtr<WinHTTPConnection> task = new WinHTTPConnection(this, hRequest, lpBody, dwBodyLength);
    return task->send(lpszHeaders, lpszHeaders == nullptr ? 0 : (DWORD)wcslen(lpszHeaders), thread);
}

WinHTTPConnection::WinHTTPConnection(WinHTTPWebsite* pWebsite, HINTERNET hRequest, LPVOID lpBody, DWORD dwBodyLength) : m_pWebsite(pWebsite), m_hRequest(hRequest), m_pWriteBuffer(lpBody), m_dwWriteBufferSize(dwBodyLength) {
}

WinHTTPConnection::~WinHTTPConnection() {
    WinHttpCloseHandle(m_hRequest);
    m_hRequest = nullptr;

    if (m_pReadBuffer) {
        delete[] m_pReadBuffer;
        m_pReadBuffer = nullptr;
    }
    m_dwReadBufferSize = 0;

    m_pWriteBuffer = nullptr;
    m_dwWriteBufferSize = 0;
}

RefPtr<Future<WinHTTPResponse>> WinHTTPConnection::send(LPCWSTR lpszHeaders, DWORD dwHeadersLength, RefPtr<Thread> thread) {
    auto statusCallback = [](
        HINTERNET hInternet,
        DWORD_PTR dwContext,
        DWORD dwInternetStatus,
        LPVOID lpvStatusInformation,
        DWORD dwStatusInformationLength
    ) {
        if (0 != dwContext) {
            WinHTTPConnection* self = reinterpret_cast<WinHTTPConnection*>(dwContext);
            DWORD dwErrorCode = self->onCallback(dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
            if (dwErrorCode != NOERROR) self->onError(dwErrorCode);
        }
    };

    if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(m_hRequest, statusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0)) {
        return Future<WinHTTPResponse>::value(nullptr);
    }
    if (m_pWriteBuffer && m_dwWriteBufferSize) {
        std::wstring header = L"Content-Length: " + std::to_wstring(m_dwWriteBufferSize);
        if (!WinHttpAddRequestHeaders(m_hRequest, header.c_str(), (DWORD)header.size(), WINHTTP_ADDREQ_FLAG_REPLACE | WINHTTP_ADDREQ_FLAG_ADD)) {
            return Future<WinHTTPResponse>::value(nullptr);
        }
    }
    if (!WinHttpSendRequest(m_hRequest, lpszHeaders, dwHeadersLength, WINHTTP_NO_REQUEST_DATA, 0, 0, reinterpret_cast<DWORD_PTR>(this))) {
        return Future<WinHTTPResponse>::value(nullptr);
    }
    retain();
    m_pCompleter = Completer<WinHTTPResponse>::async(thread);
    return m_pCompleter->future();
}

void WinHTTPConnection::cancel() {
    m_bCancelled = true;
}

DWORD WinHTTPConnection::onCallback(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) {
    if (m_bCancelled) return ERROR_CANCELLED;

    switch (dwInternetStatus) {
    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
        if (m_pWriteBuffer && m_dwWriteBufferSize) {
            if (!WinHttpWriteData(m_hRequest, m_pWriteBuffer, m_dwWriteBufferSize, NULL)) {
                return GetLastError();
            }
        }
        // NOTE: no break here
    case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
        if (!WinHttpReceiveResponse(m_hRequest, NULL)) {
            return GetLastError();
        }
        break;
    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
        DWORD dwBufferSize = 0;
        if (!WinHttpQueryHeaders(m_hRequest,
            WINHTTP_QUERY_RAW_HEADERS,
            WINHTTP_HEADER_NAME_BY_INDEX,
            NULL,
            &dwBufferSize,
            WINHTTP_NO_HEADER_INDEX
        )) {
            DWORD dwErrorCode = GetLastError();
            if (dwErrorCode != ERROR_INSUFFICIENT_BUFFER) {
                return dwErrorCode;
            }
        }

        RefPtr<WinHTTPResponse> pResponse;
        LPWSTR pszHeaders = new WCHAR[dwBufferSize];
        if (WinHttpQueryHeaders(m_hRequest,
            WINHTTP_QUERY_RAW_HEADERS,
            WINHTTP_HEADER_NAME_BY_INDEX,
            pszHeaders,
            &dwBufferSize,
            WINHTTP_NO_HEADER_INDEX
        )) {
            LPWSTR pHeader = pszHeaders;
            LPWSTR pProtocol = pHeader;

            for (; *pHeader && *pHeader != ' '; pHeader++);
            if (*pHeader == ' ') {
                *pHeader++ = 0;
                
                LPWSTR pStatus = pHeader;
                for (; *pHeader && *pHeader != ' '; pHeader++);
                if (*pHeader == ' ') {
                    *pHeader++ = 0;
                }
                pResponse = new WinHTTPResponse(this, pProtocol, pStatus, pHeader);
            } else {
                return ERROR_WINHTTP_INVALID_SERVER_RESPONSE;
            }
            while (*pHeader++);
            while (*pHeader) {
                LPCWSTR pName = pHeader;
                for (; *pHeader && *pHeader != ':'; pHeader++);
                if (*pHeader == ':') {
                    *pHeader++ = 0;
                    for (; *pHeader && *pHeader == ' '; pHeader++);
                    pResponse->setHeader(pName, pHeader);
                }
                while (*pHeader++);
            }
            delete[] pszHeaders;
        } else {
            delete[] pszHeaders;
            return GetLastError();
        }

        if (!WinHttpQueryDataAvailable(m_hRequest, NULL)) {
            return GetLastError();
        }

        onResponse(pResponse);
        break;
    }
    case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
        m_dwReadBufferSize = 8192;
        m_pReadBuffer = new BYTE[m_dwReadBufferSize];

        if (!WinHttpReadData(m_hRequest, m_pReadBuffer, m_dwReadBufferSize, NULL)) {
            return GetLastError();
        }

        break;
    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        if (0 < dwStatusInformationLength) {
            onReceived(m_pReadBuffer, dwStatusInformationLength);

            if (!WinHttpReadData(m_hRequest, m_pReadBuffer, m_dwReadBufferSize, NULL)) {
                return GetLastError();
            }
        } else {
            onFinished();
        }

        break;
    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
        WINHTTP_ASYNC_RESULT* asyncResult = static_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);
        onError(asyncResult->dwError);
    }
    default:
        return NOERROR;
    }
    return NOERROR;
}

void WinHTTPConnection::onResponse(RefPtr<WinHTTPResponse> pResponse) {
    if (m_pCompleter && !m_pCompleter->isCompleted()) m_pCompleter->complete(pResponse);
}

void WinHTTPConnection::onReceived(const LPVOID pBuffer, DWORD bytesRead) {
    if (m_pStreamController) {
        m_pStreamController->add(std::initializer_list<uint8_t>((uint8_t*)pBuffer, (uint8_t*)pBuffer + bytesRead));
        m_pStreamController = nullptr;
    }
}

void WinHTTPConnection::onFinished() {
    if (m_pStreamController) {
        m_pStreamController->close();
        m_pStreamController = nullptr;
    }
    WinHttpSetStatusCallback(m_hRequest, NULL, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
    release();
}

void WinHTTPConnection::onError(DWORD dwErrorCode) {
    if (m_pCompleter && m_pCompleter->isCompleted()) m_pCompleter->complete(nullptr);
    if (m_pStreamController) {
        m_pStreamController->addError();
        m_pStreamController->close();
        m_pStreamController = nullptr;
    }
    WinHttpSetStatusCallback(m_hRequest, NULL, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
    release();
}
