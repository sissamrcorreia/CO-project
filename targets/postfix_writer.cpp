#include <string>
#include <sstream>
#include <memory>
#include <cdk/types/types.h>
#include "targets/postfix_writer.h"
#include "targets/type_checker.h"
#include "targets/frame_size_calculator.h"
#include "targets/symbol.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated
// must come after other #includes
#include "udf_parser.tab.h"

//---------------------------------------------------------------------------

void udf::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void udf::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.DOUBLE(node->value()); // Push a double
  } else {
    _pf.SDOUBLE(node->value()); // Double is on the DATA segment
  }
}

void udf::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.INT(node->value()); // Push an integer
  } else {
    _pf.SINT(node->value()); // Integer literal is on the DATA segment
  }
}

void udf::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  // Generate the string
  _pf.RODATA(); // Strings are DATA readonly
  _pf.ALIGN(); // Make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // Give the string a name
  _pf.SSTRING(node->value()); // Output string characters

  // Leave the address on the stack
  if (_function) {
    // Local variable initializer
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  } else {
    // Global variable initializer
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // Determine the value
  _pf.NEG(); // 2-complement
}

void udf::postfix_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // Determine the value
}

void udf::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.INT(0);
  _pf.EQ();
  //_pf.NOT();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  
  if (node->left()->type()->name() == cdk::TYPE_TENSOR) {
    node->right()->accept(this, lvl + 2);
    if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
      _pf.I2D();
    } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
      _pf.INT(3);
      _pf.SHTL();
    }
    node->left()->accept(this, lvl + 2);
    if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) {
      _pf.I2D();
    } else if (node->type()->name() == cdk::TYPE_POINTER && node->left()->type()->name() == cdk::TYPE_INT) {
      _pf.INT(3);
      _pf.SHTL();
    }
  } else {
    node->left()->accept(this, lvl + 2);
    if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) {
      _pf.I2D();
    } else if (node->type()->name() == cdk::TYPE_POINTER && node->left()->type()->name() == cdk::TYPE_INT) {
      _pf.INT(3);
      _pf.SHTL();
    }
    
    node->right()->accept(this, lvl + 2);
    if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
      _pf.I2D();
    } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
      _pf.INT(3);
      _pf.SHTL();
    }
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE)
    _pf.DADD();
  else if (node->type()->name() == cdk::TYPE_TENSOR) {
    _functions_to_declare.insert("tensor_add");
    _pf.EXTERN("tensor_add");
    _pf.CALL("tensor_add");
  }
  else
    _pf.ADD();
}

void udf::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->left()->type()->name() == cdk::TYPE_TENSOR) {
    node->right()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  
    node->left()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  } else {
    node->left()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();
    
    node->right()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.I2D();
  } else if (node->type()->name() == cdk::TYPE_POINTER && node->right()->type()->name() == cdk::TYPE_INT) {
    _pf.INT(3);
    _pf.SHTL();
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE)
    _pf.DSUB();
  else if (node->type()->name() == cdk::TYPE_TENSOR) {
    _functions_to_declare.insert("tensor_sub");
    _pf.EXTERN("tensor_sub");
    _pf.CALL("tensor_sub");
  }
  else
    _pf.SUB();
}

void udf::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->left()->type()->name() == cdk::TYPE_TENSOR) {
    node->right()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  
    node->left()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  } else {
    node->left()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();
    
    node->right()->accept(this, lvl + 2);
    if ((node->type()->name() == cdk::TYPE_DOUBLE || node->type()->name() == cdk::TYPE_TENSOR) && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DMUL();
  } else if (node->left()->type()->name() == cdk::TYPE_TENSOR && node->right()->type()->name() == cdk::TYPE_TENSOR) {
    _pf.SWAP32();
    _functions_to_declare.insert("tensor_mul");
    _pf.EXTERN("tensor_mul");
    _pf.CALL("tensor_mul");
    _pf.TRASH(8);
    _pf.LDFVAL32();
  } else if (node->left()->type()->name() == cdk::TYPE_TENSOR && (node->right()->type()->name() == cdk::TYPE_INT || node->right()->type()->name() == cdk::TYPE_DOUBLE)) {
    _functions_to_declare.insert("tensor_mul_scalar");
    _pf.EXTERN("tensor_mul_scalar");
    _pf.CALL("tensor_mul_scalar");
    _pf.TRASH(12);
    _pf.LDFVAL32();
  } else if ((node->left()->type()->name() == cdk::TYPE_INT || node->left()->type()->name() == cdk::TYPE_DOUBLE) && node->right()->type()->name() == cdk::TYPE_TENSOR) {
    _functions_to_declare.insert("tensor_mul_scalar");
    _pf.EXTERN("tensor_mul_scalar");
    _pf.CALL("tensor_mul_scalar");
    _pf.TRASH(12);
    _pf.LDFVAL32();
  } else {
    _pf.MUL();
  }
}

