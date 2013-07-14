#include <node.h>
#include "Moses.h"

#include "moses/Manager.h"
#include "moses/ThreadedStatic.h"

using namespace node;
using namespace std;
using namespace v8;
using namespace Moses;

namespace MosesJS {

extern "C" void InitModule(Handle<Object> exports) {
  Moses::InitModule(exports);
}

void InitSync(uv_work_t* req) {
  InitPayload* payload = (InitPayload*) req->data;

  OptionsParameter* params = new OptionsParameter();
  try {
    if (!params->LoadParam(payload->configFile)) {
      payload->error = "Unable to load config file " + payload->configFile;
      delete params;
      return;
    }
  } catch (exception const & ex) {
    payload->error = string("Unable to load config file. Reason: ") + ex.what();
  }

  string result = payload->options->OverwriteParams(*params);
  if (!result.empty()) {
    payload->error = result;
    delete params;
    return;
  }

  ThreadedStaticHelper::SetThreadData(*(payload->threadDataPtr));

  try {
    StaticData::InstanceNonConst().LoadData(params);
  } catch (exception const & ex) {
    payload->error = string("Unable to load static data. Reason: ") + ex.what();
  }

  *(payload->threadDataPtr) = ThreadedStaticHelper::GetThreadData();
  ThreadedStaticHelper::RemoveThreadData();

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

Moses::Moses() {
}

Handle<Value> Moses::New(const Arguments& args) {
  if (!args.IsConstructCall()) {
    return ThrowException(Exception::Error(String::New("Use the new operator to create instances of this object.")));
  }
  if (args.Length() > 0) {
    return ThrowException(Exception::Error(String::New("New operator of this object does not take any argument.")));
  }

  Handle<Object> thisObject = args.This();
  Moses* moses = new Moses();
  moses->Wrap(thisObject);
  thisObject->Set(String::NewSymbol("init"), FunctionTemplate::New(Init)->GetFunction());
  thisObject->Set(String::NewSymbol("translate"), FunctionTemplate::New(Translate)->GetFunction());
  thisObject->Set(String::NewSymbol("destroy"), FunctionTemplate::New(Destroy)->GetFunction());

  return thisObject;
}

Handle<Value> Moses::InitMosesUsage() {
  HandleScope scope;
  return ThrowException(Exception::Error(String::New("init(path [, options] [, callback]) requires path to a Moses configuration file, an optional options object, and an optional callback function.")));
}

Handle<Value> Moses::Init(const Arguments& args) {
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

  Handle<Object> thisObject = args.This();
  Moses* obj = ObjectWrap::Unwrap<Moses>(thisObject);
  ThreadedStaticHelper::DestroyThreadData(obj->threadData);

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
  payload->threadDataPtr = &(obj->threadData);
  payload->callback = callback;

  uv_queue_work(uv_default_loop(), req, InitSync, InitComplete);

  return Undefined();
}

Handle<Value> Moses::Translate(const Arguments& args) {
  if (args.Length() != 1 || !(args[0]->IsString())) {
    return ThrowException(Exception::Error(String::New("translate(input) takes a string argument.")));
  }

  Handle<Object> thisObject = args.This();
  Moses* obj = ObjectWrap::Unwrap<Moses>(thisObject);
  if (obj->threadData.empty()) {
    return ThrowException(Exception::Error(String::New("The object must be initialized with init() function.")));
  }

  HandleScope scope;

  ThreadedStaticHelper::SetThreadData(obj->threadData);

  stringstream output;
  try {
    const StaticData &staticData = StaticData::Instance();
    const TranslationSystem& system = staticData.GetTranslationSystem(TranslationSystem::DEFAULT);
    istringstream input(*String::Utf8Value(args[0]));
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
  } catch (exception const & ex) {
    ThreadedStaticHelper::RemoveThreadData();
    return ThrowException(Exception::Error(String::New((string("Unable to translate: ") + ex.what()).c_str())));
  }

  ThreadedStaticHelper::RemoveThreadData();

  return scope.Close(String::New(output.str().c_str()));
}

Handle<Value> Moses::Destroy(const Arguments& args) {
  if (args.Length() != 0) {
    return ThrowException(Exception::Error(String::New("destroy() does not take any arguments.")));
  }

  Handle<Object> thisObject = args.This();
  Moses* obj = ObjectWrap::Unwrap<Moses>(thisObject);
  ThreadedStaticHelper::DestroyThreadData(obj->threadData);
  return Undefined();
}

void Moses::InitModule(Handle<Object> exports) {
  HandleScope scope;

  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Moses::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1); // for constructors
  constructor->SetClassName(String::NewSymbol("Moses"));
  exports->Set(String::NewSymbol("Moses"), constructor->GetFunction());
}

Persistent<FunctionTemplate> Moses::constructor;

NODE_MODULE(Moses, InitModule)
}
