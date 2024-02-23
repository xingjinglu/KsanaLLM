/* Copyright 2023 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include "ksana_llm/runtime/sampling_request.h"
#include "ksana_llm/samplers/base/base_sampling.h"
#include "ksana_llm/samplers/topk/topk_sampling.h"
#include "ksana_llm/utils/environment.h"
#include "ksana_llm/utils/status.h"
#include "ksana_llm/utils/tensor.h"

namespace ksana_llm {

class Sampler {
 public:
  Sampler(int rank);
  ~Sampler();
  Status Sampling(std::vector<SamplingRequest>& sampling_reqs, cudaStream_t& stream);

 private:
  int rank_;
  TopkSampling* topk_sampling_;
  uint32_t* device_output_token_;
  uint32_t host_output_token_;
  int output_token_block_id_;
  uint32_t* device_offset_;
  int offset_block_id_;
};

}  // namespace ksana_llm