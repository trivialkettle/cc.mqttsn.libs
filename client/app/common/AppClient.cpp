//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AppClient.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <type_traits>

namespace cc_mqttsn_client_app
{

namespace 
{

AppClient* asThis(void* data)
{
    return reinterpret_cast<AppClient*>(data);
}

// std::string toString(CC_MqttsnQoS val)
// {
//     static const std::string Map[] = {
//         /* CC_MqttsnQoS_AtMostOnceDelivery */ "QoS0 - At Most Once Delivery",
//         /* CC_MqttsnQoS_AtLeastOnceDelivery */ "QoS1 - At Least Once Delivery",
//         /* CC_MqttsnQoS_ExactlyOnceDelivery */ "QoS2 - Exactly Once Delivery",
//     };
//     static constexpr std::size_t MapSize = std::extent<decltype(Map)>::value;
//     static_assert(MapSize == CC_MqttsnQoS_ValuesLimit);

//     auto idx = static_cast<unsigned>(val);
//     if (MapSize <= idx) {
//         assert(false); // Should not happen
//         return std::to_string(val);
//     }

//     return Map[idx] + " (" + std::to_string(val) + ')';
// }

// void printQos(const char* prefix, CC_MqttsnQoS val)
// {
//     std::cout << "\t" << prefix << ": " << toString(val) << '\n';
// }

// void printBool(const char* prefix, bool val)
// {
//     std::cout << '\t' << prefix << ": " << std::boolalpha << val << '\n';
// }

// void printConnectReturnCode(CC_MqttsnConnectReturnCode val)
// {
//     std::cout << "\tReturn Code: " << AppClient::toString(val) << '\n';
// }

// void printSubscribeReturnCode(CC_MqttsnSubscribeReturnCode val)
// {
//     std::cout << "\tReturn Code: " << AppClient::toString(val) << '\n';
// }

} // namespace 
    
bool AppClient::start(int argc, const char* argv[])
{
    if (!m_opts.parseArgs(argc, argv)) {
        logError() << "Failed to parse arguments." << std::endl;
        return false;
    }

    if (m_opts.helpRequested()) {
        std::cout << "Usage: " << argv[0] << " [options...]" << '\n';
        m_opts.printHelp();
        io().stop();
        return true;
    }

    if (!createSession()) {
        return false;
    }

    return startImpl();
}   


std::string AppClient::toString(CC_MqttsnErrorCode val)
{
    static const std::string Map[] = {
        /* CC_MqttsnErrorCode_Success*/ "Success",
        /* CC_MqttsnErrorCode_InternalError */ "Internal Error",
        /* CC_MqttsnErrorCode_NotIntitialized */ "Not Intitialized",
        /* CC_MqttsnErrorCode_Busy*/ "Busy",
        /* CC_MqttsnErrorCode_NotConnected*/ "Not Connected",
        /* CC_MqttsnErrorCode_AlreadyConnected */ "Already Connected",
        /* CC_MqttsnErrorCode_BadParam*/ "Bad Param",
        /* CC_MqttsnErrorCode_InsufficientConfig*/ "Insufficient Config",
        /* CC_MqttsnErrorCode_OutOfMemory*/ "Out Of Memory",
        /* CC_MqttsnErrorCode_BufferOverflow*/ "Buffer Overflow",
        /* CC_MqttsnErrorCode_NotSupported*/ "Not Supported",
        /* CC_MqttsnErrorCode_RetryLater*/ "Retry Later",
        /* CC_MqttsnErrorCode_Disconnecting*/ "Disconnecting",
        /* CC_MqttsnErrorCode_NotSleeping*/ "Not sleeping",
        /* CC_MqttsnErrorCode_PreparationLocked*/ "Preparation Locked",
    };

    static constexpr std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnErrorCode_ValuesLimit);

    auto idx = static_cast<unsigned>(val);
    if (MapSize <= idx) {
        assert(false); // Should not happen
        return std::to_string(val);
    }

