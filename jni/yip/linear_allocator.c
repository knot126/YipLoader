/**
 * Linear allocator for pre and post ELF extra segment
 * 
 * -----------------------------------------------------------------------------
 * 
 * This file is part of YipLoader. Copyright (c) 2024 - 2026 Knot126.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "linear_allocator.h"
#include "log.h"

#define LA_ALIGNMENT 8

void YipLoader_LinearAllocator_Init(YipLoader_LinearAllocator *self, void *block, size_t size, bool backwards) {
	self->size = size;
	self->backwards = backwards;
	
	if (backwards) {
		self->block = block + size;
	}
	else {
		self->block = block;
	}
}

void *YipLoader_LinearAllocator_Alloc(YipLoader_LinearAllocator *self, void *block, size_t size) {
	if (block || !size) {
		return NULL;
	}
	
	// Make it aligned if its not already
	if (size % LA_ALIGNMENT) {
		size += size - (size % LA_ALIGNMENT);
	}
	
	// Make sure we have enough of the block left to satisfy the allocation
	if (self->size < size) {
		return NULL;
	}
	
	// Return block
	if (self->backwards) {
		self->size -= size;
		self->block -= size;
		LogI("LA Alloc bwd size=%zu ptr=%p", size, self->block);
		return self->block;
	}
	else {
		void * const ret = self->block;
		self->size -= size;
		self->block += size;
		LogI("LA Alloc fwd size=%zu ptr=%p", size, ret);
		return ret;
	}
	
	return NULL;
}
