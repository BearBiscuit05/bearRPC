#include "bearTcp.h"

namespace
{
const size_t kHighWatermark = 65536;
const size_t kMaxMessageLen = 100 * 1024 * 1024;
}

template <typename ProtocolServer>
BaseServer<ProtocolServer>::BaseServer(EventLoop *loop, const InetAddress& listen)
    :server_(loop, listen)
{
    server_.setConnectionCallback(std::bind(
            &BaseServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(
            &BaseServer::onMessage, this, _1, _2));
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        DEBUG("connection %s is [up]",
              conn->peer().toIpPort().c_str());
        conn->setHighWaterMarkCallback(std::bind(
                &BaseServer::onHighWatermark, this, _1, _2), kHighWatermark);
    }
    else {
        DEBUG("connection %s is [down]",
              conn->peer().toIpPort().c_str());
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    handleMessage(conn, buffer);
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onHighWatermark(const TcpConnectionPtr& conn, size_t mark)
{
    DEBUG("connection %s high watermark %lu",
         conn->peer().toIpPort().c_str(), mark);

    conn->setWriteCompleteCallback(std::bind(
            &BaseServer::onWriteComplete, this, _1));
    conn->stopRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onWriteComplete(const TcpConnectionPtr& conn)
{
    DEBUG("connection %s write complete",
         conn->peer().toIpPort().c_str());
    conn->startRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::handleMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    while (true) {

        const char *crlf = buffer.findCRLF();
        if (crlf == nullptr)
            break;
        if (crlf == buffer.peek()) {
            buffer.retrieve(2);
            break;
        }

        size_t headerLen = crlf - buffer.peek() + 2;

        json::Document header;
        auto err = header.parse(buffer.peek(), headerLen);
        if (err != json::PARSE_OK ||
            !header.isInt32() ||
            header.getInt32() <= 0)
        {
            throw RequestException(RPC_INVALID_REQUEST, "invalid message length");
        }

        auto jsonLen = static_cast<uint32_t>(header.getInt32());
        if (jsonLen >= kMaxMessageLen)
            throw RequestException(RPC_INVALID_REQUEST, "message is too long");

        if (buffer.readableBytes() < headerLen + jsonLen)
            break;

        buffer.retrieve(headerLen);
        auto json = buffer.retrieveAsString(jsonLen);
        convert().handleRequest(json, [conn, this](json::Value response) {
            if (!response.isNull()) {
                sendResponse(conn, response);
                TRACE("BaseServer::handleMessage() %s request success",
                      conn->peer().toIpPort().c_str());
            }
            else {
                TRACE("BaseServer::handleMessage() %s notify success",
                      conn->peer().toIpPort().c_str());
            }
        });
    }
}

template <typename ProtocolServer>
json::Value BaseServer<ProtocolServer>::wrapException(RequestException& e)
{
    json::Value response(json::TYPE_OBJECT);
    response.addMember("jsonrpc", "2.0");
    auto& value = response.addMember("error", json::TYPE_OBJECT);
    value.addMember("code", e.err().asCode());
    value.addMember("message", e.err().asString());
    value.addMember("data", e.detail());
    response.addMember("id", e.id());
    return response;
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::sendResponse(const TcpConnectionPtr& conn, const json::Value& response)
{
    // 序列化 将json转移为string
    json::StringWriteStream os;
    json::Writer writer(os);
    response.writeTo(writer);

    // wish sso string don't allocate heap memory...
    auto message = std::to_string(os.get().length() + 2)
            .append("\r\n")
            .append(os.get())
            .append("\r\n");
    conn->send(message);
}

template <typename ProtocolServer>
ProtocolServer& BaseServer<ProtocolServer>::convert()
{
    return static_cast<ProtocolServer&>(*this);
}

template <typename ProtocolServer>
const ProtocolServer& BaseServer<ProtocolServer>::convert() const
{
    return static_cast<const ProtocolServer&>(*this);
}


// =================[RpcService]===================
void RpcService::callProcedureReturn(std::string_view methodName,
                                     json::Value& request,
                                     const RpcDoneCallback& done)
{
    auto it = procedureReturn_.find(methodName);
    if (it == procedureReturn_.end()) {
        throw RequestException(RPC_METHOD_NOT_FOUND,
                               request["id"],
                               "method not found");
    }
    it->second->invoke(request, done);
};

void RpcService::callProcedureNotify(std::string_view methodName, json::Value& request)
{
    auto it = procedureNotfiy_.find(methodName);
    if (it == procedureNotfiy_.end()) {
        throw NotifyException(RPC_METHOD_NOT_FOUND,
                              "method not found");
    }
    it->second->invoke(request);
};


// =================[RpcServer]===================
// ================="RpcServer.utils"=================
template <json::ValueType dst, json::ValueType... rest>
void checkValueType(json::ValueType type)
{
    if (dst == type)
        return;
    if constexpr (sizeof...(rest) > 0)
        checkValueType<rest...>(type);
    else
        throw RequestException(RPC_INVALID_REQUEST, "bad type of at least one field");
}

template <json::ValueType... types>
void checkValueType(json::ValueType type, json::Value& id)
{
    // wrap exception
    try {
        checkValueType<types...>(type);
    }
    catch (RequestException& e) {
        throw RequestException(e.err(), id, e.detail());
    }
}

template <json::ValueType... types>
json::Value& findValue(json::Value &request, const char *key)
{
    static_assert(sizeof...(types) > 0, "expect at least one type");

    auto it = request.findMember(key);
    if (it == request.memberEnd())
        throw RequestException(RPC_INVALID_REQUEST, "missing at least one field");
    checkValueType<types...>(it->value.getType());
    return it->value;
}

template <json::ValueType... types>
json::Value& findValue(json::Value& request, json::Value& id, const char *key)
{
    // wrap exception
    try {
        return findValue<types...>(request, key);
    }
    catch (RequestException &e) {
        throw RequestException(e.err(), id, e.detail());
    }
}

bool isNotify(const json::Value& request)
{
    return request.findMember("id") == request.memberEnd();
}

bool hasParams(const json::Value& request)
{
    return request.findMember("params") != request.memberEnd();
}

class ThreadSafeBatchResponse
{
public:
    explicit ThreadSafeBatchResponse(const RpcCallback& done):
            data_(std::make_shared<ThreadSafeData>(done)){}
    
    void addResponse(json::Value response)
    {
        std::unique_lock lock(data_->mutex);
        data_->responses.addValue(response);
    }

private:
    struct ThreadSafeData
    {
        explicit
        ThreadSafeData(const RpcCallback& done_):
            responses(json::TYPE_ARRAY),done(done_){}

        ~ThreadSafeData()
        {
            // last reference to data is destructing, so notify RPC server we are done
            done(responses);
        }

        std::mutex mutex;
        json::Value responses;
        RpcCallback done;
    };

    typedef std::shared_ptr<ThreadSafeData> DataPtr;
    DataPtr data_;
}

// ================="RpcServer.class"=================
void RpcServer::addService(std::string_view serviceName, RpcService *service)
{
    assert(services_.find(serviceName) == services_.end());
    services_.emplace(serviceName, service);
}

// step2 序列化
void RpcServer::handleRequest(const std::string& json,
                              const RpcCallback& done)
{
    json::Document request;
    json::ParseError err = request.parse(json);
    if (err != json::PARSE_OK)
        throw RequestException(RPC_PARSE_ERROR, json::parseErrorStr(err));

    switch (request.getType()) {
        case json::TYPE_OBJECT:
            if (isNotify(request))
                handleSingleNotify(request);
            else
                handleSingleRequest(request, done);
            break;
        case json::TYPE_ARRAY:
            handleBatchRequests(request, done);
            break;
        default:
            throw RequestException(RPC_INVALID_REQUEST, "request should be json object or array");
    }
}

void RpcServer::handleSingleRequest(json::Value& request,
                                    const RpcCallback& done)
{
    validateRequest(request);

    auto& id = request["id"];
    auto methodName = request["method"].getStringView();
    auto pos = methodName.find('.');
    if (pos == std::string_view::npos)
        throw RequestException(RPC_INVALID_REQUEST, id, "missing service name in method");

    auto serviceName = methodName.substr(0, pos);
    auto it = services_.find(serviceName);
    if (it == services_.end())
        throw RequestException(RPC_METHOD_NOT_FOUND, id, "service not found");

    // skip service name and '.'
    methodName.remove_prefix(pos + 1);
    if (methodName.length() == 0)
        throw RequestException(RPC_INVALID_REQUEST, id, "missing method name in method");

    auto& service = it->second;
    service->callProcedureReturn(methodName, request, done);
}

void RpcServer::handleBatchRequests(json::Value& requests,
                                    const RpcCallback& done)
{
    size_t num = requests.getSize();
    if (num == 0)
        throw RequestException(RPC_INVALID_REQUEST, "batch request is empty");
    
    ThreadSafeBatchResponse responses(done);

    try {
        size_t n = requests.getSize();
        for (size_t i = 0; i < n; i++) {

            auto& request = requests[i];

            if (!request.isObject()) {
                throw RequestException(RPC_INVALID_REQUEST, "request should be json object");
            }

            if (isNotify(request)) {
                handleSingleNotify(request);
            }
            else {
                handleSingleRequest(request, [=](json::Value response) mutable {
                    responses.addResponse(response);
                });
            }
        }
    }
    catch (RequestException &e) {
        auto response = wrapException(e);
        responses.addResponse(response);
    }
    catch (NotifyException &e) {
        // todo: print something here
    }
}

void RpcServer::handleSingleNotify(json::Value& request)
{
    validateNotify(request);

    auto methodName = request["method"].getStringView();
    auto pos = methodName.find('.');
    if (pos == std::string_view::npos || pos == 0)
        throw NotifyException(RPC_INVALID_REQUEST, "missing service name in method");

    auto serviceName = methodName.substr(0, pos);
    auto it = services_.find(serviceName);
    if (it == services_.end())
        throw RequestException(RPC_METHOD_NOT_FOUND, "service not found");

    // skip service name and '.'
    methodName.remove_prefix(pos + 1);
    if (methodName.length() == 0)
        throw RequestException(RPC_INVALID_REQUEST, "missing method name in method");

    auto& service = it->second;
    service->callProcedureNotify(methodName, request);
}

void RpcServer::validateRequest(json::Value& request)
{
    auto& id = findValue<
            json::TYPE_STRING,
            json::TYPE_NULL, // fixme: null id is evil
            json::TYPE_INT32,
            json::TYPE_INT64>(request, "id");

    auto& version = findValue<json::TYPE_STRING>(request, id, "jsonrpc");
    if (version.getStringView() != "2.0")
        throw RequestException(RPC_INVALID_REQUEST,
                               id, "jsonrpc version is unknown/unsupported");

    auto& method = findValue<json::TYPE_STRING>(request, id, "method");
    if (method.getStringView() == "rpc.") // internal use
        throw RequestException(RPC_METHOD_NOT_FOUND,
                               id, "method name is internal use");

    // jsonrpc, method, id, params
    size_t nMembers = 3u + hasParams(request);

    if (request.getSize() != nMembers)
        throw RequestException(RPC_INVALID_REQUEST, id, "unexpected field");
}

void RpcServer::validateNotify(json::Value& request)
{
    auto& version = findValue<json::TYPE_STRING>(request, "jsonrpc");
    if (version.getStringView() != "2.0")
        throw NotifyException(RPC_INVALID_REQUEST, "jsonrpc version is unknown/unsupported");

    auto& method = findValue<json::TYPE_STRING>(request, "method");
    if (method.getStringView() == "rpc.") // internal use
        throw NotifyException(RPC_METHOD_NOT_FOUND, "method name is internal use");

    // jsonrpc, method, params, no id
    size_t nMembers = 2u + hasParams(request);

    if (request.getSize() != nMembers)
        throw NotifyException(RPC_INVALID_REQUEST, "unexpected field");
}


// ================="Procedure.class"=================