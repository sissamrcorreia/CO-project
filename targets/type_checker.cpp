#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void udf::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_capacity_node(udf::tensor_capacity_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_rank_node(udf::tensor_rank_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_dim_node(udf::tensor_dim_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_dims_node(udf::tensor_dims_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_index_node(udf::tensor_index_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_reshape_node(udf::tensor_reshape_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_contract_node(udf::tensor_contract_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_tensor_node(udf::tensor_node *const node, int lvl) {
  // TODO: implement this
}

//---------------------------------------------------------------------------

void udf::type_checker::do_block_node(udf::block_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_continue_node(udf::continue_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_break_node(udf::break_node *const node, int lvl) {
  // TODO: implement this
}

//---------------------------------------------------------------------------

void udf::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_alloc_node(udf::alloc_node *const node, int lvl) {
  // TODO: implement this
}

//---------------------------------------------------------------------------

void udf::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void udf::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void udf::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<udf::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void udf::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void udf::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    auto symbol = std::make_shared<udf::symbol>(cdk::primitive_type::create(4, cdk::TYPE_INT), id, 0);
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl);  //DAVID: bah!
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of assignment expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_node(udf::function_node *const node, int lvl) {
  // TODO: implement this
}

void udf::type_checker::do_return_node(udf::return_node *const node, int lvl) {
  // TODO: implement this
}

//---------------------------------------------------------------------------

void udf::type_checker::do_evaluation_node(udf::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void udf::type_checker::do_write_node(udf::write_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_input_node(udf::input_node *const node, int lvl) {
  try {
    node->argument()->accept(this, lvl);
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_for_node(udf::for_node *const node, int lvl) {
  node->init()->accept(this, lvl + 4);
  node->condition()->accept(this, lvl + 4);
  node->increment()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_if_node(udf::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void udf::type_checker::do_if_else_node(udf::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_declaration_node(udf::declaration_node *const node, int lvl) {
  // TODO: implement this
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_call_node(udf::function_call_node *const node, int lvl) {
  // TODO: implement this
}

//---------------------------------------------------------------------------

void udf::type_checker::do_nullptr_node(udf::nullptr_node *const node, int lvl) {
  // TODO: revisit this
  ASSERT_UNSPEC;

  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_sizeof_node(udf::sizeof_node *const node, int lvl) {
  // TODO: implement this
}