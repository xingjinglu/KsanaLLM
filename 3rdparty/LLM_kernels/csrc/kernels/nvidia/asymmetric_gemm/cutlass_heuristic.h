/*
 * Copyright 2024 Tencent Inc.  All rights reserved.
 * Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <vector>

#include "csrc/kernels/nvidia/cutlass_extensions/gemm_configs.h"
#include "csrc/utils/nvidia/cuda_utils.h"

namespace llm_kernels {
namespace nvidia {

std::vector<llm_kernels::nvidia::cutlass_extensions::CutlassGemmConfig> GetCandidateConfigs(
    int32_t sm, const bool is_weight_only, const bool simt_configs_only, const bool int8_configs_only = false,
    const int32_t max_split_k = 1);

llm_kernels::nvidia::cutlass_extensions::CutlassGemmConfig EstimateBestConfigFromOccupancies(
    const std::vector<llm_kernels::nvidia::cutlass_extensions::CutlassGemmConfig>& candidate_configs,
    const std::vector<int32_t>& occupancies, const int64_t m, const int64_t n, const int64_t k,
    const int64_t num_experts, const int32_t split_k_limit, const size_t workspace_bytes,
    const int32_t multi_processor_count, const int32_t is_weight_only);

}  // namespace nvidia
}  // namespace llm_kernels
