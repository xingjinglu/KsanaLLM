/* Copyright 2023 Tencent Inc.  All rights reserved.

==============================================================================*/

#include "ksana_llm/runtime/infer_request.h"

#include <atomic>
#include <limits>
#include <vector>
#include "ksana_llm/block_manager/block_manager.h"
#include "ksana_llm/runtime/infer_stage.h"
#include "ksana_llm/utils/memory_utils.h"
#include "ksana_llm/utils/request.h"
#include "ksana_llm/utils/singleton.h"
#include "ksana_llm/utils/status.h"
#include "ksana_llm/utils/string_utils.h"
#include "ksana_llm/utils/tensor.h"

namespace ksana_llm {

// TODO(karlluo): should load from config
constexpr size_t BLOCK_SIZE = 4096;

InferRequest::InferRequest(std::shared_ptr<Request>& request)
    : req_id(request->req_id),
      model_name(request->model_name),
      input_tokens(request->input_tokens),
      output_tokens(request->output_tokens),
      sampling_config(request->sampling_config),
      waiter(request->waiter),
      step_waiter(request->step_waiter),
      finished(request->finished),
      finish_status(request->finish_status),
      output_mutex(request->output_mutex) {
  timestamp_in_ms = GetCurrentTimeInMs();
}

InferRequest::~InferRequest() {
  NLLM_LOG_DEBUG << "req " << req_id << " destroyed, free blocks if necessary.";
  if (!kv_cache_blocks.empty()) {
    FreeBlocks();
  }
}

Status InferRequest::FreeBlocks() {
  NLLM_LOG_DEBUG << "req " << req_id << " free blocks.";
  // Free memory on every device.
  for (size_t i = 0; i < kv_cache_blocks.size(); ++i) {
    GetBlockManager()->SetDeviceId(i);
    GetBlockManager()->FreeBlocks(kv_cache_blocks[i]);
  }
  kv_cache_blocks.clear();
  return Status();
}

void InferRequest::Notify() {
  if (waiter) {
    waiter->Notify();
  }
  if (step_waiter) {
    step_waiter->Notify();
  }
}

void InferRequest::NotifyStep() {
  if (step_waiter) {
    step_waiter->Notify();
  }
}

std::vector<float*> InferRequest::GetLogitsPtr() { return model_instance->GetLogitsPtr(); }

std::vector<std::vector<void*>> InferRequest::GetBlockPtrs() {
  std::vector<std::vector<void*>> block_ptrs;
  for (int rank = 0; rank < kv_cache_blocks.size(); ++rank) {
    std::vector<void*> block_ptr(kv_cache_blocks[rank].size());
    GetBlockManager()->SetDeviceId(rank);
    GetBlockManager()->GetBlockPtrs(kv_cache_blocks[rank], block_ptr);
    block_ptrs.push_back(block_ptr);
  }
  return block_ptrs;
}

size_t InferRequest::GetBlockSize() const { return BLOCK_SIZE; }

void InferRequest::ResetInferStage() {
  if (input_tokens.size() < output_tokens.size()) {
    infer_stage = InferStage::STATE_DECODE;
  }
}

size_t InferRequest::GetStepTokenNumber() {
  size_t step_token_num = 1;
  if (infer_stage == STAGE_CONTEXT) {
    step_token_num += output_tokens.size();
  }
  return step_token_num;
}

size_t InferRequest::GetTotalTokenNumber() { return output_tokens.size() + 1; }

size_t InferRequest::GetStepBlockNumber() {
  size_t block_token_num = GetBlockManager()->GetBlockTokenNum();
  size_t last_token_num = GetTotalTokenNumber() - GetStepTokenNumber();
  return GetTotalBlockNumber() - ((block_token_num + last_token_num - 1) / block_token_num);
}

size_t InferRequest::GetTotalBlockNumber() {
  size_t block_token_num = GetBlockManager()->GetBlockTokenNum();
  return (block_token_num + GetTotalTokenNumber() - 1) / block_token_num;
}

size_t InferRequest::GetCurrentBlockNumber() {
  if (!kv_cache_blocks.empty()) {
    return kv_cache_blocks[0].size();
  }
  return 0;
}

Status InferRequest::SwapInAsync() {
  for (size_t i = 0; i < kv_cache_blocks.size(); ++i) {
    std::vector<int> device_blocks;
    GetBlockManager()->SetDeviceId(i);
    GetBlockManager()->SwapIn(kv_cache_blocks[i], device_blocks);
    kv_cache_blocks[i].swap(device_blocks);
  }

  return Status();
}

Status InferRequest::SwapOutAsync() {
  for (size_t i = 0; i < kv_cache_blocks.size(); ++i) {
    std::vector<int> host_blocks;
    GetBlockManager()->SetDeviceId(i);
    GetBlockManager()->SwapOut(kv_cache_blocks[i], host_blocks);
    kv_cache_blocks[i].swap(host_blocks);
  }

  return Status();
}

Status InferRequest::DropSwappedAsync() {
  for (size_t i = 0; i < kv_cache_blocks.size(); ++i) {
    GetBlockManager()->SetDeviceId(i);
    GetBlockManager()->SwapDrop(kv_cache_blocks[i]);
  }

  return Status();
}

bool InferRequest::CheckLoraEnable() {
  // TODO
  return false;
}

size_t InferRequest::GetLoraBlockNumber() {
  // TODO
  return 0;
}

Status InferRequest::SwapInLoraAsync() {
  // TODO
  return Status();
}

Status InferRequest::SwapOutLoraAsync() {
  // TODO
  return Status();
}

Status InferRequest::AllocateStepBlocks() {
  // TODO
  return Status();
}

}  // namespace ksana_llm
