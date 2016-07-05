//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Unsublic License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Unsublic License for more details.
//
// You should have received a copy of the GNU General Unsublic License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "comms/comms.h"
#include "mqttsn/protocol/MsgTypeId.h"
#include "mqttsn/protocol/field.h"
#include "mqttsn/protocol/ParsedOptions.h"

namespace mqttsn
{

namespace protocol
{

namespace message
{

template <typename TFieldBase, typename TOptions>
using PingreqFields =
    std::tuple<
        field::ClientId<TFieldBase, TOptions>
    >;

template <typename TMsgBase, typename TOptions = protocol::ParsedOptions<> >
class Pingreq : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_PINGREQ>,
        comms::option::FieldsImpl<PingreqFields<typename TMsgBase::Field, TOptions> >,
        comms::option::DispatchImpl<Pingreq<TMsgBase> >
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_PINGREQ>,
        comms::option::FieldsImpl<PingreqFields<typename TMsgBase::Field, TOptions> >,
        comms::option::DispatchImpl<Pingreq<TMsgBase> >
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_clientId,
        FieldIdx_numOfValues
    };

    static_assert(std::tuple_size<typename Base::AllFields>::value == FieldIdx_numOfValues,
        "Number of fields is incorrect");
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


