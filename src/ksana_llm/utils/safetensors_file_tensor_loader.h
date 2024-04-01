// Copyright 2023 Tencent Inc.  All rights reserved.

#pragma once

#include "base_file_tensor_loader.h"

namespace ksana_llm {
// Define a class named PytorchFileTensorLoader that inherits from BaseFileTensorLoader
class SafeTensorsLoader : public BaseFileTensorLoader {
 public:
  // Constructor that takes a file name as input
  SafeTensorsLoader(const std::string& file_name);

  ~SafeTensorsLoader();

  // Get the list of tensor names
  const std::vector<std::string>& GetTensorNameList() { return tensor_name_list_; }

  // Get a tensor by its name
  void* GetTensor(const std::string& tensor_name);

  DataType GetTensorDataType(const std::string& tensor_name);

  std::vector<std::size_t> GetTensorShape(const std::string& tensor_name);

 private:
  // Load the PyTorch binary file
  void LoadSafeTensors();

  DataType ConvertDtypeToDataType(const std::string& safetensors_dtype);

 private:
  // Use unordered_map to store the tensor names and their data ptr
  std::unordered_map<std::string, void*> tensor_ptr_map_;

  std::unordered_map<std::string, size_t> tensor_offset_map_;
  std::unordered_map<std::string, size_t> tensor_size_map_;
  std::unordered_map<std::string, DataType> tensor_data_type_map_;
  std::unordered_map<std::string, std::vector<size_t>> tensor_shape_map_;

  char* weights_buffer_ = nullptr;
};

}  // namespace ksana_llm
