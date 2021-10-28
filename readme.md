# jsonkit: json utility for common task

Based on rapidjson library.

While rapidjson is a wonderful library for json, some of it's API is not
friendly enouth, so I write a colloction of utility function for common task
implementation. Hope this repos is useful for reference or use it directly.

## Functionality

* json comparison
* schema validation and generation
* operator override, path(/) pipe(|)

## Dependency

There are sub-modules in `thirdparty/` directory.

- [rapidjson](https://github.com:Tencent/rapidjson.git)
- [couttast](https://github.com/lymslive/couttast.git) Optional as a
  lightweight unit test(named tast) framework.

## Build

```bash
cd build
make
make tast
make tast.run
```

Just `make` to build the static `libjsonkit.a`, then make take and run if
needed.

## Usage

See the expample and unit test in `tast/src`.
