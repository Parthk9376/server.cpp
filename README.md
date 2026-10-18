
# MallSeat Finder

A C++17 food court table management system.

## Features
- View available and occupied tables
- Search by group size
- Occupy and release tables
- Display occupancy statistics
- Regular and premium tables
- Inheritance and operator overloading
- Terminal mode and plain-text HTTP mode

## OOP concepts
- Inheritance: Table -> FoodCourtTable -> PremiumTable
- Operator overloading: == compares table IDs
- Encapsulation: table state is protected
- Polymorphism: virtual display() and type() methods

## Build
bash build.sh

## Local terminal menu
./mallseat --console

## HTTP mode
./mallseat

Open http://localhost:10000 in a browser.

## Example actions
/?action=list
/?action=find&size=4
/?action=occupy&id=3
/?action=release&id=3
/?action=stats

## Limitations
The initial implementation stores table states in memory.
State resets whenever the process restarts.
