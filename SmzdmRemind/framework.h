// header.h: 标准系统包含文件的包含文件，
// 或特定于项目的包含文件
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
//#include <shlwapi.h>
//#pragma comment(lib, "shlwapi.lib")
#pragma comment(linker, "/nodefaultlib")
// C 运行时头文件

/*
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
*/


#ifndef _DEBUG
#ifdef _MSC_VER
#pragma function(memset)
void* __cdecl memset(void* pTarget, int value, size_t cb) {
	char* p = (char*)pTarget;
	while (cb--)*p++ = (char)value;
	return pTarget;
}
#pragma function(memcpy)
void* __cdecl memcpy(void* pDest, const void* pSrc, size_t cb) {
	void* pResult = pDest;
	while (cb--) {
		*(char*)pDest = *(char*)pSrc;
		pDest = (char*)pDest + 1;
		pSrc = (char*)pSrc + 1;
	}
	return pResult;
}
#endif
#if __cplusplus
extern "C"
#endif
int _fltused = 1;
#endif

void* mem_alloc(ULONG_PTR uSize) {
	return (void*)HeapAlloc(GetProcessHeap(), 0, uSize);
}

void mem_free(LPVOID pMemBlock) {
	HeapFree(GetProcessHeap(), 0, (LPVOID)pMemBlock);
}

void* operator new (size_t uSize) {
	return (void*)HeapAlloc(GetProcessHeap(), 0, uSize);
}

void operator delete (void* pMemBlock) {
	HeapFree(GetProcessHeap(), 0, (LPVOID)pMemBlock);
}

void* operator new[](size_t uSize) {
	return (void*)HeapAlloc(GetProcessHeap(), 0, uSize);
}

void operator delete[](void* pMemBlock) {
	HeapFree(GetProcessHeap(), 0, (LPVOID)pMemBlock);
}
int _isalnum(int c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return 8;
	return 0;
}
double __fastcall my_wstrtod(const wchar_t* nptr, wchar_t** endptr)
{
	register const wchar_t* p = nptr;
	register long double value = 0.L;
	int sign = 0;
	long double factor;
	unsigned int expo;

	while (*p==L' ') // 跳过前面的空格
		p++;

	if (*p == '-' || *p == '+')
		sign = *p++; // 把符号赋给字符sign，指针后移。

	// 处理数字字符
	while ((unsigned int)(*p - '0') < 10u) // 转换整数部分
		value = value * 10 + (*p++ - '0');

	// 如果是正常的表示方式（如：1234.5678）
	if (*p == '.')
	{
		factor = 1.;
		p++;

		while ((unsigned int)(*p - '0') < 10u)
		{
			factor *= 0.1;
			value += (*p++ - '0') * factor;
		}
	}

	// 如果是IEEE754标准的格式（如：1.23456E+3）
	if ((*p | 32) == 'e')
	{
		expo = 0;
		factor = 10.L;

		switch (*++p)
		{
		case '-':
			factor = 0.1;
		case '+':
			p++;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		default:
			//            value = 0.L;
			//            p = nptr;
			goto done;
		}

		while ((unsigned int)(*p - '0') < 10u)
			expo = 10 * expo + (*p++ - '0');

		while (1)
		{
			if (expo & 1)
				value *= factor;
			if ((expo >>= 1) == 0)
				break;
			factor *= factor;
		}
	}

done:
	if (endptr != 0)
		*endptr = (wchar_t*)p;

	return (sign == '-' ? -value : value);
}

int my_atoi(char* str)
{
	int sum = 0;
	while (*str != '\0' && (*str >= '0' && *str <= '9'))
	{
		sum = sum * 10 + *str - '0';
		str++;
	}
	return sum;
}

int my_wtoi(WCHAR* str)
{
	int sum = 0;
	while (*str != L'\0' && (*str >= L'0' && *str <= L'9'))
	{
		sum = sum * 10 + *str - L'0';
		str++;
	}
	return sum;
}

double __fastcall my_wtof(wchar_t* str)
{
	return my_wstrtod(str, 0);
}
char* xstrstr(const char* str, const char* sub)
{
	int i = 0;
	int j = 0;
	while (str[i] && sub[j])
	{
		if (str[i] == sub[j])//如果相等
		{
			++i;
			++j;
		}
		else		     //如果不等
		{
			i = i - j + 1;
			j = 0;
		}
	}
	if (!sub[j])
	{
		return (char*)&str[i - strlen(sub)];
	}
	else
	{
		return (char*)0;
	}
}