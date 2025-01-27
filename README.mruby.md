# Cosmopolitan with mruby

This fork tries to replace Lua with [mruby](https://mruby.org/). There is nothing wrong with Lua. It is just a personal preference.

## Installation

### cosmopolitan toolchain

Go into cosmopolitan directory. Run `make -j8 o//tool/net/redbean` to be sure that *cosmopolitan* runs on your system. It also downloads necessary files to kick start. You may need to run `ape/apeinstall.sh` first for WSL. It may also download cosmocc.zip which will be installed under `.cosmocc` in cosmopolitan directory.

### mruby

[mruby](https://mruby.org/) has its own compilation system based on rake. Therefore, the static library (`libmruby.a`) can be compiled with `cosmocc` in its project. Go to mruby directory. Put `cosmo.rb` (see below) under `build_config/` and compile with `rake MRUBY_CONFIG=cosmo`.

```
# build_config/cosmo.rb

MRuby::CrossBuild.new('cosmo') do |conf|

  toolchain :gcc

  # Chage cosmocc version number to match your system
  COSMO_PATH = "../cosmopolitan/.cosmocc/3.3.5/"

  # C compiler
  conf.cc do |cc|
    cc.command = "#{COSMO_PATH}/bin/cosmocc"
    cc.include_paths = ["#{COSMO_PATH}/include", "./include"]
  end

  # C++ compiler
  conf.cxx do |cxx|
    cxx.command = "#{COSMO_PATH}/bin/cosmoc++"
    cxx.include_paths = conf.cc.include_paths.dup
  end

  # Linker
  conf.linker do |linker|
    linker.command = "#{COSMO_PATH}/bin/cosmocc"
    linker.library_paths = ["#{COSMO_PATH}/lib"]
  end

  # Ar
  conf.archiver.command = "#{COSMO_PATH}/bin/cosmoar"

  # All current core gems with ones with build issues commented out
  conf.gembox "stdlib"
  conf.gembox "stdlib-ext"
  conf.gembox "stdlib-io"

  # Generate mrbc command
  conf.gem :core => "mruby-bin-mrbc"

  # Generate mrdb command
#  conf.gem :core => "mruby-bin-debugger"

  # Generate mirb command
  conf.gem :core => "mruby-bin-mirb"

  # Generate mruby command
  conf.gem :core => "mruby-bin-mruby"

  # Generate mruby-strip command
#  conf.gem :core => "mruby-bin-strip"

  # Generate mruby-config command
  conf.gem :core => "mruby-bin-config"

  # Add more gems if needed

  # Turn on `enable_debug` for better debugging
  # conf.enable_debug
  conf.enable_bintest
  conf.enable_test
end
```

### third_party/mruby

Go back to cosmopolitan directory (e.g. `../cosmopolitan`). Make dummy mruby package by running `make o//third_party/mruby`. It creates a mruby package under `o/third_party/mruby`, but lacks real mruby library. Copy `libmruby.a` from `../mruby/build/cosmo/lib/` to `o/third_party/mruby` so that the rest of cosmopolitan can use it. Also copy everything in `../mruby/build/cosmo/include/` to `third_party/mruby/` directory. These are headers of mruby and associated gems. The final file structure should looks like this:

```
third_party/mruby/BUILD.mk
third_party/mruby/mruby.h
third_party/mruby/mrbconf.h
third_party/mruby/mruby/class.h
third_party/mruby/mruby/string.h
...
o/third_party/mruby/libmruby.a
o/third_party/mruby/libmruby_core.a
```

Please note that mruby headers should stay in `third_party/mruby` while compiled libraries in `o/third_party/mruby`. Headers of mruby will differ depends on the gems used. Therefore, there is no default headers under `third_party/mruby/`.

### example of mruby under cosmopolitan

Make the example of mruby under cosmopolitan by running `make o//examples/mruby/hello`. It creates an executable `o/examples/mruby/hello`. Run it to be sure mruby is correctly installed. You can also copy this `hello` to Windows. Rename it as `hello.com`and run. It should work. If there is any error, check file size of libmruby.a under `o/third_part/mruby` and see whether it is overrided with a dummy one. If so, copy the real one from mruby directory again.

### redbean-mruby (work in progress)

Run `make -j8 o//tool/net/redbean-mruby` to get redbean with mruby support. Run `o/tool/net/redbean-mruby` to start.

A `tool/net/mruby/.init.mrb` example is included. It supports `on_http_request` hook and some redbean api. Since it intercepts all http requests, normal pages will not display. Use `curl http://127.0.0.1:8080/?foo&bar=123` to test. Or use a browser to check it.

#### mruby-shelf (rack-like interface)

Rails uses Rack for web server interface. There is a [mruby-shelf](https://github.com/katzer/mruby-shelf) for mruby. To use this gem, add the following line in mruby/build_config/cosmo.rb

```
  conf.gem :core => "mruby-shelf"
```

Also put mruby-shelf source code under `mruby/mrbgems/mruby-shelf/`. Run `rake MRUBY_CONFIG=cosmo` to rebuild libmruby.a and move it into your cosmo directory. Rebuild `redbean-mruby` by `make -j8 o//tool/net/redbean-mruby` and now mruby-shelf can be used like this in `.init.mrb`

```
def on_http_request
  puts "on_http_request\n"
  puts "path: #{get_path}"
  puts "effective path: #{get_effective_path}"
  puts "method: #{get_method}"
  puts "host: #{get_header 'HOST'}"
  puts "user-agent: #{get_header 'User-Agent'}"
  puts "http version: #{get_http_version}"
  puts "scheme: #{get_scheme}"
  puts get_params

  app = ShelfApp.new
  output = app.call('REQUEST_METHOD' => get_method, 'PATH_INFO' => get_effective_path)

  return output[-1][0]
end

class RootPage
  def call(env)
    s = <<-HEREDOC
      <html>
        <head></head>
        <body>
          <h1>This is root page</h1>
        </body>
      </html>
    HEREDOC
    [200, { 'content-type' => 'text/plain' }, [s]]
  end
end


class ShelfApp
  def initialize
    @app = Shelf::Builder.app do
       get('/users/{id}') { run ->(env) { [200, { 'content-type' => 'text/plain' }, [env.to_s]] } }
       get('/') { run RootPage.new }
    end
  end

  def call(env)
    @app.call(env)
  end
end

```

## Update

Whenever a new version is installed, mostly likely under `.cosmocc` of cosmopolitan repository, libmruby.a need to be rebuilt. Change `COSMO_PATH` in mruby build_config to new path, compile and move `libmruby.a` into `o/third_party/mruby`.

## TODO

### Suport ARM (aarch64)

On my system, cosmo not only creates `libmruby.a`, but also `.aarch64/libmruby.a`, suggesting support of ARM system. But I don't know how to have its ARM support built into the executable of mruby example under `o/examples/mruby/hello`.
