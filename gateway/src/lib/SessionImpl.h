//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <limits>

#include "cc_mqttsn_gateway/Session.h"
#include "MsgHandler.h"
#include "SessionOp.h"
#include "common.h"
#include "comms/util/ScopeGuard.h"

namespace cc_mqttsn_gateway
{

class SessionImpl : public MsgHandler
{
    typedef MsgHandler Base;
public:
    typedef Session::AuthInfo AuthInfo;

    typedef Session::NextTickProgramReqCb NextTickProgramReqCb;
    typedef Session::SendDataReqCb SendDataReqCb;
    typedef Session::CancelTickWaitReqCb CancelTickWaitReqCb;
    typedef Session::TerminationReqCb TerminationReqCb;
    typedef Session::BrokerReconnectReqCb BrokerReconnectReqCb;
    typedef Session::ClientConnectedReportCb ClientConnectedReportCb;
    typedef Session::AuthInfoReqCb AuthInfoReqCb;

    SessionImpl();
    ~SessionImpl() = default;


    template <typename TFunc>
    void setNextTickProgramReqCb(TFunc&& func)
    {
        m_nextTickProgramCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setCancelTickWaitReqCb(TFunc&& func)
    {
        m_cancelTickCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSendDataClientReqCb(TFunc&& func)
    {
        m_sendToClientCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSendDataBrokerReqCb(TFunc&& func)
    {
        m_sendToBrokerCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setTerminationReqCb(TFunc&& func)
    {
        m_termReqCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setBrokerReconnectReqCb(TFunc&& func)
    {
        m_brokerReconnectReqCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setClientConnectedReportCb(TFunc&& func)
    {
        m_clientConnectedCb = std::forward<TFunc>(func);
    }

    template  <typename TFunc>
    void setAuthInfoReqCb(TFunc&& func)
    {
        m_authInfoReqCb = std::forward<TFunc>(func);
    }

    void setGatewayId(std::uint8_t value)
    {
        m_state.m_gwId = value;
    }

    void setRetryPeriod(unsigned value)
    {
        m_state.m_retryPeriod = std::min(std::numeric_limits<unsigned>::max() / 1000, value) * 1000;
    }

    void setRetryCount(unsigned value)
    {
        m_state.m_retryCount = value;
    }

    void setSleepingClientMsgLimit(std::size_t value)
    {
        m_state.m_sleepPubAccLimit = std::min(m_state.m_brokerPubs.max_size(), value);
    }

    void setDefaultClientId(const std::string& value)
    {
        m_state.m_defaultClientId = value;
    }

    void setPubOnlyKeepAlive(std::uint16_t value)
    {
        m_state.m_pubOnlyKeepAlive = value;
    }

    bool start()
    {
        if ((m_state.m_running) ||
            (!m_nextTickProgramCb) ||
            (!m_cancelTickCb) ||
            (!m_sendToClientCb) ||
            (!m_sendToBrokerCb) ||
            (!m_termReqCb) ||
            (!m_brokerReconnectReqCb)) {
            return false;
        }

        m_state.m_running = true;
        return true;
    }

    void stop()
    {
        m_state.m_running = false;
    }

    bool isRunning() const
    {
        return m_state.m_running;
    }

    void tick();

    std::size_t dataFromClient(const std::uint8_t* buf, std::size_t len);
    std::size_t dataFromBroker(const std::uint8_t* buf, std::size_t len);

    void setBrokerConnected(bool connected);
    bool addPredefinedTopic(const std::string& topic, std::uint16_t topicId);
    bool setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal);

private:

    using ReturnCodeVal = cc_mqttsn::field::ReturnCodeVal;
    typedef std::vector<SessionOpPtr> OpsList;

    using Base::handle;
    virtual void handle(SearchgwMsg_SN& msg) override;
    virtual void handle(RegisterMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;

    virtual void handle(MqttMessage& msg) override;

    template <typename TStack>
    std::size_t processInputData(const std::uint8_t* buf, std::size_t len, TStack& stack);

    template <typename TMsg, typename TStack>
    void sendMessage(const TMsg& msg, TStack& stack, SendDataReqCb& func, DataBuf& buf);

    template <typename TMsg>
    void dispatchToOpsCommon(TMsg& msg);

    void sendToClient(const MqttsnMessage& msg);
    void sendToBroker(const MqttMessage& msg);
    void startOp(SessionOp& op);
    void dispatchToOps(MqttsnMessage& msg);
    void dispatchToOps(MqttMessage& msg);
    void programNextTimeout();
    void updateTimestamp();
    void updateOps();
    void apiCallExit();

#ifdef _MSC_VER
    typedef std::function<void ()> ApiCallGuard;
    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::declval<ApiCallGuard>()));
#else
    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&SessionImpl::apiCallExit, this)));
#endif

    NextTickProgramReqCb m_nextTickProgramCb;
    CancelTickWaitReqCb m_cancelTickCb;
    SendDataReqCb m_sendToClientCb;
    SendDataReqCb m_sendToBrokerCb;
    TerminationReqCb m_termReqCb;
    BrokerReconnectReqCb m_brokerReconnectReqCb;
    ClientConnectedReportCb m_clientConnectedCb;
    AuthInfoReqCb m_authInfoReqCb;

    MqttsnProtStack m_mqttsnStack;
    MqttProtStack m_mqttStack;

    DataBuf m_mqttsnMsgData;
    DataBuf m_mqttMsgData;

    OpsList m_ops;

    SessionState m_state;
};

}  // namespace cc_mqttsn_gateway


