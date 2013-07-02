#include <node.h>
#include "moses/Hypothesis.h"
#include "moses/Manager.h"
#include "moses/StaticData.h"
#include "Options.h"

using namespace std;
using namespace v8;
using namespace Moses;

namespace MosesJS {

Handle<Value> InitMoses(const Arguments& args) {
  HandleScope scope;

  if (args.Length() == 0 || args.Length() > 2 || !(args[0]->IsString()) || !(args[1]->IsUndefined() || args[1]->IsObject())) {
    return ThrowException(Exception::Error(String::New("init(path [, options]) requires path to a Moses configuration file and an optional option object.")));
  }

  OptionsParameter* params = new OptionsParameter();
  string file = *String::Utf8Value(args[0]);
  if (!params->LoadParam(file)) {
    delete params;
    return ThrowException(Exception::Error(String::New(("Unable to load file " + file).c_str())));
  }

  Handle<Value> result;
  if (args[1]->IsObject()) {
    Handle<Object> object = Handle<Object>::Cast(args[1]);
    Options options(object);
    result = options.OverwriteParams(*params);
  } else {
    Options options;
    result = options.OverwriteParams(*params);
  }
  if (!result.IsEmpty()) {
    return result;
  }

  StaticData::InstanceNonConst().LoadData(params);
  return scope.Close(Boolean::New(true));
}

Handle<Value> Translate(const Arguments& args) {
  if (args.Length() != 1 || !(args[0]->IsString())) {
    return ThrowException(Exception::Error(String::New("translate(input) takes a string argument.")));
  }

  HandleScope scope;
  const StaticData &staticData = StaticData::Instance();
  const TranslationSystem& system = staticData.GetTranslationSystem(TranslationSystem::DEFAULT);

  istringstream input(*String::Utf8Value(args[0]));
  stringstream output;
  int i = 0;
  do {
    Sentence source;
    if (!source.Read(input, staticData.GetInputFactorOrder())) {
      break;
    }
    Manager manager(++i, source, staticData.GetSearchAlgorithm(), &system);
    manager.ProcessSentence();
    const Hypothesis* bestHypo = manager.GetBestHypothesis();
    if (bestHypo) {
      bestHypo->ToStringStream(output);
      output << endl;
    } else {
      stringstream errorInput;
      source.Print(errorInput);
      return ThrowException(Exception::Error(String::New(("Failed to translate \"" + errorInput.str() + "\"").c_str())));
    }
  } while (true);
  return scope.Close(String::New(output.str().c_str()));
}

}

void Init(Handle<Object> exports) {
  HandleScope scope;
  exports->Set(String::NewSymbol("init"), FunctionTemplate::New(MosesJS::InitMoses)->GetFunction());
  exports->Set(String::NewSymbol("translate"), FunctionTemplate::New(MosesJS::Translate)->GetFunction());
}

extern "C" void InitAll(Handle<Object> exports) {
  Init(exports);
}

NODE_MODULE(Moses, InitAll)
