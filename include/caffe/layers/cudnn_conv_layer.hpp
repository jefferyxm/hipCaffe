#ifndef CAFFE_CUDNN_CONV_LAYER_HPP_
#define CAFFE_CUDNN_CONV_LAYER_HPP_

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"

#include "caffe/layers/conv_layer.hpp"

namespace caffe {

#ifdef USE_ACCMI
/*
 * @brief cuDNN implementation of ConvolutionLayer.
 *        Fallback to ConvolutionLayer for CPU mode.
 *
 * cuDNN accelerates convolution through forward kernels for filtering and bias
 * plus backward kernels for the gradient w.r.t. the filters, biases, and
 * inputs. Caffe + cuDNN further speeds up the computation through forward
 * parallelism across groups and backward parallelism across gradients.
 *
 * The CUDNN engine does not have memory overhead for matrix buffers. For many
 * input and filter regimes the CUDNN engine is faster than the CAFFE engine,
 * but for fully-convolutional models and large inputs the CAFFE engine can be
 * faster as long as it fits in memory.
*/
template <typename Dtype>
class CuDNNConvolutionLayer : public ConvolutionLayer<Dtype> {
 public:
#ifdef CONV_LAYER_COUNT
  static int count;
#endif

  explicit CuDNNConvolutionLayer(const LayerParameter& param);

  virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);
  virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);
  virtual ~CuDNNConvolutionLayer();

 protected:
  virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);
  virtual void Backward_gpu(const vector<Blob<Dtype>*>& top,
      const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);


  bool handles_setup_;

#ifdef USE_MIOPEN

  // algorithms for forward and backwards convolutions
  vector<miopenConvFwdAlgorithm_t>        fwd_algo_;
  vector<miopenConvBwdWeightsAlgorithm_t> bwd_weight_algo_;
  vector<miopenConvBwdDataAlgorithm_t>    bwd_data_algo_;

  vector<miopenTensorDescriptor_t>      bottom_descs_, top_descs_;
  miopenTensorDescriptor_t              bias_desc_;
  miopenTensorDescriptor_t              filter_desc_;
  vector<miopenConvolutionDescriptor_t> conv_descs_;

  int N_, C_, W_, H_;
  miopenHandle_t handle_;
#endif

  int bottom_offset_, top_offset_, bias_offset_;

  vector<size_t> workspace_fwd_sizes_;
  vector<size_t> workspace_bwd_filter_sizes_;
  vector<size_t> workspace_bwd_data_sizes_;
  size_t workspaceSizeInBytes;  // size of underlying storage
  void *workspaceData;  // underlying storage
  vector<void*>  workspace;  // aliases into workspaceData
};
#endif

}  // namespace caffe

#endif  // CAFFE_CUDNN_CONV_LAYER_HPP_
