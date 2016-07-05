//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include <tuple>

#include "mqttsn/protocol/option.h"

namespace mqttsn
{

namespace protocol
{

namespace details
{

template <typename... TOptions>
struct OptionsParser;

template <>
struct OptionsParser<>
{
    static const bool HasGwAddStaticStorageSize = false;
    static const bool HasClientIdStaticStorageSize = false;
    static const bool HasTopicNameStaticStorageSize = false;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    option::GwAddStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef option::GwAddStaticStorageSize<TSize> Option;
public:
    static const bool HasGwAddStaticStorageSize = true;
    static const std::size_t GwAddStaticStorageSize = Option::Value;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    option::ClientIdStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef option::ClientIdStaticStorageSize<TSize> Option;
public:
    static const bool HasClientIdStaticStorageSize = true;
    static const std::size_t ClientIdStaticStorageSize = Option::Value;
};

template <std::size_t TSize, typename... TOptions>
class OptionsParser<
    option::TopicNameStaticStorageSize<TSize>,
    TOptions...> : public OptionsParser<TOptions...>
{
    typedef option::TopicNameStaticStorageSize<TSize> Option;
public:
    static const bool HasTopicNameStaticStorageSize = true;
    static const std::size_t TopicNameStaticStorageSize = Option::Value;
};

template <typename... TTupleOptions, typename... TOptions>
class OptionsParser<
    std::tuple<TTupleOptions...>,
    TOptions...> : public OptionsParser<TTupleOptions..., TOptions...>
{
};


}  // namespace details

}  // namespace protocol

}  // namespace mqttsn


