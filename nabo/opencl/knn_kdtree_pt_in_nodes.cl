typedef struct { 
	int dim;  // >=0 cut dim, -1 == leaf, -2 == invalid
	int index;
} Node;

#define OFFSIDE 0
#define ONSIDE 1

typedef struct {
	size_t n;
	uint state;
} StackEntry;

size_t childLeft(const size_t pos) { return 2*pos + 1; }
size_t childRight(const size_t pos) { return 2*pos + 2; }

// for cloud and result, use DirectAccessBit and stride
// preconditions:
// 		K < MAX_K
//		stack_ptr < MAX_STACK_DEPTH
kernel void knnKDTree(	const global T* cloud,
						const global T* query,
						global int* indices,
						global T* dists2,
						uint K,
						T maxError,
						uint optionFlags,
						uint indexStride,
						uint dists2Stride,
						const global Node* nodes
 					)
{
	StackEntry stack[MAX_STACK_DEPTH];
	HeapEntry heap[MAX_K];
	
	const size_t queryId = get_global_id(0);
	const bool allowSelfMatch = optionFlags & ALLOW_SELF_MATCH;
	const bool doSort = optionFlags & SORT_RESULTS;
	const global T* q = &query[queryId * POINT_STRIDE];
	
	heapInit(heap, K);
	
	uint stackPtr = 1;
	stack[0].state = ONSIDE;
	stack[0].n = 0;
	
	while (stackPtr != 0)
	{
		--stackPtr;
		StackEntry* s = stack + stackPtr;
		const size_t n = s->n;
		const global Node* node = nodes + n;
		const int cd = node->dim;
		if (cd == -2)
			continue;
		const int index = node->index;
		const global T* p = &cloud[index * POINT_STRIDE];
		// check whether we already have better dist
		if ((s->state == OFFSIDE) && (cd >= 0))
		{
			const T side = q[cd] - p[cd];
			// TODO: use approximate dist
			// FIXME: bug in this early out
			//if (side*side > heapHeadValue(heap))
			//	continue;
		}
		// compute new distance and update if lower
		T dist = 0;
		for (uint i = 0; i < DIM_COUNT; ++i)
		{
			const T diff = q[i] - p[i];
			dist += diff * diff;
		}
		if (dist < heapHeadValue(heap) &&
			(allowSelfMatch || (dist > (T)EPSILON)))
			heapHeadReplace(heap, index, dist, K);
		// look for recursion
		if (cd >= 0)
		{
			const T side = q[cd] - p[cd];
			// should we enqueue off side?
			if (side*side < heapHeadValue(heap))
			{
				// yes
				s->state = OFFSIDE;
				if (side >= 0)
					s->n = childLeft(n);
				else
					s->n = childRight(n);
				++stackPtr;
			}
			// always enqueue on side
			s = stack + stackPtr;
			s->state = ONSIDE;
			if (side >= 0)
				s->n = childRight(n);
			else
				s->n = childLeft(n);
			++stackPtr;
		}
	}
	
	if (doSort)
		heapSort(heap);
	heapCopy(&indices[queryId * indexStride], &dists2[queryId * dists2Stride], heap, K);
}

