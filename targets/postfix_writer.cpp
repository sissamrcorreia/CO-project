#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
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
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.DOUBLE(node->value()); // push a double
  } else {
    _pf.SDOUBLE(node->value());    // double is on the DATA segment
  }
}

void udf::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.INT(node->value()); // push an integer
  } else {
    _pf.SINT(node->value()); // integer literal is on the DATA segment
  }
}

void udf::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  /* leave the address on the stack */
  if (_function) {
    // local variable initializer
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  } else {
    // global variable initializer
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.NEG(); // 2-complement
}

void udf::postfix_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
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
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TENSOR)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.EXTERN("tensor_add_scalar");
      _pf.CALL("tensor_add_scalar");
      _pf.LDFVAL32();
    } else {
      _pf.EXTERN("tensor_add");
      _pf.CALL("tensor_add");
      _pf.LDFVAL32();
    }
  } else {
    _pf.ADD(); // Integer addition
  }
}
void udf::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TENSOR)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.EXTERN("tensor_sub_scalar");
      _pf.CALL("tensor_sub_scalar");
      _pf.LDFVAL32();
    } else {
      _pf.EXTERN("tensor_sub");
      _pf.CALL("tensor_sub");
      _pf.LDFVAL32();
    }
  } else {
    _pf.SUB(); // Integer subtraction
  }
}
void udf::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TENSOR)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.EXTERN("tensor_mul_scalar");
      _pf.CALL("tensor_mul_scalar");
      _pf.LDFVAL32();
    } else {
      _pf.EXTERN("tensor_mul");
      _pf.CALL("tensor_mul");
      _pf.LDFVAL32();
    }
  } else {
    _pf.MUL(); // Integer multiplication
  }
}
void udf::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_TENSOR)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.EXTERN("tensor_div_scalar");
      _pf.CALL("tensor_div_scalar");
      _pf.LDFVAL32();
    } else {
      _pf.EXTERN("tensor_div");
      _pf.CALL("tensor_div");
      _pf.LDFVAL32();
    }
  } else {
    _pf.DIV(); // Integer division
  }
}
void udf::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void udf::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
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
  _pf.EXTERN("tensor_size");
  _pf.CALL("tensor_size");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_rank_node(udf::tensor_rank_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.EXTERN("tensor_get_n_dims");
  _pf.CALL("tensor_get_n_dims");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_dim_node(udf::tensor_dim_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  node->dimension()->accept(this, lvl + 2);
  _pf.EXTERN("tensor_get_dim_size");
  _pf.CALL("tensor_get_dim_size");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_dims_node(udf::tensor_dims_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.EXTERN("tensor_get_dims");
  _pf.CALL("tensor_get_dims");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_index_node(udf::tensor_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl + 2);
  auto indices = node->position();
  for (size_t i = 0; i < indices->size(); i++) indices->node(i)->accept(this, lvl + 2);
  _pf.EXTERN("tensor_get");
  _pf.CALL("tensor_get");
  _pf.LDFVAL64();
}

void udf::postfix_writer::do_tensor_reshape_node(udf::tensor_reshape_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->tensor()->accept(this, lvl + 2);
  auto new_dims = node->arguments();
  _pf.INT(new_dims->size());
  for (size_t i = 0; i < new_dims->size(); i++) new_dims->node(i)->accept(this, lvl + 2);
  _pf.EXTERN("tensor_reshape");
  _pf.CALL("tensor_reshape");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_contract_node(udf::tensor_contract_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  _pf.EXTERN("tensor_matmul");
  _pf.CALL("tensor_matmul");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_node(udf::tensor_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto elements = node->elements();
  if (!elements || elements->size() == 0) {
    std::cerr << "ERROR: Empty tensor literal" << std::endl;
    return;
  }

  // Get dimensions
  size_t n_dims = 1;
  size_t dim_size = elements->size();
  auto first_subseq = dynamic_cast<cdk::sequence_node*>(elements->node(0));
  if (first_subseq && first_subseq->size() > 0) {
    n_dims = 2;
  }

  _pf.INT(n_dims);
  _pf.INT(dim_size);
  if (n_dims == 2) {
    _pf.INT(first_subseq->size());
  }

  _pf.EXTERN("tensor_create");
  _pf.CALL("tensor_create");
  _pf.LDFVAL32();

  for (size_t i = 0; i < elements->size(); i++) {
    auto subseq = dynamic_cast<cdk::sequence_node*>(elements->node(i));
    for (size_t j = 0; j < subseq->size(); j++) {
      subseq->node(j)->accept(this, lvl + 2);
      _pf.DUP32();
      _pf.INT(i);
      if (n_dims == 2) {
        _pf.INT(j);
      }
      _pf.EXTERN("tensor_set");
      _pf.CALL("tensor_set");
      _pf.TRASH(4);
    }
  }
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
  _pf.ALLOC(); // allocate
  _pf.SP(); // put base pointer in stack
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // simplified generation: all variables are global
  _pf.ADDR(node->name());
}

void udf::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl); // Load address
  auto symbol = _symtab.find(dynamic_cast<cdk::variable_node*>(node->lvalue())->name());
  if (!symbol) {
    std::cerr << "ERROR: Undeclared variable in rvalue at line " << node->lineno() << std::endl;
    exit(1);
  }
  if (symbol->type()->name() == cdk::TYPE_INT || symbol->type()->name() == cdk::TYPE_STRING || symbol->type()->name() == cdk::TYPE_POINTER) {
    _pf.LDINT(); // Load 32-bit value
  } else if (symbol->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.LDDOUBLE(); // Load 64-bit double
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
  } else if (node->lvalue()->is_typed(cdk::TYPE_TENSOR)) {
    _pf.STFVAL32(); // Store Tensor*
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

  // remember symbol so that args and body know
  _function = new_symbol();
  _functions_to_declare.erase(_function->name());
  reset_new_symbol();

  _currentBodyRetLabel = mklbl(++_lbl);

  _offset = 8; // prepare for arguments (4: remember to account for return address)
  _symtab.push(); // scope of args

  if (node->arguments()->size() > 0) {
    _inFunctionArgs = true; //FIXME
    for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
      cdk::basic_node *arg = node->arguments()->node(ix);
      if (arg == nullptr) break; // this means an empty sequence of arguments
      arg->accept(this, 0); // the function symbol is at the top of the stack
    }
    _inFunctionArgs = false; //FIXME
  }

  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == tPUBLIC) _pf.GLOBAL(_function->name(), _pf.FUNC());
  _pf.LABEL(_function->name());

  // compute stack size to be reserved for local variables
  frame_size_calculator lsc(_compiler, _symtab, _function);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize()); // total stack size reserved for local variables

  _offset = 0; // prepare for local variable

  // FIXME: the following flag is a slight hack: it won't work with nested functions
  _inFunctionBody = true;
  os() << "        ;; before body " << std::endl;
  node->block()->accept(this, lvl + 4); // block has its own scope
  os() << "        ;; after body " << std::endl;
  _inFunctionBody = false;
  _returnSeen = false;

  _pf.LABEL(_currentBodyRetLabel);
  _pf.LEAVE();
  _pf.RET();

  _symtab.pop(); // scope of arguments

  if (node->identifier() == "udf") {
    // declare external functions
    for (std::string s : _functions_to_declare)
      _pf.EXTERN(s);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_return_node(udf::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  // should not reach here without returning a value (if not void)
  if (_function->type()->name() != cdk::TYPE_VOID) {
    node->retval()->accept(this, lvl + 2);

    if (_function->type()->name() == cdk::TYPE_INT || _function->type()->name() == cdk::TYPE_STRING
        || _function->type()->name() == cdk::TYPE_POINTER) { // TODO: tensor
      _pf.STFVAL32();
    } else if (_function->type()->name() == cdk::TYPE_DOUBLE) {
      if (node->retval()->type()->name() == cdk::TYPE_INT) _pf.I2D();
      _pf.STFVAL64();
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
  // type size?
  int offset = 0, typesize = node->type()->size(); // in bytes
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
    offset = 0; // global variable
  }
  std::cout << "OFFSET: " << id << ", " << offset << std::endl;
  auto symbol = new_symbol();
  if (symbol) {
    symbol->set_offset(offset);
    reset_new_symbol();
  }
  if (_inFunctionBody) {
    // if we are dealing with local variables, then no action is needed
    // unless an initializer exists
    if (node->initializer()) {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER)) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else if (node->is_typed(cdk::TYPE_TENSOR)) {
        // single var initialized with tensor
        // FIXME
      } else {
        std::cerr << "cannot initialize" << std::endl;
      }
    }
  } else { //TODO tensor
    if (!_function) {
      if (node->initializer() == nullptr) {
        _pf.BSS();
        _pf.ALIGN();
        _pf.LABEL(id);
        _pf.SALLOC(typesize);
      } else {
        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER)) {
          if (false) { // FIXME era node->isConstant()
            _pf.RODATA();
          } else {
            _pf.DATA();
          }
          _pf.ALIGN();
          _pf.LABEL(id);
          if (node->is_typed(cdk::TYPE_INT)) {
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
          if (false) { // FIXME era node->isConstant()
            int litlbl;
            // FIXME: string literal initializers must be emitted before the string identifier
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
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    _pf.TRASH(4); // delete the evaluated value
  } else if (node->argument()->is_typed(cdk::TYPE_STRING)) {
    _pf.TRASH(4); // delete the evaluated value's address
  } else {
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
}

void udf::postfix_writer::do_write_node(udf::write_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
    auto child = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ix));

    std::shared_ptr<cdk::basic_type> etype = child->type();
    child->accept(this, lvl); // expression to print
    _pf.ALIGN();
    if (etype->name() == cdk::TYPE_INT) {
      _functions_to_declare.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // trash int
    } else if (etype->name() == cdk::TYPE_DOUBLE) {
      _functions_to_declare.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // trash double
    } else if (etype->name() == cdk::TYPE_STRING) {
      _functions_to_declare.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4);
    } else if (etype->name() == cdk::TYPE_TENSOR) {
      _functions_to_declare.insert("print_tensor");
      _pf.CALL("print_tensor");
      _pf.TRASH(4);
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

  _forIni.push(++_lbl); // after init, before body
  _forStep.push(++_lbl); // after intruction
  _forEnd.push(++_lbl); // after for

  os() << "        ;; FOR initialize" << std::endl;
  // initialize: be careful with variable declarations:
  // they are done here, but the space is occupied in the function
  _symtab.push();
  _inForInit = true;  // remember this for local declarations

  // initialize
  node->declaration()->accept(this, lvl + 2);

  os() << "        ;; FOR test" << std::endl;
  // prepare to test
  _pf.ALIGN();
  _pf.LABEL(mklbl(_forIni.top()));
  node->condition()->accept(this, lvl + 2);
  _pf.JZ(mklbl(_forEnd.top()));

  os() << "        ;; FOR instruction" << std::endl;
  // execute instruction
  node->increment()->accept(this, lvl + 2);

  os() << "        ;; FOR increment" << std::endl;
  // prepare to increment
  _pf.ALIGN();
  _pf.LABEL(mklbl(_forStep.top()));
  node->block()->accept(this, lvl + 2);

  os() << "        ;; FOR jump to test" << std::endl;
  _pf.JMP(mklbl(_forIni.top()));

  os() << "        ;; FOR end" << std::endl;
  _pf.ALIGN();
  _pf.LABEL(mklbl(_forEnd.top()));

  _inForInit = false;  // remember this for local declarations
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
    // cannot happen!
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
  _pf.ADD(); // add pointer and index
}

void udf::postfix_writer::do_block_node(udf::block_node * const node, int lvl) {
  _symtab.push();
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_nullptr_node(udf::nullptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; // a pointer is a 32-bit integer
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_continue_node(udf::continue_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forStep.top())); // jump to next cycle
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
