#include "Options.h"

using namespace std;
using namespace v8;
using namespace Moses;

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

Handle<Value> Options::OverwriteParams(OptionsParameter& params) {
  HandleScope scope;
  map<string, string>::const_iterator it = options.begin();
  vector<string> value;
  while (it != options.end()) {
    pair<string, string> pair = *it++;
    if (!params.IsValid(pair.first)) {
      return ThrowException(Exception::Error(String::New(("Option name \"" + pair.first + "\" is not supported.").c_str())));
    }
    value.clear();
    value.push_back(pair.second);
    params.OverwriteParam(pair.first, value);
  }
  return scope.Close(Local<Value>());
}

void Options::InitDefaults() {
  options["verbose"] = "0";
}
