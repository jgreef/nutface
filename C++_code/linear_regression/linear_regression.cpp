



#include <boost/numeric/ublas/operation_blocked.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;
using namespace std;
namespace ublas = boost::numeric::ublas;

 /* Matrix inversion routine.
 Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
template<class T>
bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse)
{
    typedef permutation_matrix<std::size_t> pmatrix;

    // create a working copy of the input
    matrix<T> A(input);

    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());

    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0)
        return false;

    // create identity matrix of "inverse"
    inverse.assign(identity_matrix<T> (A.size1()));

    // backsubstitute to get the inverse
    lu_substitute(A, pm, inverse);

    return true;
}
/*
template <typename T, typename F=row_major>
matrix<T, F> makeMatrix(size_t m, size_t n, const vector<T> & v)
{
    if(m*n!=v.size()) {
        ; // Handle this case
    }
    unbounded_array<T> storage(m*n);
    copy(v.begin(), v.end(), storage.begin());
    return matrix<T>(m, n, storage);
}
*/

template<class T>
bool get_weight(const matrix<T>& data, const ublas::vector<T>& target, ublas::vector<T>& weights) {
    matrix<T> data_trans(data.size2(), data.size1());

    data_trans = trans(data);
    
    matrix<T> temp(data.size2(), data.size2()), inv(data.size2(), data.size2());

    temp = block_prod<matrix<T>, 3> (data_trans, data);
    cout << data << endl;
    cout << data_trans << endl;
    cout << temp << endl;
    InvertMatrix(temp, inv);


    //data_trans = block_prod<matrix<T>, 64> (temp, data_trans);
    
    //weights = block_prod<matrix<T>, 64> (data_trans, target);
     
    cout << data << endl;
    cout << data_trans << endl;
    cout << temp << endl;
    return true;
}

#include "storage_adaptors.hpp"
int main()
{
   double initialValues[6] = {
        1, 2, 3,
        5, 1, 4
    };

    matrix<double> A(2, 3), Z(3, 3), C(3,3);
    ublas::vector<double> v(3), w(3);
    for(int i = 1; i < 4; i++) {
        v(i-1) = i;
        w(i-1) = i*i;
    }
    A = make_matrix_from_pointer(2,3, &initialValues[0]);
    get_weight(A, v, w);
    /*InvertMatrix(A, Z);
    C = block_prod<matrix<double>, 3> (A, Z);
    cout << "A=" << A << endl << "Z=" << Z <<  "C=" << C << endl;
*/
    return 0;
}
