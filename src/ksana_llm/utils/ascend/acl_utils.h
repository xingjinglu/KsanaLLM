/* Copyright 2024 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include <vector>

#include "acl/acl.h"
#include "acl/acl_base.h"
#include "acl/acl_op_compiler.h"
#include "acl/acl_rt.h"
#include "aclnn/acl_meta.h"

#include "ksana_llm/utils/device_types.h"
#include "ksana_llm/utils/logger.h"
#include "ksana_llm/utils/ret_code.h"

#ifdef WITH_ACL_ATB
#include "atb/types.h"
#endif

namespace ksana_llm {

std::vector<int64_t> GetAclTensorShape(aclTensor* tensor);

DataType GetAclTensorDataType(aclTensor* tensor);

// ref:
// https://www.hiascend.com/document/detail/zh/canncommercial/700/inferapplicationdev/aclcppdevg/aclcppdevg_03_1251.html
static const std::string GetACLErrorString(aclError error) {
  static const std::unordered_map<aclError, std::string> error_to_string_map{
      {ACL_SUCCESS, "ACL_SUCCESS"},
      {ACL_ERROR_INVALID_DUMP_CONFIG, "ACL_ERROR_INVALID_DUMP_CONFIG"},
      {ACL_ERROR_FILE_ATTR_INVALID, "ACL_ERROR_FILE_ATTR_INVALID"},
      {ACL_ERROR_FILE_MISSING_ATTR, "ACL_ERROR_FILE_MISSING_ATTR"},
      {ACL_ERROR_PARSE_FILE, "ACL_ERROR_PARSE_FILE"},
      {ACL_ERROR_INVALID_FILE_SIZE, "ACL_ERROR_INVALID_FILE_SIZE"},
      {ACL_ERROR_INVALID_FILE, "ACL_ERROR_INVALID_FILE"},
      {ACL_ERROR_WRITE_FILE, "ACL_ERROR_WRITE_FILE"},
      {ACL_ERROR_REPEAT_INITIALIZE, "ACL_ERROR_REPEAT_INITIALIZE"},
      {ACL_ERROR_UNINITIALIZE, "ACL_ERROR_UNINITIALIZE"},
      {ACL_ERROR_INVALID_PARAM, "ACL_ERROR_INVALID_PARAM"},
      {ACL_ERROR_MODEL_OUTPUT_NOT_MATCH, "ACL_ERROR_MODEL_OUTPUT_NOT_MATCH"},
      {ACL_ERROR_MODEL_INPUT_NOT_MATCH, "ACL_ERROR_MODEL_INPUT_NOT_MATCH"},
      {ACL_ERROR_MODEL_MISSING_ATTR, "ACL_ERROR_MODEL_MISSING_ATTR"},
      {ACL_ERROR_MODEL_SIZE_INVALID, "ACL_ERROR_MODEL_SIZE_INVALID"},
      {ACL_ERROR_READ_MODEL_FAILURE, "ACL_ERROR_READ_MODEL_FAILURE"},
      {ACL_ERROR_PARSE_MODEL, "ACL_ERROR_PARSE_MODEL"},
      {ACL_ERROR_DESERIALIZE_MODEL, "ACL_ERROR_DESERIALIZE_MODEL"},
      {ACL_ERROR_INVALID_MODEL_ID, "ACL_ERROR_INVALID_MODEL_ID"},
      {ACL_ERROR_BIN_SELECTOR_NOT_REGISTERED, "ACL_ERROR_BIN_SELECTOR_NOT_REGISTERED"},
      {ACL_ERROR_FORMAT_NOT_MATCH, "ACL_ERROR_FORMAT_NOT_MATCH"},
      {ACL_ERROR_UNSUPPORTED_DATA_TYPE, "ACL_ERROR_UNSUPPORTED_DATA_TYPE"},
      {ACL_ERROR_OP_LOAD_FAILED, "ACL_ERROR_OP_LOAD_FAILED"},
      {ACL_ERROR_OP_NOT_FOUND, "ACL_ERROR_OP_NOT_FOUND"},
      {ACL_ERROR_OP_ATTR_NOT_MATCH, "ACL_ERROR_OP_ATTR_NOT_MATCH"},
      {ACL_ERROR_OP_OUTPUT_NOT_MATCH, "ACL_ERROR_OP_OUTPUT_NOT_MATCH"},
      {ACL_ERROR_OP_INPUT_NOT_MATCH, "ACL_ERROR_OP_INPUT_NOT_MATCH"},
      {ACL_ERROR_OP_TYPE_NOT_MATCH, "ACL_ERROR_OP_TYPE_NOT_MATCH"},
      {ACL_ERROR_MODEL_NOT_DYNAMIC, "ACL_ERROR_MODEL_NOT_DYNAMIC"},
      {ACL_ERROR_RT_DRV_INTERNAL_ERROR, "ACL_ERROR_RT_DRV_INTERNAL_ERROR"},
      {ACL_ERROR_RT_MEMORY_ALLOCATION, "ACL_ERROR_RT_MEMORY_ALLOCATION"},
      {ACL_ERROR_DRV_FAILURE, "ACL_ERROR_DRV_FAILURE"},
      {ACL_ERROR_RT_FAILURE, "ACL_ERROR_RT_FAILURE"},
      {ACL_ERROR_GE_FAILURE, "ACL_ERROR_GE_FAILURE"},
      {ACL_ERROR_FAILURE, "ACL_ERROR_FAILURE"},
      {ACL_ERROR_INTERNAL_ERROR, "ACL_ERROR_INTERNAL_ERROR"},
      {ACL_ERROR_STORAGE_OVER_LIMIT, "ACL_ERROR_STORAGE_OVER_LIMIT"},
      {ACL_ERROR_FEATURE_UNSUPPORTED, "ACL_ERROR_FEATURE_UNSUPPORTED"},
      {ACL_ERROR_INVALID_RESOURCE_HANDLE, "ACL_ERROR_INVALID_RESOURCE_HANDLE"},
      {ACL_ERROR_RESOURCE_NOT_MATCH, "ACL_ERROR_RESOURCE_NOT_MATCH"},
      {ACL_ERROR_MEMORY_ADDRESS_UNALIGNED, "ACL_ERROR_MEMORY_ADDRESS_UNALIGNED"},
      {ACL_ERROR_INVALID_DEVICE, "ACL_ERROR_INVALID_DEVICE"},
      {ACL_ERROR_API_NOT_SUPPORT, "ACL_ERROR_API_NOT_SUPPORT"},
      {ACL_ERROR_BAD_ALLOC, "ACL_ERROR_BAD_ALLOC"},
      {ACL_ERROR_RELATIVE_RESOURCE_NOT_CLEARED, "ACL_ERROR_RELATIVE_RESOURCE_NOT_CLEARED"},
      {ACL_ERROR_OP_UNSUPPORTED_DYNAMIC, "ACL_ERROR_OP_UNSUPPORTED_DYNAMIC"},
      {ACL_ERROR_INVALID_OPP_PATH, "ACL_ERROR_INVALID_OPP_PATH"},
      {ACL_ERROR_INVALID_MAX_OPQUEUE_NUM_CONFIG, "ACL_ERROR_INVALID_MAX_OPQUEUE_NUM_CONFIG"},
      {ACL_ERROR_DUMP_NOT_RUN, "ACL_ERROR_DUMP_NOT_RUN"},
      {ACL_ERROR_DUMP_ALREADY_RUN, "ACL_ERROR_DUMP_ALREADY_RUN"},
      {ACL_ERROR_GROUP_NOT_CREATE, "ACL_ERROR_GROUP_NOT_CREATE"},
      {ACL_ERROR_GROUP_NOT_SET, "ACL_ERROR_GROUP_NOT_SET"},
      {ACL_ERROR_COMPILING_STUB_MODE, "ACL_ERROR_COMPILING_STUB_MODE"},
      {ACL_ERROR_NOT_STATIC_AIPP, "ACL_ERROR_NOT_STATIC_AIPP"},
      {ACL_ERROR_REPEAT_FINALIZE, "ACL_ERROR_REPEAT_FINALIZE"},
      {ACL_ERROR_WAIT_CALLBACK_TIMEOUT, "ACL_ERROR_WAIT_CALLBACK_TIMEOUT"},
      {ACL_ERROR_STREAM_NOT_SUBSCRIBE, "ACL_ERROR_STREAM_NOT_SUBSCRIBE"},
      {ACL_ERROR_REPEAT_SUBSCRIBE, "ACL_ERROR_REPEAT_SUBSCRIBE"},
      {ACL_ERROR_INVALID_QUEUE_ID, "ACL_ERROR_INVALID_QUEUE_ID"},
      {ACL_ERROR_KERNEL_ALREADY_REGISTERED, "ACL_ERROR_KERNEL_ALREADY_REGISTERED"},
      {ACL_ERROR_BIN_SELECTOR_ALREADY_REGISTERED, "ACL_ERROR_BIN_SELECTOR_ALREADY_REGISTERED"},
      {ACL_ERROR_KERNEL_NOT_FOUND, "ACL_ERROR_KERNEL_NOT_FOUND"}};
  if (error_to_string_map.count(error) != 0ul) {
    return error_to_string_map.at(error);
  } else {
    return "UNKOWN, refer: "
           "https://www.hiascend.com/document/detail/zh/canncommercial/700/inferapplicationdev/aclcppdevg/"
           "aclcppdevg_03_1251.html for more information";
  }
}

template <typename T>
void CheckACLError(T result, const char* func, const char* file, const int line) {
  if (result != ACL_SUCCESS) {
    NLLM_LOG_ERROR << fmt::format("ACL runtime error {}: {} {}:{}@{}", result, GetACLErrorString(result), file, line,
                                  func);
    abort();
    exit(RetCode::RET_INVALID_ARGUMENT);
  }
}

#define ACL_CHECK(val) CheckACLError((val), #val, __FILE__, __LINE__)

#ifdef WITH_ACL_ATB
static const std::string GetATBErrorString(atb::ErrorType error) {
  static const std::unordered_map<atb::ErrorType, std::string> error_to_string_map{
      {NO_ERROR, "NO_ERROR"},
      {ERROR_INVALID_PARAM, "ERROR_INVALID_PARAM"},
      {ERROR_INVALID_GRAPH, "ERROR_INVALID_GRAPH"},
      {ERROR_INTERNAL_ERROR, "ERROR_INTERNAL_ERROR"},
      {ERROR_RT_FAIL, "ERROR_RT_FAIL"},
      {ERROR_INVALID_IN_TENSOR_NUM, "ERROR_INVALID_IN_TENSOR_NUM"},
      {ERROR_INVALID_TENSOR_DTYPE, "ERROR_INVALID_TENSOR_DTYPE"},
      {ERROR_INVALID_TENSOR_FORMAT, "ERROR_INVALID_TENSOR_FORMAT"},
      {ERROR_INVALID_TENSOR_DIM, "ERROR_INVALID_TENSOR_DIM"},
      {ERROR_INVALID_TENSOR_SIZE, "ERROR_INVALID_TENSOR_SIZE"},
      {ERROR_OPERATION_NULL_RUNNER, "ERROR_OPERATION_NULL_RUNNER"},
      {ERROR_GRAPH_INFERSHAPE_FUNC_FAIL, "ERROR_GRAPH_INFERSHAPE_FUNC_FAIL"},
      {ERROR_CANN_ERROR, "ERROR_CANN_ERROR"},
      {ERROR_INVALID_TENSOR_INI_MATCH, "ERROR_INVALID_TENSOR_INI_MATCH"}};
  if (error_to_string_map.count(error) != 0ul) {
    return error_to_string_map.at(error);
  } else {
    return "UNKOWN, refer: "
           "https://www.hiascend.com/document/detail/zh/mindie/1.0.RC1/mindiert/rtdev/ascendtb_01_0008.html";
  }
}

template <typename T>
void CheckATBError(T result, const char* func, const char* file, const int line) {
  if (result != ACL_SUCCESS) {
    NLLM_LOG_ERROR << fmt::format("ATB runtime error {}: {} {}:{}@{}", result, GetATBErrorString(result), file, line,
                                  func);
    abort();
    exit(RetCode::RET_INVALID_ARGUMENT);
  }
}

#define ATB_CHECK(val) CheckATBError((val), #val, __FILE__, __LINE__)

#endif

std::vector<int>& GetPaddedTokenSize();

}  // namespace ksana_llm
