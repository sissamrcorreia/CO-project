#pragma once

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace udf {

  class symbol {
    int _offset = 0; // 0 (zero) means global variable/function

    std::string _name;
    int _value; // hack!
    bool _constant;
    int _qualifier; // qualifiers: public, forward, "private" (i.e., none)
    std::shared_ptr<cdk::basic_type> _type;
    bool _initialized;
    bool _function; // false for variables
    bool _forward = false;
    
    std::vector<std::shared_ptr<cdk::basic_type>> _argument_types;


  public:
    symbol(bool constant, int qualifier, std::shared_ptr<cdk::basic_type> type, const std::string &name, bool initialized,
           bool function, bool forward = false) :
        _name(name), _value(0), _constant(constant), _qualifier(qualifier), _type(type), _initialized(initialized), _function(
            function), _forward(forward) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    int value() const {
      return _value;
    }
    int value(int v) {
      return _value = v;
    }
    bool constant() const {
      return _constant;
    }
    int qualifier() const {
      return _qualifier;
    }
    
    int offset() const {
      return _offset;
    }
    void set_offset(int offset) {
      _offset = offset;
    }
    bool argument_is_typed(size_t ax, cdk::typename_type name) const {
      return _argument_types[ax]->name() == name;
    }
    std::shared_ptr<cdk::basic_type> argument_type(size_t ax) const {
      return _argument_types[ax];
    }

    size_t argument_size(size_t ax) const {
      return _argument_types[ax]->size();
    }

    void set_argument_types(const std::vector<std::shared_ptr<cdk::basic_type>> &types) {
      _argument_types = types;
    }

    size_t number_of_arguments() const {
      return _argument_types.size();
    }
    bool isFunction() const {
      return _function;
    }

    bool forward() const {
      return _forward;
    }
    
    void set_type(std::shared_ptr<cdk::basic_type> t) {
      _type = t;
    }

    bool global() const {
      return _offset == 0;
    }
  };


  inline auto make_symbol(bool constant, int qualifier, std::shared_ptr<cdk::basic_type> type, const std::string &name,
                          bool initialized, bool function, bool forward = false) {
    return std::make_shared<symbol>(constant, qualifier, type, name, initialized, function, forward);
  }

} // udf
