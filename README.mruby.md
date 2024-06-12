# Cosmopolitan with mruby

This fork tries to replace Lua with [mruby](https://mruby.org/). There is nothing wrong with Lua. It is just a personal preference.

## Installation

### cosmopolitan toolchain

Go into cosmopolitan directory. Run `make -j8 o//tool/net/redbean` to be sure that *cosmopolitan* runs on your system. It also downloads necessary files to kick start.

### mruby

[mruby](https://mruby.org/) has its own compilation system based on rake. Therefore, the static library (`libmruby.a`) can be compiled with `cosmocc` in its project. Go to mruby directory. Put `cosmo.rb` under `build_config/` and compile with `rake BUILD_CONFIG=cosmo`.

```
# build_config/cosmo.rb

MRuby::CrossBuild.new('cosmo') do |conf|

  toolchain :gcc

  # Chage cosmocc version number to match your system
  COSMO_PATH = "../cosmopolitan/.cosmocc/3.5.5/"

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

Go back to cosmopolitan directory (e.g. `../cosmopolitan`). Make dummy mruby package by running `make o//third_party/mruby`. It creates a mruby package under `o/third_party/mruby`, but lacks real mruby library. Copy `libmruby.a` from `../mruby/build/cosmo/lib/` to `o/third_party/mruby` so that the rest of cosmopolitan can use it. Also copy everything in `../mruby/build/cosmo/mruby/include/` to `third_party/mruby/` directory. These are headers of mruby and associated gems. The final file structure should looks like this:

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

Make the example of mruby under cosmopolitan by running `o//examples/mruby/hello`. It creates an executable `o/examples/mruby/hello`. Run it to be sure mruby is correctly installed.

### redbean-mruby (TODO)

Run `make -j8 o//tool/net/redbean-mruby` to get redbean with mruby support.
