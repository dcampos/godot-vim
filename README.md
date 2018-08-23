# GodotVim

This project aims to bring Vim keybindings to Godot's script editor.

## Status

Development is currently on hold. You may wish to use it as a starting point for further development or for understanding how the script editor works (or worked).

## Getting Started

The module in its current state is a work in progress and much has to be done in order to make it more usable. If you want to try it out, just follow the steps below.

### Installing

Because it is written as a C++ module, GodotVim needs Godot to be compiled from source in order to work.

First, clone Godot to your local (and set up your favorite C++ editor if you want to tinker with the code). The plugin currently only works with Godot 2.1.

```
git clone -b 2.1 git@github.com:godotengine/godot.git
```

Clone godot-vim:

```
git clone git@github.com:dcampos/godot-vim.git
```

Copy or symlink the folder godot_vim of the plugin into the modules folder of the godot repository. Then compile Godot as per the [official documentation](http://docs.godotengine.org/en/stable/development/compiling/index.html).

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

