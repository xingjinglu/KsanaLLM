/* Copyright 2023 Tencent Inc.  All rights reserved.

==============================================================================*/

#include "numerous_llm/batch_manager/batch_manager.h"
#include "numerous_llm/utils/logger.h"

namespace numerous_llm {

BatchManager::BatchManager(const BatchManagerConfig &batch_manager_config) {
  batch_manager_config_ = batch_manager_config;
  Initialize();
}

Status BatchManager::Initialize() {

  batch_scheduler_ = std::make_shared<BatchScheduler>(
      batch_manager_config_.batch_scheduler_config);

  context_caching_ = std::make_shared<ContextCaching>(
      batch_manager_config_.context_caching_config);

  block_manager_ = std::make_shared<BlockManager>(
      batch_manager_config_.block_manager_config);

  lora_coordinator_ = std::make_shared<LoraCoordinator>(
      batch_manager_config_.lora_coordinator_config);

  request_batching_ = std::make_shared<RequestBatching>(
      batch_manager_config_.request_batching_config);

  llm_runtime_ = std::make_shared<LlmRuntime>();

  llm_sampler_ = std::make_shared<Sampler>();

  return Status();
}

Status BatchManager::RegisterModelInstance(
    const std::shared_ptr<ModelInstance> &model_instance) {
  model_instances_[model_instance->name] = model_instance;
  return Status();
}

Status
BatchManager::Enqueue(int req_id, const std::vector<TensorMap> &tensor_maps,
                      const std::vector<SamplingConfig> &sampling_configs) {
  NLLM_LOG_INFO << "batch manager enqueue." << std::endl;

  // Split into multiple prompt
  if (tensor_maps.size() != sampling_configs.size()) {
    return Status(RET_INVALID_ARGUMENT,
                  "Size of tensor_maps and sampling_configs should be equal.");
  }

  for (size_t i = 0; i < tensor_maps.size(); ++i) {
    InferRequest infer_req;
    infer_req.req_id = req_id;
    infer_req.input_tensor_map = tensor_maps[i];
    infer_req.sampling_config = sampling_configs[i];
    infer_req.model_instance = model_instances_[infer_req.model_name];

    batch_scheduler_->AddInferRequest(infer_req);
    NLLM_LOG_INFO << "batch schdule add request." << std::endl;
  }

  return Status();
}

Status BatchManager::WaitDone(int req_id, std::vector<TensorMap> &tensor_maps) {
  return Status();
}

Status BatchManager::WaitAllDone() { return Status(); }

Status BatchManager::Process() {
  while (!terminated_) {
    std::vector<InferRequest> scheduled_reqs;
    batch_scheduler_->Schedule(scheduled_reqs);
    if (scheduled_reqs.empty()) {
      continue;
    }

    NLLM_LOG_INFO << "batch scheduler result:" << scheduled_reqs.size()
                  << std::endl;

    llm_runtime_->Step(scheduled_reqs);
    llm_sampler_->Sampling(scheduled_reqs);
  }

  return Status();
}

Status BatchManager::Start() {
  batch_manager_thread_ = std::unique_ptr<std::thread>(
      new std::thread(&BatchManager::Process, this));

  return Status();
}

Status BatchManager::Stop() {
  terminated_ = true;
  if (batch_manager_thread_ && batch_manager_thread_->joinable()) {
    batch_manager_thread_->join();
  }
  return Status();
}

} // namespace numerous_llm
