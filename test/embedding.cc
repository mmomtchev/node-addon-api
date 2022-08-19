#include <assert.h>
#include <stdio.h>
#define NAPI_EMBEDDING
#include <napi.h>

using namespace Napi;

void importPkg(Napi::Env env, const char* pkg_name) {
  Napi::HandleScope scope(env);

  Napi::Function import = env.Global().Get("import").As<Napi::Function>();

  Napi::Value pkg_export_promise =
      import.MakeCallback(env.Global(), {Napi::String::New(env, pkg_name)});
  assert(pkg_export_promise.IsPromise());

  Napi::Value pkg_export = pkg_export_promise.As<Napi::Promise>().Await();
  assert(pkg_export.IsObject());

  Napi::Value pkg_default = pkg_export.ToObject().Get("default");
  assert(pkg_default.IsString());
  assert(pkg_default.ToString().Utf8Value() == "genuine");
}

void requirePkg(Napi::Env env, const char* pkg_name) {
  Napi::HandleScope scope(env);

  Napi::Function require = env.Global().Get("require").As<Napi::Function>();
  Napi::Value pkg_default = require({Napi::String::New(env, pkg_name)});

#ifndef NAPI_CPP_EXCEPTIONS
  if (env.IsExceptionPending()) {
    Napi::Error e = env.GetAndClearPendingException();
    throw e;
  }
#endif

  assert(pkg_default.IsString());
  assert(pkg_default.ToString().Utf8Value() == "original");
}

int main() {
  try {
    Napi::Platform platform;
    Napi::PlatformEnv env(platform);
    Napi::HandleScope scope(env);

    try {
      requirePkg(env, "./embedding.cjs");

      try {
        requirePkg(env, "./notembedding.cjs");
        fprintf(stderr,
                "Failed getting an exception for non-existing package\n");
        return -1;
      } catch (const Napi::Error& e) {
        assert(strstr(e.Message().c_str(), "Cannot find module") != nullptr);
      }

      importPkg(env, "./embedding.mjs");

    } catch (const Napi::Error& e) {
      fprintf(stderr, "Caught a JS exception: %s\n", e.Message().c_str());
      return -1;
    }
  } catch (napi_status r) {
    fprintf(stderr, "Failed initializing Node.js environment: %d\n", (int)r);
    return -1;
  }

  return 0;
}
