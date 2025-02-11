// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "pass_level2.h"

namespace pnnx {

class F_batch_norm : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
11 10
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 running_mean
pnnx.Input              input_2     0 1 running_var
pnnx.Input              input_3     0 1 weight
pnnx.Input              input_4     0 1 bias
prim::Constant          op_0        0 1 training value=*
prim::Constant          op_1        0 1 momentum value=*
prim::Constant          op_2        0 1 eps value=%eps
prim::Constant          op_3        0 1 cudnn_enabled value=*
aten::batch_norm        op_4        9 1 input weight bias running_mean running_var training momentum eps cudnn_enabled out
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.batch_norm";
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_batch_norm, 130)

class F_batch_norm_1 : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
9 10
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 running_mean
pnnx.Input              input_2     0 1 running_var
pnnx.Input              input_3     0 1 weight
pnnx.Input              input_4     0 1 bias
prim::Constant          op_0        0 1 momentum value=*
prim::Constant          op_1        0 1 eps value=%eps
aten::_native_batch_norm_legit_no_training op_2 7 3 input weight bias running_mean running_var momentum eps out save_mean save_invstd
pnnx.Output             output      3 0 out save_mean save_invstd
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.batch_norm";
    }

    void write(Operator* op, const std::map<std::string, Parameter>& captured_params, const std::map<std::string, Attribute>& captured_attrs) const
    {
        GraphRewriterPass::write(op, captured_params, captured_attrs);

        op->outputs.resize(1);
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_batch_norm_1, 130)

class F_batch_norm_onnx : public GraphRewriterPass
{
public:
    const char* match_pattern_graph() const
    {
        return R"PNNXIR(7767517
7 6
pnnx.Input              input_0     0 1 input
pnnx.Input              input_1     0 1 weight
pnnx.Input              input_2     0 1 bias
pnnx.Input              input_3     0 1 running_mean
pnnx.Input              input_4     0 1 running_var
BatchNormalization      op_0        5 1 input weight bias running_mean running_var out %*=%*
pnnx.Output             output      1 0 out
)PNNXIR";
    }

    const char* type_str() const
    {
        return "F.batch_norm";
    }

    void write(Operator* op, const std::map<std::string, Parameter>& captured_params) const
    {
        if (captured_params.find("op_0.epsilon") != captured_params.end())
        {
            op->params["eps"] = captured_params.at("op_0.epsilon");
        }
        else
        {
            op->params["eps"] = 1e-05;
        }

        std::swap(op->inputs[1], op->inputs[3]);
        std::swap(op->inputs[2], op->inputs[4]);
        std::swap(op->inputnames[1], op->inputnames[3]);
        std::swap(op->inputnames[2], op->inputnames[4]);
    }
};

REGISTER_GLOBAL_PNNX_GRAPH_REWRITER_PASS(F_batch_norm_onnx, 130)

} // namespace pnnx
