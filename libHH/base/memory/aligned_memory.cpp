// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/aligned_memory.h"
#include "build/build_config.h"


namespace base {

void* AlignedAlloc(size_t size, size_t alignment) 
{
	void* ptr = NULL;
#if defined(COMPILER_MSVC)
	ptr = _aligned_malloc(size, alignment);
#endif

	return ptr;
}

}  // namespace base
