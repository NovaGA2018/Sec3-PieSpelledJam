#pragma once
#include "stdafx.h"
#include <iostream>
#include <string>

using namespace std;

class player
{
public:
	player(); 
	player(string inputName);

	string name;
	void greet();
};