#include <node.h>
#include <uv.h>
#include "Moses.h"

#include "moses/Hypothesis.h"
#include "moses/Manager.h"
#include "moses/StaticData.h"

using namespace node;
using namespace std;
using namespace v8;
using namespace Moses;

namespace MosesJS {

extern "C" void InitModule(Handle<Object> exports) {
  HandleScope scope;
  exports->Set(String::NewSymbol("init"), FunctionTemplate::New(MosesJS::Init)->GetFunction());
  exports->Set(String::NewSymbol("translate"), FunctionTemplate::New(MosesJS::Translate)->GetFunction());
}

Handle<Value> InitMosesUsage() {
  HandleScope scope;
  return ThrowException(Exception::Error(String::New("init(path [, options] [, callback]) requires path to a Moses configuration file, an optional options object, and an optional callback function.")));
}

Handle<Value> Init(const Arguments& args) {
  HandleScope scope;
  int numArgs = args.Length();

  if (numArgs < 1 || numArgs > 3 || !args[0]->IsString()) {
    return InitMosesUsage();
  }

  if (numArgs > 1) {
    if (args[1]->IsObject() && numArgs == 3 && !args[2]->IsFunction()) {
      return InitMosesUsage();
    } else if (!args[1]->IsObject() && !args[1]->IsFunction()) {
      return InitMosesUsage();
    } else if (args[1]->IsFunction() && numArgs > 2) {
      return InitMosesUsage();
    }
  }

  string configFile = *String::Utf8Value(args[0]);

  Options* options;
  if (numArgs > 1 && args[1]->IsObject()) {
    Handle<Object> object = Handle<Object>::Cast(args[1]);
    options = new Options(object);
  } else {
    options = new Options();
  }

  Persistent<Function> callback;
  if (numArgs == 2 && args[1]->IsFunction()) {
    callback = Persistent<Function>::New(Handle<Function>::Cast(args[1]));
  } else if (numArgs == 3 && args[2]->IsFunction()) {
    callback = Persistent<Function>::New(Handle<Function>::Cast(args[2]));
  } else {
    callback = Persistent<Function>();
  }

  uv_work_t *req = new uv_work_t;
  InitPayload* payload = new InitPayload();
  req->data = payload;
  payload->configFile = configFile;
  payload->options = options;
  payload->callback = callback;

  uv_queue_work(uv_default_loop(), req, InitSync, InitComplete);

  return Undefined();
}

void InitSync(uv_work_t* req) {
  InitPayload* payload = (InitPayload*) req->data;

  OptionsParameter* params = new OptionsParameter();
  if (!params->LoadParam(payload->configFile)) {
    payload->error = "Unable to load file " + payload->configFile;
    delete params;
    return;
  }

  string result = payload->options->OverwriteParams(*params);
  if (!result.empty()) {
    payload->error = result;
    delete params;
    return;
  }

  try {
    StaticData::InstanceNonConst().LoadData(params);
  } catch (...) {
    payload->error = "Unable to load static data.";
  }

  delete params;
}

void InitComplete(uv_work_t* req, int status) {
  HandleScope scope;

  InitPayload* payload = (InitPayload*) req->data;

  Handle<Value> err;
  if (payload->error.empty()) {
    err = Undefined();
  } else {
    err = String::New(payload->error.c_str());
  }

  TryCatch try_catch;
  if (!payload->callback.IsEmpty()) {
    Handle<Value> argv[1];
    argv[0] = err;
    payload->callback->Call(Context::GetCurrent()->Global(), 1, argv);
  }

  payload->callback.Dispose();
  delete payload->options;
  delete payload;
  delete req;

  if(try_catch.HasCaught()) {
    FatalException(try_catch);
  }
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

NODE_MODULE(Moses, MosesJS::InitModule)
