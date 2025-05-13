#pragma once

namespace udf
{
    /**
     * Class for describing tensor dimensions queries.
     * Provides access to the tensor's dimensions as an integer array (e.g., `t.dims`).
     */

    class tensor_dims_node : public cdk::expression_node
    {
        cdk::expression_node *_argument;
        cdk::expression_node *_index;

    public:
        tensor_dims_node(int lineno, cdk::expression_node *argument, cdk::expression_node *index) : cdk::expression_node(lineno), _argument(argument), _index(index)
        {
        }

        cdk::expression_node *argument() { return _argument; }
        cdk::expression_node *index() { return _index; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_dims_node(this, level); }
    };

} // udf
