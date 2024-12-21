#pragma once

#include <windows.h>
#include <iostream>

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        // ������� �� ���⿡ breakpoint ����
        throw std::exception();
    }
}
