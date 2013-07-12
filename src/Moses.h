#include <string>
#include <node.h>
#include "moses/ThreadedStatic.h"
#include "Options.h"

using namespace node;
using namespace std;
using namespace v8;
using namespace Moses;

namespace MosesJS {

typedef struct {
  string configFile;
  Options* options;
  ThreadDataType* threadDataPtr;
  Persistent<Function> callback;
  string error;
} InitPayload;

extern "C" void InitModule(Handle<Object> exports);
void InitSync(uv_work_t* req);
void InitComplete(uv_work_t* req, int status);

class Moses : public ObjectWrap {
  public:
    Moses();

    static Handle<Value> New(const Arguments& args);
    static Handle<Value> Init(const Arguments& args);
    static Handle<Value> Translate(const Arguments& args);
    static Handle<Value> Destroy(const Arguments& args);

    static void InitModule(Handle<Object> exports);

  private:
    static Handle<Value> InitMosesUsage();

    ThreadDataType threadData;

    static Persistent<FunctionTemplate> constructor;
};

}