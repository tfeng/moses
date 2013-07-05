#ifndef MOSES_OPTIONS_H
#define MOSES_OPTIONS_H

#include <fstream>
#include <map>
#include <string>
#include <node.h>
#include "moses/Parameter.h"

using namespace std;
using namespace v8;
using namespace Moses;

namespace MosesJS {

class OptionsParameter : public Parameter {
  public:
    inline bool IsValid(string name) {
      return m_valid.find(name) != m_valid.end();
    }

    inline bool LoadParam(const string &filePath) {
      ifstream file(filePath.c_str());
      if (file.good()) {
        file.close();
        return Parameter::LoadParam(filePath);
      } else {
        file.close();
        return false;
      }
    }
};

class Options {
  public:
    Options();
    Options(Handle<Object> initObject);
    string OverwriteParams(OptionsParameter& params);

  private:
    void InitDefaults();

    map<string, string> options;
};

}

#endif
