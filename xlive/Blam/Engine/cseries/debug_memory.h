#pragma once

// should probably expand these into macros that also include filename and line
uint8* __cdecl debug_malloc(uint32 size);
void __cdecl debug_free(uint8* buffer);
