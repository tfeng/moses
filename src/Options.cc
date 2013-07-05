#include "Options.h"

using namespace std;
using namespace v8;
using namespace Moses;

namespace MosesJS {

Options::Options() {
  InitDefaults();
}

Options::Options(Handle<Object> initObject) {
  InitDefaults();
  Local<Array> propertyNames = initObject->GetPropertyNames();
  for (size_t i = 0; i < propertyNames->Length(); i++) {
    Handle<String> keyString = propertyNames->Get((uint32_t) i)->ToString();
    string key = *String::Utf8Value(keyString);
    string value = *String::Utf8Value(initObject->Get(keyString)->ToString());
    options[key] = value;
  }
}

string Options::OverwriteParams(OptionsParameter& params) {
  map<string, string>::const_iterator it = options.begin();
  vector<string> value;
  while (it != options.end()) {
    pair<string, string> pair = *it++;
    if (!params.IsValid(pair.first)) {
      return "Option name \"" + pair.first + "\" is not supported.";
    }
    value.clear();
    value.push_back(pair.second);
    params.OverwriteParam(pair.first, value);
  }
  return "";
}

void Options::InitDefaults() {
  options["verbose"] = "0";
}

}