    return Map[idx] + " (" + std::to_string(val) + ')';
}

// std::string AppClient::toString(const std::uint8_t* data, unsigned dataLen, bool forceBinary)
// {
//     bool binary = forceBinary;
//     if (!binary) {
//         binary = 
//             std::any_of(
//                 data, data + dataLen,
//                 [](auto byte)
//                 {
//                     if (std::isprint(static_cast<int>(byte)) == 0) {
//                         return true;
//                     }

//                     if (byte > 0x7e) {
//                         return true;
//                     }

//                     return false;
//                 });
//     } 

//     if (!binary) {
//         return std::string(reinterpret_cast<const char*>(data), dataLen);
//     }

//     std::stringstream stream;
//     stream << std::hex;
//     for (auto idx = 0U; idx < dataLen; ++idx) {
//         stream << std::setw(2) << std::setfill('0') << static_cast<unsigned>(data[idx]) << ' ';
//     }
//     return stream.str();
// }

// void AppClient::print(const CC_MqttsnMessageInfo& info, bool printMessage)
// {
//     std::cout << "[INFO]: Message Info:\n";
//     if (printMessage) {
//         std::cout << 
//             "\tTopic: " << info.m_topic << '\n' <<
//             "\tData: " << toString(info.m_data, info.m_dataLen, m_opts.subBinary()) << '\n';
//     }

//     printQos("QoS", info.m_qos);
//     printBool("Retained", info.m_retained);
//     std::cout << std::endl;
// }

AppClient::AppClient(boost::asio::io_context& io, int& result) : 
    m_io(io),
    m_result(result),
    m_timer(io),
    m_client(::cc_mqttsn_client_alloc())
{
    assert(m_client);
    ::cc_mqttsn_client_set_send_output_data_callback(m_client.get(), &AppClient::sendDataCb, this);
    ::cc_mqttsn_client_set_message_report_callback(m_client.get(), &AppClient::messageReceivedCb, this);
    ::cc_mqttsn_client_set_error_log_callback(m_client.get(), &AppClient::logMessageCb, this);
    ::cc_mqttsn_client_set_next_tick_program_callback(m_client.get(), &AppClient::nextTickProgramCb, this);
    ::cc_mqttsn_client_set_cancel_next_tick_wait_callback(m_client.get(), &AppClient::cancelNextTickWaitCb, this);
}

std::ostream& AppClient::logError()
{
    return std::cerr << "ERROR: ";
}

void AppClient::doTerminate(int result)
{
    m_result = result;
    m_io.stop();
}

void AppClient::doComplete()
{
    // if (m_opts.willTopic().empty()) {
    //     auto ec = ::cc_mqttsn_client_disconnect(m_client.get());
    //     if (ec != CC_MqttsnErrorCode_Success) {
    //         logError() << "Failed to send disconnect with ec=" << toString(ec) << std::endl;
    //         doTerminate();
    //         return;
    //     }       
    // }

    boost::asio::post(
        m_io,
        [this]()
        {
            doTerminate(0);
        });
}

bool AppClient::startImpl()
{
    return true;
}


void AppClient::messageReceivedImpl([[maybe_unused]] const CC_MqttsnMessageInfo* info)
{
}

std::vector<std::uint8_t> AppClient::parseBinaryData(const std::string& val)
{
    std::vector<std::uint8_t> result;
    result.reserve(val.size());
    auto pos = 0U;
    while (pos < val.size()) {
        auto ch = val[pos];
        auto addChar = 
            [&result, &pos, ch]()
            {
                result.push_back(static_cast<std::uint8_t>(ch));
                ++pos;
            };

        if (ch != '\\') {
            addChar();
            continue;
        }

        auto nextPos = pos + 1U;
        if ((val.size() <= nextPos)) {
            addChar();
            continue;
        }

        auto nextChar = val[nextPos];
        if (nextChar == '\\') {
            // Double backslash (\\) is treated as single one
            addChar();
            ++pos;
            continue;
        }

        if (nextChar != 'x') {
            // Not hex byte prefix, treat backslash as regular character
            addChar();
            continue;
        }

        auto bytePos = nextPos + 1U;
        auto byteLen = 2U;
        if (val.size() < bytePos + byteLen) {
            // Bad hex byte encoding, add characters as is
            addChar();
            continue;
        }

        try {
            auto byte = static_cast<std::uint8_t>(stoul(val.substr(bytePos, byteLen), nullptr, 16));
            result.push_back(byte);
            pos = bytePos + byteLen;
            continue;
        }
        catch (...) {
            addChar();
            continue;
        }
    }

    return result;
}

