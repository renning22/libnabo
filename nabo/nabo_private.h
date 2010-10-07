/*

Copyright (c) 2010, Stephane Magnenat, ASL, ETHZ, Switzerland
You can contact the author at <stephane at magnenat dot net>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ETH-ASL BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __NABO_PRIVATE_H
#define __NABO_PRIVATE_H

#include "nabo.h"


/*!	\file nabo_private.h
	\brief header for implementation
	\ingroup private
*/

namespace Nabo
{
	//! \defgroup private private implementation
	//@{
	
	//! Euclidean distance
	template<typename T, typename A, typename B>
	inline T dist2(const A& v0, const B& v1)
	{
		return (v0 - v1).squaredNorm();
	}

	//! Brute-force nearest neighbour
	template<typename T>
	struct BruteForceSearch: public NearestNeighbourSearch<T>
	{
		typedef typename NearestNeighbourSearch<T>::Vector Vector;
		typedef typename NearestNeighbourSearch<T>::Matrix Matrix;
		typedef typename NearestNeighbourSearch<T>::Index Index;
		typedef typename NearestNeighbourSearch<T>::IndexVector IndexVector;

		//! constructor, calls NearestNeighbourSearch<T>(cloud)
		BruteForceSearch(const Matrix& cloud);
		virtual IndexVector knn(const Vector& query, const Index k, const T epsilon, const unsigned optionFlags);
	};
	
	//! KDTree, unbalanced, points in leaves, stack, implicit bounds, ANN_KD_SL_MIDPT, optimised implementation
	template<typename T, typename Heap>
	struct KDTreeUnbalancedPtInLeavesImplicitBoundsStackOpt: public NearestNeighbourSearch<T>
	{
		typedef typename NearestNeighbourSearch<T>::Vector Vector;
		typedef typename NearestNeighbourSearch<T>::Matrix Matrix;
		typedef typename NearestNeighbourSearch<T>::Index Index;
		typedef typename NearestNeighbourSearch<T>::IndexVector IndexVector;
		typedef typename NearestNeighbourSearch<T>::IndexMatrix IndexMatrix;
		
		using NearestNeighbourSearch<T>::statistics;
		using NearestNeighbourSearch<T>::cloud;
		using NearestNeighbourSearch<T>::minBound;
		using NearestNeighbourSearch<T>::maxBound;
		
	protected:
		//! indices of points during kd-tree construction
		typedef std::vector<Index> BuildPoints;
		//! iterator to indices of points during kd-tree construction
		typedef typename BuildPoints::iterator BuildPointsIt;
		//! const-iterator to indices of points during kd-tree construction
		typedef typename BuildPoints::const_iterator BuildPointsCstIt;
		
		//! search node
		struct Node
		{
			enum
			{
				INVALID_CHILD = 0xffffffff,
				INVALID_PT = 0
			};
			Index dim; //!< cut dimension for split nodes, index of point for leaf nodes
			unsigned rightChild; //!< index of right node, left index is current+1
			union
			{
				T cutVal; //!< for split node, split value
				const T* pt; //!< for leaf node, pointer to data-point coordinates
			};
			
			//! construct a split node
			Node(const Index dim, const T cutVal, unsigned rightChild):
				dim(dim), rightChild(rightChild), cutVal(cutVal) {}
			//! construct a leaf node
			Node(const Index index = 0, const T* pt = 0):
				dim(index), rightChild(INVALID_CHILD), pt(pt) {}
		};
		//! dense vector of search nodes, provides better memory performances than many small objects
		typedef std::vector<Node> Nodes;
		
		//! search nodes
		Nodes nodes;
		
		//! return the bounds of points from [first..last[ on dimension dim
		std::pair<T,T> getBounds(const BuildPointsIt first, const BuildPointsIt last, const unsigned dim);
		//! construct nodes for points [first..last[ inside the hyperrectangle [minValues..maxValues]
		unsigned buildNodes(const BuildPointsIt first, const BuildPointsIt last, const Vector minValues, const Vector maxValues);
		
		//! recursive search, strongly inspired by ANN and [Arya & Mount, Algorithms for fast vector quantization, 1993]
		/**	\param query pointer to query coordinates 
		 * 	\param n index of node to visit
		 * 	\param rd squared dist to this rect
		 * 	\param heap reference to heap
		 * 	\param off reference to array of offsets
		 * 	\param maxError error factor (1 + epsilon) */
		template<bool allowSelfMatch>
		void recurseKnn(const T* query, const unsigned n, T rd, Heap& heap, std::vector<T>& off, const T maxError);
		
	public:
		//! constructor, calls NearestNeighbourSearch<T>(cloud)
		KDTreeUnbalancedPtInLeavesImplicitBoundsStackOpt(const Matrix& cloud);
		virtual IndexVector knn(const Vector& query, const Index k, const T epsilon, const unsigned optionFlags);
		virtual IndexMatrix knnM(const Matrix& query, const Index k, const T epsilon, const unsigned optionFlags);
	};
	
	//@}
}

#endif // __NABO_H
