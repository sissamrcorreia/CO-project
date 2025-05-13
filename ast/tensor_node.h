#pragma once

namespace udf
{
    /**
     * Class for describing tensor declaration nodes.
     * Represents the creation of a tensor with specified dimensions (e.g., `tensor<2,3> t`).
     */

    class tensor_node : public cdk::expression_node
    {
        std::vector<cdk::expression_node *> _dimensions;
        std::string _name;
        bool _is_global;

    public:
        tensor_node(int lineno, const std::string &name, bool is_global,
                    const std::vector<cdk::expression_node *> &dimensions) : cdk::expression_node(lineno), _dimensions(dimensions), _name(name), _is_global(is_global)
        {
        }

        const std::string &name() { return _name; }
        bool is_global() { return _is_global; }
        const std::vector<cdk::expression_node *> &dimensions() { return _dimensions; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_tensor_node(this, level); }
    };

} // udf
