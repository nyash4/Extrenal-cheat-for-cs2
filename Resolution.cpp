#include "Resolution.hpp"



ScreenRes::ScreenRes() {
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
}