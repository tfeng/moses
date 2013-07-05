#include <node.h>
#include <uv.h>
#include "Options.h"

using namespace node;
using namespace v8;

namespace MosesJS {

extern "C" void InitModule(Handle<Object> exports);

typedef struct {
  string configFile;
  Options* options;
  Persistent<Function> callback;
  string error;
} InitPayload;

Handle<Value> Init(const Arguments& args);
void InitSync(uv_work_t* req);
void InitComplete(uv_work_t* req, int status);
Handle<Value> Translate(const Arguments& args);

}