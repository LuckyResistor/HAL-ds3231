# Maxim DS3231 Real Time Clock Hardware Access Layer
This is a HAL for the Maxim DS3231 real time clock chips. It builds upon the HAL core layer system.

See "HAL-feather-m0" for details how to use the HAL system in your projects.

## How to use this HAL
To use this HAL, include this repository as a submodule in your project. The submodule must be a subdirectory `hal-ds3231` in your Arduino project directory as shown below:

```
YourProject/
	YourProject.ino
	src/
		Application.hpp
		Application.cpp
		hal/ <-- The core HAL submodule
		hal-ds3231/ <-- This repository as submodule
			[files]
```

Make sure you also include the core HAL as submodule in your project. This code builds upon the `WireMaster` system.

Alternatively, just copy the directory in your project, if you do not wish to use GIT for updates of this library. Still, you have to copy the directory structure from above.

**The `src` subdirectory is important!** Place all your sources, except the `.ino` file in the `src` directory. All `hal` layer directories have to be placed below the `src` directory. The Arduino environment will only compile `cpp` files inside of the `src` subdirectory. If the `hal*` directories are placed outside of the `src` directory, the implementations of the are not compiled into the project.

```
cd YourProject/
git submodule add git@github.com:LuckyResistor/HAL-feather-m0.git src/hal
git submodule add git@github.com:LuckyResistor/HAL-ds3231.git src/hal-ds3231
```

## Status
This library is a work in progress. It is published merely as an inspiration and in the hope it may be useful. 

## License
Copyright 2019 by Lucky Resistor.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.