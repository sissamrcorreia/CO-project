#pragma once

namespace udf
{
    /**
     * Class for describing single-dimension queries.
     * Retrieves the size of a specific tensor dimension (e.g., `t.dim(0)`).
     */

    class tensor_dim_node : public cdk::expression_node
    {
        cdk::expression_node *_argument;
        cdk::expression_node *_dimension;

    public:
        tensor_dim_node(int lineno, cdk::expression_node *argument, cdk::expression_node *dimension) : cdk::expression_node(lineno), _argument(argument), _dimension(dimension)
        {
        }

        cdk::expression_node *argument() { return _argument; }
        cdk::expression_node *dimension() { return _dimension; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_dim_node(this, level); }
    };

} // udf
