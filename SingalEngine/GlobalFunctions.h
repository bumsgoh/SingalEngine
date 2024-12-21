#pragma once

#include <windows.h>
#include <iostream>

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        // 디버깅할 때 여기에 breakpoint 설정
        throw std::exception();
    }
}
