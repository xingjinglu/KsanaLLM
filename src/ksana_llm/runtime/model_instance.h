/* Copyright 2024 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include <future>
#include <memory>
#include <string>

#include "ksana_llm/kernels/cast.h"
#include "ksana_llm/models/base/base_model.h"
#include "ksana_llm/runtime/forward_request.h"
#include "ksana_llm/runtime/infer_stage.h"
#include "ksana_llm/runtime/threadpool.h"
#include "ksana_llm/utils/context.h"
#include "ksana_llm/utils/environment.h"
#include "ksana_llm/utils/search_path.h"
#include "ksana_llm/utils/singleton.h"
#include "ksana_llm/utils/tensor.h"

namespace ksana_llm {
class Worker;
class WorkerGroup;

class ModelInstance {
 public:
  ModelInstance(const ModelConfig& model_config, std::shared_ptr<Context> context)
      : model_config_(model_config), context_(context) {
    loader_weight_threadpool_ = std::make_shared<ThreadPool>(context->GetTensorParallelSize());
    loader_weight_threadpool_->Start();
  }
  ~ModelInstance() {
    loader_weight_threadpool_->Stop();
    models_.clear();
    weights_.clear();
  }
  // Load model with specified model config.
  void Load();

  // The instance name.
  std::string name;
  std::string type;

  std::vector<Status> Forward(std::shared_ptr<WorkerGroup> worker_group, InferStage stage,
                              std::vector<ForwardRequest>& forward_reqs, bool epilogue);

  std::vector<std::future<Status>> ForwardAsync(std::shared_ptr<WorkerGroup> worker_group, InferStage stage,
                                                std::vector<ForwardRequest>& forward_reqs, bool epilogue);

  // Get  the data type.
  DataType GetWeightDataType() { return model_config_.weight_data_type; }

  // Get the base ptr of model's logits buf.
  std::vector<float*> GetLogitsPtr();

  const ModelConfig& GetModelConfig() { return model_config_; }

  size_t GetMaxTokenNum() { return model_config_.max_token_num; }

  uint32_t GetLayerNum() {
    static bool initialized = false;
    if (!initialized) {
      Singleton<Environment>::GetInstance()->GetPipelineConfig(pipeline_config_);
      initialized = true;
    }
    return (pipeline_config_.upper_layer_idx - pipeline_config_.lower_layer_idx + 1);
  }

  static void Reset() {
    KLLM_LOG_INFO << "ModelInstance::Reset clear models and weights.";
    models_.clear();
    weights_.clear();
  }

 private:
  // Create the object and return a shared pointer.
  template <template <class> class ClassT, class BaseT, class... Args>
  std::shared_ptr<BaseT> CreatetModelObject(int rank, Args&&... args) {
    std::shared_ptr<BaseT> model_obj = nullptr;
    switch (model_config_.weight_data_type) {
      case DataType::TYPE_FP16:
        model_obj = std::make_shared<ClassT<float16>>(model_config_, rank, context_, std::forward<Args>(args)...);
        break;
#ifdef ENABLE_BFLOAT16
      case DataType::TYPE_BF16:
        model_obj = std::make_shared<ClassT<bfloat16>>(model_config_, rank, context_, std::forward<Args>(args)...);
        break;
#endif
      case DataType::TYPE_FP32:
        model_obj = std::make_shared<ClassT<float>>(model_config_, rank, context_, std::forward<Args>(args)...);
        break;
      default:
        KLLM_THROW(fmt::format("Unsupported Tensor type: {}.", model_config_.weight_data_type));
    }
    return model_obj;
  }

  template <template <class> class ClassT>
  std::shared_ptr<BaseModel> CreateModel(int rank, std::shared_ptr<BaseWeight> base_weight) {
    return CreatetModelObject<ClassT, BaseModel>(rank, base_weight);
  }

  template <template <class> class ClassT>
  std::shared_ptr<BaseWeight> CreateModelWeight(int rank) {
    return CreatetModelObject<ClassT, BaseWeight>(rank);
  }

  void SetEmbeddingsConfig();

  void CheckTieEmbeddings(int weight_file_size);

  void CheckTieEmbeddings(std::vector<std::string>& custom_name_list);

  void LoadWeightsAndModelsMap();

  template <template <class> class ModelType, template <class> class WeightType>
  void CreateModelInstance(const std::string model_name) {
    KLLM_LOG_INFO << "Start to init model instance " << model_name;
    for (int worker_id = 0; worker_id < context_->GetTensorParallelSize(); ++worker_id) {
      KLLM_LOG_INFO << "Start to create empty model weight on device " << worker_id;
      weights_.push_back(CreateModelWeight<WeightType>(worker_id));
    }
    LoadWeightsAndModelsMap();
    for (int worker_id = 0; worker_id < context_->GetTensorParallelSize(); ++worker_id) {
      KLLM_LOG_INFO << "Start to create model on device " << worker_id;
      models_.push_back(CreateModel<ModelType>(worker_id, weights_[worker_id]));
    }
  }

 private:
  // The model config.
  ModelConfig model_config_;

  PipelineConfig pipeline_config_;

  // The global context.
  std::shared_ptr<Context> context_ = nullptr;

  // The base model and weight, shared by all model instances.
  static std::vector<std::shared_ptr<BaseModel>> models_;
  static std::vector<std::shared_ptr<BaseWeight>> weights_;

  std::shared_ptr<ThreadPool> loader_weight_threadpool_ = nullptr;
};

}  // namespace ksana_llm
