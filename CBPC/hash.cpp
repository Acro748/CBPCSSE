#include "hash.h"
#include "xs_Float.h"


int fastTrunc(float x)
{
	//long i = (long)x; /* truncate */
	//return i - (i > x); /* convert trunc to floor */
	//return static_cast<int>(x);
	if (x >= 0)
		return xs_CeilToInt(x);
	else
		return xs_FloorToInt(x);
}

uint32_t generateId(uint8_t v1, uint8_t v2, uint8_t v3)
{
	uint32_t id;
	id = v1 | (((uint32_t)v2) << 8) | (((uint32_t)v3) << 16);
	return id;
}

int CreateHashId(float x, float y, float z, int gridSize, int actordistance)
{
	//return (fastTrunc(x / gridSize) * 92821 + fastTrunc(y / gridSize) * 7243 + fastTrunc(z / gridSize) * 31) % size;
	return generateId(fastTrunc((x + actordistance) / gridSize), fastTrunc((y + actordistance) / gridSize), fastTrunc((z + actordistance) / gridSize));
}