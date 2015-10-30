#include "MathUtils.h"

#include <iostream>
#include <Eigen/Sparse>
	
class Solver {
	public:
		Solver(){}
		~Solver(){}
		
		void set(uint n);
		void solve();

		Eigen::VectorXf B;
		Eigen::VectorXf X;
		Eigen::SparseMatrix<float> A;
};

void Solver::set(uint n){
	B = Eigen::VectorXf(n);
	X = Eigen::VectorXf(n);
	A = Eigen::SparseMatrix<float>(n,n);
	A.reserve(Eigen::VectorXi::Constant(n,5));
}

void Solver::solve(){
	Eigen::ConjugateGradient<Eigen::SparseMatrix<float> > cg;
	cg.compute(A);
	X = cg.solve(B);
	/*std::cout << "#iterations:     " << cg.iterations() << std::endl;
	std::cout << "estimated error: " << cg.error()      << std::endl;
	std::cout << B << std::endl;
	std::cout << "_____\n";
	std::cout << X << std::endl;
	std::cout << A << std::endl;*/
}
