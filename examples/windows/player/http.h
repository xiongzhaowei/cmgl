#include <sdkddkver.h>
#include <Windows.h>
#include <winhttp.h>
#include "CMGL.h"

using namespace wheel;
using namespace wheel::ffmpeg;

class WinHTTPWebsite;
class WinHTTPConnection;
class WinHTTPResponse;

class WinHTTPSession : public Object {
    HINTERNET m_hSession = nullptr;
public:
    WinHTTPSession();
    ~WinHTTPSession();

    RefPtr<WinHTTPWebsite> open(std::wstring host, uint16_t port = 0);
    bool setProxy(std::wstring host, uint16_t port);
};

class WinHTTPWebsite : public Object {
    HINTERNET m_hConnect = nullptr;
public:
    WinHTTPWebsite(HINTERNET hConnect);
    ~WinHTTPWebsite();

    RefPtr<Future<WinHTTPResponse>> open(bool bSecure, LPCWSTR lpszMethod, LPCWSTR lpszPath, LPCWSTR lpszHeaders, LPVOID lpBody, DWORD dwBodyLength, RefPtr<Thread> thread);
};

class WinHTTPRequest : public Object {
    bool _secure = false;
    std::wstring _method = L"GET";
    std::wstring _host;
    std::uint16_t _port = 80;
    std::wstring _path;
    std::map<std::wstring, std::wstring> _headers;
    std::vector<uint8_t> _body;
public:
    virtual bool secure() const;
    virtual std::wstring method() const;
    virtual std::wstring host() const;
    virtual uint16_t port() const;
    virtual std::wstring path() const;
    virtual std::wstring headers() const;
    virtual std::wstring header(const std::wstring& name) const;
    virtual std::vector<uint8_t> body() const;
    virtual void setSecure(bool secure);
    virtual void setMethod(const std::wstring& method);
    virtual void setHost(const std::wstring& host);
    virtual void setPort(std::uint16_t port);
    virtual void setPath(const std::wstring& path);
    virtual void setHeader(const std::wstring& name, const std::wstring& value);
    virtual void setBody(const std::vector<uint8_t>& body);

    RefPtr<Future<WinHTTPResponse>> open(RefPtr<WinHTTPSession> session, RefPtr<Thread> thread);
    RefPtr<Future<WinHTTPResponse>> open(RefPtr<WinHTTPWebsite> website, RefPtr<Thread> thread);
};

class WinHTTPResponse : public Stream<std::initializer_list<uint8_t>> {
    RefPtr<WinHTTPConnection> _connection;
    RefPtr<StreamController<std::initializer_list<uint8_t>>> _controller;
    std::wstring _protocol;
    std::wstring _status;
    std::wstring _message;
    std::map<std::wstring, std::wstring> _headers;
public:
    WinHTTPResponse(RefPtr<WinHTTPConnection> connection, std::wstring protocol, std::wstring status, std::wstring message);
    ~WinHTTPResponse();

    virtual std::wstring protocol() const;
    virtual std::wstring status() const;
    virtual std::wstring message() const;
    virtual std::wstring header(const std::wstring& name) const;
    virtual void setHeader(const std::wstring& name, const std::wstring& value);

    RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<std::initializer_list<uint8_t>>> consumer) override;
    void close();
};

class WinHTTPConnection : public Object {
    RefPtr<WinHTTPWebsite> m_pWebsite;
    RefPtr<StreamController<std::initializer_list<uint8_t>>> m_pStreamController;
    RefPtr<Completer<WinHTTPResponse>> m_pCompleter;
    HINTERNET m_hRequest = nullptr;
    LPVOID m_pReadBuffer = nullptr;
    DWORD m_dwReadBufferSize = 0;
    LPVOID m_pWriteBuffer = nullptr;
    DWORD m_dwWriteBufferSize = 0;
    bool m_bCancelled = false;
public:
    WinHTTPConnection(WinHTTPWebsite* pWebsite, HINTERNET hRequest, LPVOID lpBody, DWORD dwBodyLength);
    ~WinHTTPConnection();
    RefPtr<Future<WinHTTPResponse>> send(LPCWSTR lpszHeaders, DWORD dwHeadersLength, RefPtr<Thread> thread);
    void cancel();
    DWORD onCallback(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
    void onResponse(RefPtr<WinHTTPResponse> pResponse);
    void onReceived(const LPVOID pBuffer, DWORD bytesRead);
    void onFinished();
    void onError(DWORD dwErrorCode);
};

class WinHTTPVirtualFileBlock : public StreamConsumer<std::initializer_list<uint8_t>> {
    RefPtr<WinHTTPResponse> _response;
    RefPtr<MovieFile> _cache;
    std::int64_t _offset = 0;
    std::int64_t _received = 0;
    std::optional<int64_t> _size = std::nullopt;
public:
    WinHTTPVirtualFileBlock(RefPtr<WinHTTPResponse> response, RefPtr<MovieFile> cache, std::int64_t offset) : _response(response), _cache(cache), _offset(offset) {}
    void add(std::initializer_list<uint8_t> bytes) {
        _cache->seek(_offset + _received, SEEK_SET);
        _cache->write(bytes.begin(), (DWORD)bytes.size());
        _received += bytes.size();
        if (_size.has_value() && _received >= _size.value()) {
            close();
        }
    }
    void addError() {
        close();
    }
    void close() {
        if (_response) {
            _response->close();
            _response = nullptr;
        }
    }
    bool available() const {
        return true;
    }
    bool contains(int64_t offset) const {
        if (_offset < offset) return false;
        offset -= _offset;
        if (!_size.has_value()) return true;
        return offset < _size.value();
    }
    void setEndOfRange(int64_t offset) {
        if (_offset > offset) {
            int64_t size = offset - _offset;
            if (!_size.has_value() || _size.value() > size) {
                _size = size;
            }
        }
    }
};

class WinHTTPVirtualFile : public MovieFile {
    RefPtr<MovieFile> _cache;
    RefPtr<Thread> _thread;
    RefPtr<WinHTTPWebsite> _website;
    RefPtr<WinHTTPRequest> _request;
    size_t _bufferSize;
    std::unique_ptr<uint8_t> _buffer = std::unique_ptr<uint8_t>(new uint8_t[_bufferSize]);
    int64_t _fileSize = -1;
    std::vector<RefPtr<WinHTTPVirtualFileBlock>> _blocks;
public:
    WinHTTPVirtualFile(RefPtr<MovieFile> cache, RefPtr<Thread> thread, RefPtr<WinHTTPWebsite> website, RefPtr<WinHTTPRequest> request, size_t bufferSize) : _cache(cache), _thread(thread), _website(website), _request(request), _bufferSize(bufferSize) {

    }
    size_t bufferSize() const override { return _bufferSize; }
    int read(uint8_t* buf, int buf_size) override {
        return 0;
    }
    int write(const uint8_t* buf, int buf_size) override {
        return 0;
    }
    int64_t seek(int64_t offset, int whence) override {
        return 0;
    }
    void createBlock(int64_t offset) {
        for (auto block : _blocks) {
            if (block->contains(offset)) {
                block->setEndOfRange(offset);
                break;
            }
        }
        _request->setHeader(L"Range", L"bytes=" + std::to_wstring(offset) + L"-");
        _request->open(_website, _thread)->then([self = RefPtr<WinHTTPVirtualFile>(this), offset](RefPtr<WinHTTPResponse> response) {
            RefPtr<WinHTTPVirtualFileBlock> block = new WinHTTPVirtualFileBlock(response, self->_cache, offset);
            response->listen(block);
            self->_blocks.push_back(block);
        });
    }
};
