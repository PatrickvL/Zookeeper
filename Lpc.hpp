#pragma once
#include "Zookeeper.hpp"

class Lpc : public Device {
public:
	Lpc();

	void readPci(uint32_t reg, void *buffer, uint32_t length);
	void writePci(uint32_t reg, void *buffer, uint32_t length);
};