std::string AppClient::toString(CC_MqttsnAsyncOpStatus val)
{
    static const std::string Map[] = {
        /* CC_MqttsnAsyncOpStatus_Complete */ "Complete",
        /* CC_MqttsnAsyncOpStatus_InternalError */ "Internal Error",
        /* CC_MqttsnAsyncOpStatus_Timeout */ "Timeout",
        /* CC_MqttsnAsyncOpStatus_Aborted */ "Aborted",
        /* CC_MqttsnAsyncOpStatus_OutOfMemory */ "Out of Memory",
        /* CC_MqttsnAsyncOpStatus_BadParam */ "Bad Param",
        /* CC_MqttsnAsyncOpStatus_GatewayDisconnected */ "Gateway Disconnected",
    };

    static constexpr std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnAsyncOpStatus_ValuesLimit);

    auto idx = static_cast<unsigned>(val);
    if (MapSize <= idx) {
        assert(false); // Should not happen
        return std::to_string(val);
    }

    return Map[idx] + " (" + std::to_string(val) + ')';
}

void AppClient::nextTickProgramInternal(unsigned duration)
{
    m_lastWaitProgram = Clock::now();
    m_timer.expires_after(std::chrono::milliseconds(duration));
    m_timer.async_wait(
        [this, duration](const boost::system::error_code& ec)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (ec) {
                logError() << "Timer error: " << ec.message();
                doTerminate();
                return;
            }

            ::cc_mqttsn_client_tick(m_client.get(), duration);
        }
    );
}

unsigned AppClient::cancelNextTickWaitInternal()
{
    boost::system::error_code ec;
    m_timer.cancel(ec);
    auto now = Clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastWaitProgram).count();
    return static_cast<unsigned>(diff);
}

void AppClient::sendDataInternal(const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius)
{
    assert(m_session);
    m_session->sendData(buf, bufLen, broadcastRadius);
}

bool AppClient::createSession()
{
    m_session = Session::create(m_io, m_opts);
    if (!m_session) {
        logError() << "Failed to create network connection session." << std::endl;
        return false;
    }

    m_session->setDataReportCb(
        [this](const Addr& addr, const std::uint8_t* buf, std::size_t bufLen)
        {
            assert(m_client);
            m_lastAddr = addr;
            ::cc_mqttsn_client_process_data(m_client.get(), buf, static_cast<unsigned>(bufLen));
        });

    m_session->setNetworkErrorReportCb(
        [this]()
        {
            assert(m_client);
            doTerminate();
        }
    );

    if (!m_session->start()) {
        logError() << "Failed to start session." << std::endl;
        return false;
    }

    return true;
}

void AppClient::sendDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius)
{
    asThis(data)->sendDataInternal(buf, bufLen, broadcastRadius);
}

void AppClient::messageReceivedCb(void* data, const CC_MqttsnMessageInfo* info)
{
    asThis(data)->messageReceivedImpl(info);
}

void AppClient::logMessageCb([[maybe_unused]] void* data, const char* msg)
{
    logError() << msg << std::endl;
}

void AppClient::nextTickProgramCb(void* data, unsigned duration)
{
    asThis(data)->nextTickProgramInternal(duration);
}

unsigned AppClient::cancelNextTickWaitCb(void* data)
{
    return asThis(data)->cancelNextTickWaitInternal();
}

} // namespace cc_mqttsn_client_app
