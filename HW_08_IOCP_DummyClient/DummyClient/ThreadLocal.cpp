﻿#include "stdafx.h"
#include "ThreadLocal.h"

__declspec(thread) int LThreadType = -1;
__declspec(thread) int LIoThreadId = -1;