#pragma once

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>

#include "skse64\PapyrusVM.h"
#include "dirent.h"


// trim from start (in place)
static inline void ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s)
{
	ltrim(s);
	rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s)
{
	ltrim(s);
	return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s)
{
	rtrim(s);
	return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s)
{
	trim(s);
	return s;
}

static inline std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::string str = trim_copy(s);

	std::vector<std::string> tokens;
	if (!str.empty())
	{
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter))
		{
			trim(token);
			tokens.emplace_back(token);
		}
	}
	return tokens;
}

static inline std::vector<std::string> splitNonEmpty(const std::string& s, char delimiter)
{
	std::string str = trim_copy(s);

	std::vector<std::string> tokens;
	if (!str.empty())
	{
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter))
		{
			trim(token);
			if (token.size() > 0)
				tokens.emplace_back(token);
		}
	}
	return tokens;
}

static inline std::vector<std::string> splitMulti(const std::string& s, std::string delimiters)
{
	std::string str = trim_copy(s);

	std::vector<std::string> tokens;
	std::stringstream stringStream(str);
	std::string line;
	while (std::getline(stringStream, line))
	{
		std::size_t prev = 0, pos;
		while ((pos = line.find_first_of(delimiters, prev)) != std::string::npos)
		{			
			if (pos > prev)
			{
				std::string token = line.substr(prev, pos - prev);
				trim(token);
				tokens.emplace_back(token);
			}

			prev = pos + 1;
		}
		if (prev < line.length())
		{
			std::string token = line.substr(prev, std::string::npos);
			trim(token);
			tokens.emplace_back(token);
		}
	}
	return tokens;
}

static inline std::vector<std::string> splitMultiNonEmpty(const std::string& s, std::string delimiters)
{
	std::string str = trim_copy(s);

	std::vector<std::string> tokens;
	std::stringstream stringStream(str);
	std::string line;
	while (std::getline(stringStream, line))
	{
		std::size_t prev = 0, pos;
		while ((pos = line.find_first_of(delimiters, prev)) != std::string::npos)
		{
			if (pos > prev)
			{
				std::string token = line.substr(prev, pos - prev);
				trim(token);
				if (token.size() > 0)
					tokens.emplace_back(token);
			}

			prev = pos + 1;
		}
		if (prev < line.length())
		{
			std::string token = line.substr(prev, std::string::npos);
			trim(token);
			if (token.size() > 0)
				tokens.emplace_back(token);
		}
	}
	return tokens;
}

static inline std::vector<std::string> split(const std::string& s, std::string delimiter)
{
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) 
	{
		token = s.substr(pos_start, pos_end - pos_start);
		trim(token);
		pos_start = pos_end + delim_len;
		res.emplace_back(token);
	}

	std::string lasttoken = s.substr(pos_start);
	trim(lasttoken);
	res.emplace_back(lasttoken);
	return res;
}

static inline bool Contains(std::string str, std::string ministr)
{
	if (str.find(ministr) != std::string::npos) {
		return true;
	}
	else
		return false;
}

static inline bool ContainsNoCase(std::string str, std::string ministr)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::transform(ministr.begin(), ministr.end(), ministr.begin(), ::tolower);

	if (str.find(ministr) != std::string::npos) {
		return true;
	}
	else
		return false;
}

static inline void skipComments(std::string& str)
{
	auto pos = str.find("#");
	if (pos != std::string::npos)
	{
		str.erase(pos);
	}
}

static inline std::string gettrimmed(const std::string& s) {
	std::string temp = s;
	if (temp.front() == '[' && temp.back() == ']') {
		temp.erase(0, 1);
		temp.pop_back();
	}
	return temp;
}

static inline void NormalizeNiPoint(NiPoint3 &collisionVector, float low, float high)
{
	if (collisionVector.x < low)
		collisionVector.x = low;
	else if (collisionVector.x > high)
		collisionVector.x = high;

	if (collisionVector.y < low)
		collisionVector.y = low;
	else if (collisionVector.y > high)
		collisionVector.y = high;

	if (collisionVector.z < low)
		collisionVector.z = low;
	else if (collisionVector.z > high)
		collisionVector.z = high;
}

static inline float clamp(float val, float min, float max) {
	if (val < min) return min;
	else if (val > max) return max;
	return val;
}

static inline bool CompareNiPoints(NiPoint3 collisionVector, NiPoint3 emptyPoint)
{
	return fabsf(collisionVector.x - emptyPoint.x) < 0.000001f && fabsf(collisionVector.y == emptyPoint.y) < 0.000001f && fabsf(collisionVector.z == emptyPoint.z) < 0.000001f;
}

