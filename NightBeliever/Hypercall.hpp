// Automatically generated by hypercallgen.py
#pragma once
#include "NightBeliever.hpp"
#include "HypercallSupport.hpp"

void * map(void *, uint32_t);
uint32_t query_eeprom(uint32_t);
void unmap(void *, uint32_t);
void log_(const char *);
uint32_t create_thread(thread_ep_t, void *, uint32_t);
XbeHeader_t * get_xbebase();