void udf::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) _pf.I2D();


  if (node->type()->name() == cdk::TYPE_DOUBLE)
    _pf.DDIV();
  else if (node->type()->name() == cdk::TYPE_TENSOR) {
    _functions_to_declare.insert("tensor_div");
    _pf.EXTERN("tensor_div");
    _pf.CALL("tensor_div");
  }
  else
    _pf.DIV();
}

void udf::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  _pf.MOD();
}

void udf::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.LT();
}

void udf::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.LE();
}

void udf::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.GE();
}

void udf::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.GT();
}

void udf::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.NE();
}

void udf::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == cdk::TYPE_INT && node->right()->type()->name() == cdk::TYPE_DOUBLE) _pf.I2D();

  _pf.EQ();
}

void udf::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

void udf::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_tensor_capacity_node(udf::tensor_capacity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);

  _functions_to_declare.insert("tensor_size");
  _pf.EXTERN("tensor_size");
  _pf.CALL("tensor_size");
  _pf.TRASH(4);
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_rank_node(udf::tensor_rank_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);

  _functions_to_declare.insert("tensor_get_n_dims");
  _pf.EXTERN("tensor_get_n_dims");
  _pf.CALL("tensor_get_n_dims");
  _pf.TRASH(4);
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_dim_node(udf::tensor_dim_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->dimension()->accept(this, lvl + 2);
  node->argument()->accept(this, lvl + 2);

  _functions_to_declare.insert("tensor_get_dim_size");
  _pf.EXTERN("tensor_get_dim_size");
  _pf.CALL("tensor_get_dim_size");
  _pf.TRASH(8);
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_dims_node(udf::tensor_dims_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _functions_to_declare.insert("tensor_get_dims");
  _pf.EXTERN("tensor_get_dims");
  _pf.CALL("tensor_get_dims");
  _pf.TRASH(8);
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_index_node(udf::tensor_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->position()->accept(this, lvl + 2);
  node->base()->accept(this, lvl + 2);

  _functions_to_declare.insert("tensor_getptr");
  _pf.EXTERN("tensor_getptr");
  _pf.CALL("tensor_getptr");
  _pf.TRASH((node->position()->size() + 1)* 4);
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_reshape_node(udf::tensor_reshape_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  for (size_t ax = node->arguments()->size(); ax > 0; ax--) {
    auto expr = dynamic_cast<cdk::integer_node*>(node->arguments()->node(ax - 1));
    if (expr) {
      _pf.INT(expr->value());
    } else {
      std::cerr << "Argumento não é integer_node!" << std::endl;
    }
  }

  _pf.INT(node->arguments()->size());
  node->tensor()->accept(this, lvl + 2);

  _functions_to_declare.insert("tensor_reshape");
  _pf.EXTERN("tensor_reshape");
  _pf.EXTERN("tensor_reshape");
  _pf.CALL("tensor_reshape");
  _pf.TRASH((node->arguments()->size() + 2) * 4);
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_contract_node(udf::tensor_contract_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->right()->accept(this, lvl + 2);
  node->left()->accept(this, lvl + 2);

  _functions_to_declare.insert("tensor_matmul");
  _pf.EXTERN("tensor_matmul");
  _pf.CALL("tensor_matmul");
  _pf.TRASH(8);
  _pf.LDFVAL32();
}

void udf::postfix_writer::tensor_elements(cdk::sequence_node *seq, int &offset, int lvl) {
  for (size_t i = 0; i < seq->size(); ++i) {
    auto node = seq->node(i);
    if (auto sub_seq = dynamic_cast<cdk::sequence_node *>(node)) {
      tensor_elements(sub_seq, offset, lvl);
    } else if (auto tensor = dynamic_cast<udf::tensor_node *>(node)) {
      tensor_elements(tensor->elements(), offset, lvl);
    } else if (auto expr = dynamic_cast<cdk::expression_node *>(node)) {
      _pf.ADDR("temp");
      _pf.LDINT();
      _pf.INT(offset);
      expr->accept(this, lvl + 2);
      if (expr->type()->name() == cdk::TYPE_INT) _pf.I2D();
      _functions_to_declare.insert("tensor_put");
      _pf.EXTERN("tensor_put");
      _pf.CALL("tensor_put");
      _pf.TRASH(16);
      offset += 1;
    }
  }
}

std::vector<size_t> udf::postfix_writer::tensor_dimensions(cdk::sequence_node *seq) {
  std::vector<size_t> dims;
  while (seq) {
    dims.push_back(seq->size());
    if (seq->size() == 0)
      break;
    auto first = seq->node(0);
    seq = dynamic_cast<cdk::sequence_node *>(first);
    if (!seq) {
      auto t = dynamic_cast<udf::tensor_node *>(first);
      if (!t)
        break;
      seq = t->elements();
    }
  }
  return dims;
}

void udf::postfix_writer::do_tensor_node(udf::tensor_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (!_initMemory) {
    _functions_to_declare.insert("mem_init");
    _pf.CALL("mem_init");
    _initMemory = true;

    _pf.BSS();
    _pf.ALIGN();
    _pf.LABEL("temp");
    _pf.SALLOC(4);
    _pf.TEXT();
    _pf.ALIGN();
  }

  std::vector<size_t> dims = tensor_dimensions(node->elements());
  for (int i = dims.size() - 1; i >= 0; i--) {
    _pf.INT(dims[i]);
  }
  _pf.INT(dims.size());

  _functions_to_declare.insert("tensor_create");
  _pf.EXTERN("tensor_create");
  _pf.CALL("tensor_create");
  _pf.TRASH((dims.size() + 1) * 4);
  _pf.LDFVAL32();
  _pf.ADDR("temp");
  _pf.STINT();

  int offset = 0;
  tensor_elements(node->elements(), offset, lvl);

  _pf.ADDR("temp");
  _pf.LDINT();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_address_of_node(udf::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_alloc_node(udf::alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(3);
  _pf.SHTL();
  _pf.ALLOC(); // Allocate
  _pf.SP(); // Put base pointer in stack
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  const std::string &id = node->name();
  auto symbol = _symtab.find(id);
  if(!symbol) {
    std::cerr << "ERROR: variable '" << id << "' not found" << std::endl;
    return;
  }
  if (symbol->global()) {
    _pf.ADDR(symbol->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void udf::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.LDDOUBLE();
  } else {
    // Integers, pointers, strings, tensors
    _pf.LDINT();
  }
}

void udf::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE) {
    if (node->rvalue()->type()->name() == cdk::TYPE_INT) _pf.I2D();
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.STDOUBLE(); // Store double
  } else {
    _pf.STINT(); // Store int
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_function_declaration_node(udf::function_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody || _inFunctionArgs) {
    std::cerr << "ERROR: cannot declare function in body or in args" << std::endl;
    return;
  }

  if (!new_symbol()) return;

  auto function = new_symbol();
  _functions_to_declare.insert(function->name());
  reset_new_symbol();
}

void udf::postfix_writer::do_function_definition_node(udf::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody || _inFunctionArgs) {
    std::cerr << "ERROR: cannot define function in body or in args" << std::endl;
    return;
  }

  // Remember symbol so that args and body know
  _function = new_symbol();
  _functions_to_declare.erase(_function->name());
  reset_new_symbol();

  _currentBodyRetLabel = mklbl(++_lbl);

  _offset = 8; // Prepare for arguments (4: remember to account for return address)
  _symtab.push(); // Scope of args

  if (node->arguments()->size() > 0) {
    _inFunctionArgs = true;
    for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
      cdk::basic_node *arg = node->arguments()->node(ix);
      if (arg == nullptr) break; // This means an empty sequence of arguments
      arg->accept(this, 0); // The function symbol is at the top of the stack
    }
    _inFunctionArgs = false;
  }

  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == tPUBLIC) _pf.GLOBAL(_function->name(), _pf.FUNC());
  _pf.LABEL(_function->name());

  // Compute stack size to be reserved for local variables
  frame_size_calculator lsc(_compiler, _symtab, _function);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize()); // Total stack size reserved for local variables

  _offset = 0; // Prepare for local variable

  _inFunctionBody = true;
  os() << "        ;; before body " << std::endl;
  node->block()->accept(this, lvl + 4); // Block has its own scope
  os() << "        ;; after body " << std::endl;
  _inFunctionBody = false;
  _returnSeen = false;

  _pf.LABEL(_currentBodyRetLabel);
  _pf.LEAVE();
  _pf.RET();

  _symtab.pop(); // Scope of arguments

  if (node->identifier() == "udf") {
    // Declare external functions
    for (std::string s : _functions_to_declare)
      _pf.EXTERN(s);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_return_node(udf::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // Should not reach here without returning a value (if not void)
  if (_function->type()->name() != cdk::TYPE_VOID) {
    node->retval()->accept(this, lvl + 2);

    if (_function->type()->name() == cdk::TYPE_INT || _function->type()->name() == cdk::TYPE_STRING
        || _function->type()->name() == cdk::TYPE_POINTER || _function->type()->name() == cdk::TYPE_TENSOR) {
      _pf.STFVAL32();
    } else if (_function->type()->name() == cdk::TYPE_DOUBLE) {
      if (node->retval()->type()->name() == cdk::TYPE_INT) _pf.I2D();
      _pf.STFVAL64();
    } else if (_function->type()->name() == cdk::TYPE_STRUCT) {
    } else {
      std::cerr << node->lineno() << ": should not happen: unknown return type" << std::endl;
    }
  }

  _pf.JMP(_currentBodyRetLabel);
  _returnSeen = true;
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_variable_declaration_node(udf::variable_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto id = node->identifier();
  std::cout << "INITIAL OFFSET: " << _offset << std::endl;

  int offset = 0, typesize = node->type()->size(); // In bytes
  std::cout << "ARG: " << id << ", " << typesize << std::endl;
  if (_inFunctionBody) {
    std::cout << "IN BODY" << std::endl;
    _offset -= typesize;
    offset = _offset;
  } else if (_inFunctionArgs) {
    std::cout << "IN ARGS" << std::endl;
    offset = _offset;
    _offset += typesize;
  } else {
    std::cout << "GLOBAL!" << std::endl;
    offset = 0; // Global variable
  }
  std::cout << "OFFSET: " << id << ", " << offset << std::endl;
  auto symbol = new_symbol();
  if (symbol) {
    std::cerr << node->lineno() << ": ERROR: No symbol created for variable '" << id << "'" << std::endl;
    symbol->set_offset(offset);
    reset_new_symbol();
  }

  if (_inFunctionBody) {
    // If we are dealing with local variables, then no action is needed
    // unless an initializer exists
    if (node->initializer()) {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_TENSOR)) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        std::cerr << "cannot initialize" << std::endl;
      }
    } else if (node->is_typed(cdk::TYPE_TENSOR)) {
      if(!_initMemory) {
        _functions_to_declare.insert("mem_init");
        _pf.CALL("mem_init");
        _initMemory = true;
  
        _pf.BSS();
        _pf.ALIGN();
        _pf.LABEL("temp");
        _pf.SALLOC(4);
        _pf.TEXT();
        _pf.ALIGN();
      }

      auto dims = std::dynamic_pointer_cast<cdk::tensor_type>(node->type())->dims();

      for (int i = dims.size() - 1; i >= 0; i--) {
        _pf.INT(dims[i]);
      }
      _pf.INT(dims.size());

      _functions_to_declare.insert("tensor_create");
      _pf.EXTERN("tensor_create");
      _pf.CALL("tensor_create");
      _pf.TRASH((dims.size() + 1) * 4);
      _pf.LDFVAL32();
      _pf.LOCAL(symbol->offset());
      _pf.STINT();
    }
  } else {
    if (!_function) {
      if (node->initializer() == nullptr) {
        _pf.BSS();
        _pf.ALIGN();
        _pf.LABEL(id);
        _pf.SALLOC(typesize);
      } else {
        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_TENSOR)) {
          if (node->constant()) {
            _pf.RODATA();
          } else {
            _pf.DATA();
          }
          _pf.ALIGN();
          _pf.LABEL(id);

          if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_TENSOR)) {
            node->initializer()->accept(this, lvl);
          } else if (node->is_typed(cdk::TYPE_POINTER)) {
            node->initializer()->accept(this, lvl);
          } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
            if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
              node->initializer()->accept(this, lvl);
            } else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
              cdk::integer_node *dclini = dynamic_cast<cdk::integer_node*>(node->initializer());
              cdk::double_node ddi(dclini->lineno(), dclini->value());
              ddi.accept(this, lvl);
            } else {
              std::cerr << node->lineno() << ": '" << id << "' has bad initializer for real value\n";
              _errors = true;
            }
          }
        } else if (node->is_typed(cdk::TYPE_STRING)) {
          if (node->constant()) {
            int litlbl;
            _pf.RODATA();
            _pf.ALIGN();
            _pf.LABEL(mklbl(litlbl = ++_lbl));
            _pf.SSTRING(dynamic_cast<cdk::string_node*>(node->initializer())->value());
            _pf.ALIGN();
            _pf.LABEL(id);
            _pf.SADDR(mklbl(litlbl));
          } else {
            _pf.DATA();
            _pf.ALIGN();
            _pf.LABEL(id);
            node->initializer()->accept(this, lvl);
          }
        } else {
          std::cerr << node->lineno() << ": '" << id << "' has unexpected initializer\n";
          _errors = true;
        }
      }
    }
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_evaluation_node(udf::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.TRASH(node->argument()->type()->size());
}

