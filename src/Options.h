#ifndef MOSES_OPTIONS_H
#define MOSES_OPTIONS_H

#include <map>
#include <string>
#include <node.h>
#include "moses/Parameter.h"

using namespace std;
using namespace v8;
using namespace Moses;

class OptionsParameter : public Parameter {
  public:
    inline bool IsValid(string name) {
      return m_valid.find(name) != m_valid.end();
    }
};

class Options {
  public:
    Options();
    Options(Handle<Object> initObject);
    Handle<Value> OverwriteParams(OptionsParameter& params);

  private:
    void InitDefaults();

    map<string, string> options;
};

#endif
