cmake_policy(SET CMP0012 NEW)

macro (set_default_var_value name val)
    if (NOT DEFINED ${name})
        set (${name} ${val})
    endif ()
endmacro()

macro (replace_in_text var_name)
    string (REPLACE "##${var_name}##" "${${var_name}}" text "${text}")    
endmacro()

set_default_var_value(CC_MQTTSN_CLIENT_CUSTOM_NAME "")
set_default_var_value(CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC TRUE)
set_default_var_value(CC_MQTTSN_CLIENT_ALLOC_LIMIT 0)
#set_default_var_value(CC_MQTTSN_CLIENT_STRING_FIELD_FIXED_LEN 0)
#set_default_var_value(CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN 0)
#set_default_var_value(CC_MQTTSN_CLIENT_USERNAME_FIELD_FIXED_LEN 0)
#set_default_var_value(CC_MQTTSN_CLIENT_PASSWORD_FIELD_FIXED_LEN 0)
#set_default_var_value(CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN 0)
#set_default_var_value(CC_MQTTSN_CLIENT_BIN_DATA_FIELD_FIXED_LEN 0)
set_default_var_value(CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE 0)
set_default_var_value(CC_MQTTSN_CLIENT_RECEIVE_MAX_LIMIT 0)
set_default_var_value(CC_MQTTSN_CLIENT_SEND_MAX_LIMIT 0)
set_default_var_value(CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT 0)
set_default_var_value(CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT 0)
set_default_var_value(CC_MQTTSN_CLIENT_HAS_ERROR_LOG TRUE)
set_default_var_value(CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION TRUE)
set_default_var_value(CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION TRUE)
#set_default_var_value(CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT 0)
set_default_var_value(CC_MQTTSN_CLIENT_MAX_QOS 2)