void udf::postfix_writer::do_write_node(udf::write_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
    auto child = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ix));

    std::shared_ptr<cdk::basic_type> etype = child->type();
    child->accept(this, lvl); // Expression to print
    _pf.ALIGN();
    if (etype->name() == cdk::TYPE_INT) {
      _functions_to_declare.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // Trash int
    } else if (etype->name() == cdk::TYPE_DOUBLE) {
      _functions_to_declare.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // Trash double
    } else if (etype->name() == cdk::TYPE_STRING) {
      _functions_to_declare.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4);
    } else if (etype->name() == cdk::TYPE_TENSOR) {
      _functions_to_declare.insert("tensor_print");
      _pf.EXTERN("tensor_print");
      _pf.CALL("tensor_print");
      _pf.TRASH(4); // Trash pointer to tensor after print
    } else {
      std::cerr << "cannot print expression of unknown type" << std::endl;
      return;
    }
  }

  if (node->newline()) {
    _functions_to_declare.insert("println");
    _pf.ALIGN();
    _pf.CALL("println");
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_input_node(udf::input_node * const node, int lvl) {
  if (_lvalueType == cdk::TYPE_DOUBLE) {
    _functions_to_declare.insert("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else if (_lvalueType == cdk::TYPE_INT) {
    _functions_to_declare.insert("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else {
    std::cerr << "FATAL: " << node->lineno() << ": cannot read type" << std::endl;
    return;
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_for_node(udf::for_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _forIni.push(++_lbl); // After init, before body
  _forStep.push(++_lbl); // After intruction
  _forEnd.push(++_lbl); // After for

  os() << "        ;; FOR initialize" << std::endl;
  // Initialize: be careful with variable declarations:
  // they are done here, but the space is occupied in the function
  _symtab.push();
  _inForInit = true;  // Remember this for local declarations

  // Initialize
  node->declaration()->accept(this, lvl + 2);

  os() << "        ;; FOR test" << std::endl;
  // Prepare to test
  _pf.ALIGN();
  _pf.LABEL(mklbl(_forIni.top()));
  node->condition()->accept(this, lvl + 2);
  _pf.JZ(mklbl(_forEnd.top()));

  os() << "        ;; FOR instruction" << std::endl;
  // Execute instruction
  node->block()->accept(this, lvl + 2);

  os() << "        ;; FOR increment" << std::endl;
  // Prepare to increment
  _pf.ALIGN();
  _pf.LABEL(mklbl(_forStep.top()));
  node->increment()->accept(this, lvl + 2);

  os() << "        ;; FOR jump to test" << std::endl;
  _pf.JMP(mklbl(_forIni.top()));

  os() << "        ;; FOR end" << std::endl;
  _pf.ALIGN();
  _pf.LABEL(mklbl(_forEnd.top()));

  _inForInit = false;  // Remember this for local declarations
  _symtab.pop();

  _forIni.pop();
  _forStep.pop();
  _forEnd.pop();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_if_node(udf::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

void udf::postfix_writer::do_if_else_node(udf::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl_else, lbl_end;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl_else = lbl_end = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  if (node->elseblock()) {
    _pf.JMP(mklbl(lbl_end = ++_lbl));
    _pf.LABEL(mklbl(lbl_else));
    node->elseblock()->accept(this, lvl + 2);
  }
  _pf.LABEL(mklbl(lbl_end));
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_function_call_node(udf::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = _symtab.find(node->identifier());

  size_t argsSize = 0;
  if (node->arguments()->size() > 0) {
    for (int ax = node->arguments()->size() - 1; ax >= 0; ax--) {
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ax));
      arg->accept(this, lvl + 2);
      if (symbol->argument_is_typed(ax, cdk::TYPE_DOUBLE) && arg->is_typed(cdk::TYPE_INT)) {
        _pf.I2D();
      }
      argsSize += symbol->argument_size(ax);
    }
  }
  _pf.CALL(node->identifier());
  if (argsSize != 0) {
    _pf.TRASH(argsSize);
  }

  if (symbol->is_typed(cdk::TYPE_INT) || symbol->is_typed(cdk::TYPE_POINTER) || symbol->is_typed(cdk::TYPE_STRING)) {
    _pf.LDFVAL32();
  } else if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else {
    // Cannot happen!
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_index_node(udf::index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->base()) {
    node->base()->accept(this, lvl);
  } else {
    if (_function) {
      _pf.LOCV(-_function->type()->size());
    } else {
      std::cerr << "FATAL: " << node->lineno() << ": trying to use return value outside function" << std::endl;
    }
  }
  node->index()->accept(this, lvl);
  _pf.INT(3);
  _pf.SHTL();
  _pf.ADD(); // Add pointer and index
}

void udf::postfix_writer::do_block_node(udf::block_node * const node, int lvl) {
  _symtab.push();
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_nullptr_node(udf::nullptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // A pointer is a 32-bit integer
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_continue_node(udf::continue_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forStep.top())); // Jump to next cycle
  } else
    std::cerr << "ERROR: 'continue' outside 'for'" << std::endl;
}

void udf::postfix_writer::do_break_node(udf::break_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forEnd.top())); // jump to for end
  } else
  std::cerr << "ERROR: 'break' outside 'for'" << std::endl;
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_sizeof_node(udf::sizeof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->argument()->is_typed(cdk::TYPE_TENSOR)) {
    _pf.EXTERN("tensor_size");
    node->argument()->accept(this, lvl + 2);
    _pf.CALL("tensor_size");
    _pf.LDFVAL32();
    _pf.INT(8); // Size of double
    _pf.MUL();
  } else {
    _pf.INT(node->argument()->type()->size());
  }
}

//---------------------------------------------------------------------------
