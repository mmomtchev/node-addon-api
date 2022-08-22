# Embedding woth `libnode`

Embedding a Node.js instance to be loaded and called as a shared library
from C++ code requires `libnode` and defining the `NAPI_EMBEDDING` macro
before including `napi.h`.

Two additional types become available in this case:

* `Napi::Platform` of which you need to create exactly one object
* `Napi::PlatformEnv` which can be used as a substitute for `Napi::Env` and
which allows the creation of a new Node.js instance (V8 isolate)

Unlike other NAPI functions which throw JS-compatible exceptions, these functions
throw `napi_status` values because they do not have access to a working Node.js
environment.


### Usage Suggestions

Here is a quick example for embedding a Node.js instance through `node-addon-api`
and retrieving a reference to `axios.get` after `require('axios')`.

```cpp
#define NAPI_EXPERIMENTAL
#define NAPI_EMBEDDING
#include <napi.h>

int main() {
    try {
        Napi::Platform platform;
        Napi::PlatformEnv env(platform);

        try {
            Napi::HandleScope scope(env);
            Napi::Function require =
                env.Global().Get("require").As<Napi::Function>();
            Napi::Object axios = require({Napi::String::New(env, "axios")})
                    .ToObject();
            Napi::Function get = axios.Get("get").As<Napi::Function>();
        } catch (const Napi::Error &e) {
            fprintf(stderr, "Caught a JS exception: %s\n", e.what());
            return -1;
        }
    } catch (napi_status r) {
        fprintf(stderr, "Failed initializing Node.js environment: %d\n",
                (int)r);
        return -1;
    }

    return 0;
}
```