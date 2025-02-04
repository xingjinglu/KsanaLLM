/* Copyright 2024 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include <vector>

#include "ksana_llm/runtime/forward_request.h"
#include "ksana_llm/runtime/infer_stage.h"
#include "ksana_llm/utils/environment.h"
#include "ksana_llm/utils/tensor.h"

namespace ksana_llm {

struct InputRefitCPUTensor {
  // Tensor to hold pairs(pos, data_length) of positions for input_refit on the CPU.
  Tensor pos_pair_tensor;
  Tensor emb_fp32_ptr_tensor;
};

// Convert input ids to expected format.
class ModelInput {
 public:
  ModelInput(const ModelConfig& model_config, int rank, std::shared_ptr<Context> context);
  ~ModelInput();

  // Parse forward request.
  void ParseFromRequests(const std::vector<ForwardRequest>& forward_reqs);

 private:
  // Prepare the kv cache blocks, in CSR format.
  void PrepareKVCacheBlocks(const std::vector<ForwardRequest>& forward_reqs, size_t begin_idx, size_t end_idx,
                            size_t total_block_num);

#ifdef ENABLE_FLASH_ATTN_WITH_CACHE
  // Prepare the kv cache block_table.
  void PrepareKVCacheBlockTable(const std::vector<ForwardRequest>& forward_reqs, size_t begin_idx, size_t end_idx,
                                size_t total_block_num, Tensor& block_table);
#endif

  void PrepareMultiTokenRequestPositionIds(const std::vector<ForwardRequest>& forward_reqs);

  void PrepareSingleTokenRequestPositionIds(const std::vector<ForwardRequest>& forward_reqs);

  void PrepareMultiTokenRequestInputIds(const std::vector<ForwardRequest>& forward_reqs);

  void PrepareSingleTokenRequestInputIds(const std::vector<ForwardRequest>& forward_reqs);

  void PrepareInputRefit(const std::vector<ForwardRequest>& forward_reqs);

#ifdef ENABLE_CUDA
  void PrepareCudagraphParams(const std::vector<ForwardRequest>& forward_reqs);
#endif

#ifdef ENABLE_ACL
  void PrepareATBKVCache(const std::vector<ForwardRequest>& forward_reqs, bool is_multi_token_forward);
#endif

  // Determine whether to use cache for the current batch of multi token requests.
  void CheckUseCache(const std::vector<ForwardRequest>& forward_reqs);

 public:
  // The input batch size.
  size_t batch_size;

  // The multi-token forwarding request total sequence length.
  size_t multi_token_request_total_seq_len = 0;

  // Number of requests who are forwarding multi-tokens in this step.
  size_t multi_token_request_num = 0;

  // Number of requests who are forwarding single-token in this step.
  size_t single_token_request_num = 0;

  // The total prefix length.
  size_t total_prefix_len = 0;

  // The total block number.
  size_t multi_token_request_total_block_num = 0;
  size_t single_token_request_total_block_num = 0;

  // The max tokens.
  size_t multi_token_request_max_tokens = 0;
  size_t single_token_request_max_tokens = 0;

  size_t cudagraph_batch_size = 0;

  // current request batchsize matches cudagraph catpure range
  bool is_cudagraph_batchsize_matched = false;

  // if current req is cudagraph capture request
  bool is_cudagraph_capture_request = false;

  // Whether to use kv cache.
  bool use_cache = true;

  // The cache offset list.
  std::vector<int> kv_cache_offset_list;
  std::vector<size_t> input_offset_list_uint64;
  std::vector<size_t> input_prefix_list_uint64;
  std::vector<int> input_ids_cpu;

  // The infer stage, context decode or decode.
  InferStage infer_stage;

  // The input ids, int32
  Tensor input_ids;

  // The ids offset tensor, uint64
  Tensor input_offset_uint64_tensor;

  // The input length tensor, int32
  Tensor input_length_int32_tensor;

  // If use_logits_custom_length is true, use logits_custom_length_uint64_tensor instead of input_offset_uint64_tensor
  // for calculation.
  Tensor logits_custom_length_uint64_tensor;
  // Flag to indicate if custom logits length is used.
  bool use_logits_custom_length = false;

  // Indicate the corresponding index position of the input during rotary_embedding kernel.
  Tensor rotary_embedding_pos;

  // Due to the optimization of PrefixCaching for computation reuse, a mask is used during
  // rotary_embedding computation to avoid multiple executions of rotary_embedding on the prefix block.
  Tensor rotary_embedding_mask;
  // Indicate the corresponding index position of the input during the flexible rotary_embedding kernel computation,
  // considering the impact of flexible cache optimization.
  Tensor flexible_rotary_embedding_pos;

  // Due to the optimization of PrefixCaching for computation reuse, incorporating the effects of flexible caching, a
  // mask is used during the flexible rotary_embedding computation to avoid multiple executions of flexible
  // rotary_embedding on the prefix block.
  Tensor flexible_rotary_embedding_mask;

  // The 3-dimentional index position for multimodal rotarty embedding.
  Tensor mrotary_embedding_pos;

  // The mrope sections for multimodal rotary embedding.
  // Corresponds to `model_config.rope_scaling_factor_config.mrope_section`.
  Tensor mrotary_section_tensor;

  // The input's prefix length
  Tensor input_prefix_uint64_tensor;
  // If use_logits_custom_length is true, use logits_length_prefix_uint64_tensor instead of input_prefix_uint64_tensor
  // for calculation.
  Tensor logits_length_prefix_uint64_tensor;

  // Input offset sequence and input prefix sequence on the CPU
  std::vector<int> input_offset_list;
  std::vector<int> input_prefix_list;

  Tensor kv_cache_buffer;
  Tensor kv_cache_offset_tensor;
  Tensor kv_list;
  std::vector<void*> cpu_kv_list;

  Tensor dst_flexible_kv_cache_tensor;
  Tensor src_flexible_kv_cache_tensor;
  Tensor dst_flexible_token_idx_tensor;
  Tensor src_flexible_token_idx_tensor;
  Tensor flexible_offset_uint64_tensor;

  // Tensors to hold pairs(pos, data_length) and embeddings ptr of positions for input_refit on the CPU.
  InputRefitCPUTensor cpu_input_refit_tensor;

  Event kvcache_offset_event;
  Event rotary_embedding_event;
  Event input_ids_event;

#ifdef ENABLE_ACL
  // record all reqs token number on host, shape: [batch_size]
  Tensor seq_len_host;
  // Tensor to save kv cache base. detail doc please refer:
  // docs/Technology/kvcache-relationship-between-ascend-atb-and-ksana.md shape: [total_k/v_blocks, block_token_num,
  // kv_head_num, head_dim]
  Tensor k_cache_blocks_base;
  Tensor v_cache_blocks_base;

  // for multi-token forwarding: layers_slot_mapping shape is [num_layers, all_reqs_tokens_num]
  // for single-token forwarding: layers_block_table shape is [num_layers, batch_size]
  std::vector<int32_t> layers_slot_mapping_host;
  Tensor layers_slot_mapping;

  // only used for single-token forwarding: layers_block_table shape is [num_layers, batch_size *
  // max_num_blocks_per_query]
  std::vector<int32_t> layers_block_table_host;
  Tensor layers_block_table;

  // since layer's forward only support Tensor as input (nothing to do with karlluo), such crappy design ignore runtime
  // attribute, so we need a tensor to be attribute.
  // shape: [2]; 0: layers_slot_mapping_dim_1; 1: max_num_blocks_per_query
  Tensor atb_attention_attr;

  // assemble last token index for gather, dtype is int64_t
  Tensor last_token_index_tensor;

  std::vector<void*> kv_cache_ptrs;
  Tensor kv_cache_ptrs_tensor;
#endif

#ifdef ENABLE_FLASH_ATTN_WITH_CACHE
  std::vector<int32_t> block_table_host;
  Tensor multi_token_request_block_table, single_token_request_block_table;
  Tensor layer_kv_cache_ptr_tensor;
  std::vector<size_t> input_without_prefix_list_uint64;
  Tensor input_without_prefix_uint64_tensor;
  size_t max_forwarding_tokens = 0;
#endif

 private:
  ModelConfig model_config_;
  PipelineConfig pipeline_config_;

  int rank_;
  std::shared_ptr<Context> context_;

  int block_size_;
  size_t max_batch_size_;
  size_t max_token_num_;
  int layer_num_on_node_;

  // Used for assisant.
  std::vector<int64_t> multi_token_cpu_rotary_pos_;
  std::vector<int64_t> multi_token_cpu_rotary_mask_;
  std::vector<int64_t> single_token_cpu_rotary_pos_;
  std::vector<int64_t> single_token_cpu_rotary_mask_;
};

}  // namespace ksana_llm
