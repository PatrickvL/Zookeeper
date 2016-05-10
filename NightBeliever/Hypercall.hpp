#pragma once

#include "NightBeliever.hpp"
#include "../vmcalls.hpp"

void log(const char *fmt, ...);
#define printf log
void log(const char *fmt, va_list arglist);

void *map(void *virt_base, uint32_t count);
void unmap(void *virt_base, uint32_t count);
XbeHeader_t *get_xbebase();

typedef void (*thread_ep_t)(uint32_t, uint32_t);
uint32_t create_thread(thread_ep_t eip, void *esp, uint32_t arg);

uint32_t query_eeprom(uint32_t index);
