#pragma once

#include "SOTCooker.pb.h"

#include "ProtoHelp.hpp" //NOLINT
#include "CPSMProtoConstants.hpp"

namespace pb {

template<FromPbMessage Proto_t>
inline
int32_t InitKeysToDefault(Proto_t *msg){
    int32_t match_count{};

    std::function<bool(const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)> to_apply =
    [&msg,&match_count](const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)->bool{
        auto isKey{pb::GetFieldOption<Proto_t>(desc.number(),sot::is_keyboard_key)};
        if(!isKey){
            return false;
        }
        ++match_count;
        reflection.SetInt32(msg,&desc,sot::kUnbindKey);

        return false;
    };
    pb::ApplyOnPbFields(msg,to_apply);
    return match_count;
}

//https://protobuf.dev/reference/cpp/api-docs/google.protobuf.message/#Reflection
template<>
inline
sot::KeyboardProfile GetDefault(){
    sot::KeyboardProfile out{};

    out.set_profile_name("Default");

    constexpr auto kExpectedKeyFieldCount{10};

    int32_t keyFieldCount{InitKeysToDefault(&out)};

    if(keyFieldCount != kExpectedKeyFieldCount)
    {
        qDebug() << "KeyFieldCount:" << keyFieldCount;
        qDebug() << "Expected:" << kExpectedKeyFieldCount;
        throw std::runtime_error(std::string{__PRETTY_FUNCTION__}+" Invalid init for: "+out.descriptor()->full_name());
    }
    return out;
}

} // namespace pb