static inline BSFixedString ReturnUsableString(std::string str)
{
	BSFixedString fs("");
	CALL_MEMBER_FN(&fs, Set)(str.c_str());
	return fs;
}
/*
static inline void showRotation(NiMatrix33 &r) {
	LOG("%8.2f %8.2f %8.2f", r.data[0][0], r.data[0][1], r.data[0][2]);
	LOG("%8.2f %8.2f %8.2f", r.data[1][0], r.data[1][1], r.data[1][2]);
	LOG("%8.2f %8.2f %8.2f", r.data[2][0], r.data[2][1], r.data[2][2]);
	LOG("-----------------");
}
*/
static inline float distanceNoSqrt2D(NiPoint3 po1, NiPoint3 po2)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("distanceNoSqrt2D Start");*/

	float x = po1.x - po2.x;
	float y = po1.y - po2.y;
	float result = x*x + y*y;

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("distanceNoSqrt2D Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return result;
}

static inline float distanceNoSqrt(NiPoint3 po1, NiPoint3 po2)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("distanceNoSqrt Start");*/

	float x = po1.x - po2.x;
	float y = po1.y - po2.y;
	float z = po1.z - po2.z;
	float result = x*x + y*y + z*z;

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("distanceNoSqrt Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return result;
}

static inline void GetAttitudeAndHeadingFromTwoPoints(NiPoint3 source, NiPoint3 target, float& attitude, float& heading)
{
	NiPoint3 vector = target - source;

	const float sqr = vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
	if (sqr != 0)
	{
		vector = vector * (1.0f / sqrtf(sqr));

		attitude = (float)atan2(vector.z, sqrtf(vector.y * vector.y + vector.x * vector.x)) * -1;

		heading = (float)atan2(vector.x, vector.y);
	}
	else
	{
		attitude = 0;
		heading = 0;
	}
}

static inline float AngleDifference(float angle1, float angle2)
{
	if ((angle1 < -90 && angle2 > 90) || (angle2 < -90 && angle1 > 90))
	{
		return (180 - abs(angle1)) + (180 - abs(angle2));
	}
	else if ((angle1 > -90 && 0 >= angle1 && angle2 < 90 && 0 <= angle2) || (angle2 > -90 && 0 >= angle2 && angle1 < 90 && 0 <= angle1))
	{
		return abs(angle1) + abs(angle2);
	}
	else
	{
		return abs(angle1 - angle2);
	}
}

static inline bool IsValidModIndex(UInt32 modIndex)
{
	return modIndex > 0 && modIndex != 0xFF;
}

// get base formID (without mod index)
static inline UInt32 GetBaseFormID(UInt32 formId)
{
	return formId & 0x00FFFFFF;
}

static inline std::vector<std::string> get_all_files_names_within_folder(std::string folder)
{
	std::vector<std::string> names;

	DIR *directory = opendir(folder.c_str());
	struct dirent *direntStruct;

	if (directory != nullptr) {
		while (direntStruct = readdir(directory)) {
			names.emplace_back(direntStruct->d_name);
		}
	}
	closedir(directory);

	return names;
}

static inline bool stringStartsWith(std::string str, std::string prefix)
{
	// convert string to back to lower case
	std::for_each(str.begin(), str.end(), [](char & c) 
	{
		c = ::tolower(c);
	});
	// std::string::find returns 0 if toMatch is found at starting
	if (str.find(prefix) == 0)
		return true;
	else
		return false;
}

static inline bool stringEndsWith(std::string str, std::string const& suffix) 
{
	std::for_each(str.begin(), str.end(), [](char& c)
	{
		c = ::tolower(c);
	});
	if (str.length() >= suffix.length()) 
	{
		return (0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix));
	}
	else 
	{
		return false;
	}
}

static inline double GetPercentageValue(double number1, double number2, float perc)
{
	if (perc == 100)
		return number2;
	else if (perc == 0)
		return number1;
	else
	{
		return number1 + ((number2 - number1)*(perc * 0.01f));
	}
}

static inline float GetPercentageValue(float number1, float number2, float perc)
{
	if (perc == 100)
		return number2;
	else if (perc == 0)
		return number1;
	else
	{
		return number1 + ((number2 - number1) * (perc * 0.01f));
	}
}

static inline double vlibGetSetting(const char * name) {
	Setting * setting = GetINISetting(name);
	double value;
	if (!setting)
		return -1;
	if (setting->GetDouble(&value))
		return value;
	return -1;
}

static inline float magnitude(NiPoint3 p)
{
	return sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
}

static inline float magnitude2d(NiPoint3 p)
{
	return sqrtf(p.x * p.x + p.y * p.y);
}

static inline float magnitudePwr2(NiPoint3 p)
{
	return p.x * p.x + p.y * p.y + p.z * p.z;
}

static inline NiPoint3 crossProduct(NiPoint3 A, NiPoint3 B)
{
	return NiPoint3(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
}

static inline UInt32 getHex(std::string hexstr)
{
	return (UInt32)strtoul(hexstr.c_str(), 0, 16);
}

template <typename I> static inline std::string num2hex(I w, size_t hex_len = sizeof(I) << 1) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}

static inline bool isWantSlot(TESObjectARMO* thisArmor, UInt32 wantSlot)
{
	UInt32 slot = (thisArmor) ? thisArmor->bipedObject.GetSlotMask() : 0;
	return (slot == wantSlot);
}
