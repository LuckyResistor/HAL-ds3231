# Maxim DS3231 Real Time Clock Hardware Access Layer
This is a HAL for the Maxim DS3231 real time clock chips. It builds upon the HAL core layer system.

See "HAL-feather-m0" for details how to use the HAL system in your projects.

## How to use this HAL
To use this HAL, include this repository as a submodule in your project. The submodule must be a subdirectory `hal-ds3231` in your Arduino project directory as shown below:

```
YourProject/
	YourProject.ino
	Application.hpp
	Application.cpp
	hal/ <-- The core HAL submodule
	hal-ds3231/ <-- This repository as submodule
		[files]
```

Make sure you also include the core HAL as submodule in your project. This code builds upon the `WireMaster` system.

Alternatively instead of a submodule, just copy the directory in your project, if you do not wish to use GIT for updates of this library.

## Status
This library is a work in progress. It is published merely as an inspiration and in the hope it may be useful. 

## License
Copyright 2018 by Lucky Resistor.

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