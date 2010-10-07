// This example is in the public domain

#include "nabo/nabo.h"

int main()
{
	using namespace Nabo;
	using namespace Eigen;
	
	// 100 points in 3D
	MatrixXf M = MatrixXf::Random(3, 100);
	// 1 query points
	VectorXf q = VectorXf::Random(3);
	
	// create a kd-tree for M, note that M must stay valid during the lifetime of the kd-tree
	NNSearchF* nns = NNSearchF::createKDTreeLinearHeap(M);
	// look for the 5 nearest neighbour
	const int K = 5;
	VectorXi n = nns->knn(q, K);
	
	// cleanup kd-tree
	delete nns;
	
	return 0;
}