#include "det.h"
#include <memory>
#include <functional>
#include <algorithm>


//with extra memory allocation (minors)
double det_1(double** matrix, const uint32_t size)
{

    if (size == 1)
        return matrix[0][0];
    if (size == 2)
        return matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1];

    auto minor = [&matrix, size](uint32_t k, uint32_t g)
    {
        std::unique_ptr<double*, std::function<void(double**)>> result
        (
            new double* [size - 1](),
            [size](double** self)
            {
                std::for_each(self, self + size - 1, std::default_delete<double[]>());
                delete[] self;
            }
        );

        for (uint32_t i = 0; i < size - 1; ++i)
        {
            result.get()[i] = new double[size - 1]();
            for (uint32_t j = 0; j < size - 1; j++)
            {
                uint8_t drow = (i >= k ? 1 : 0);
                uint8_t dcol = (j >= g ? 1 : 0);
                result.get()[i][j] = matrix[i + drow][j + dcol];
            }
        }

        return result;
    };

    double result = 0.0;
    for (uint32_t i = 0; i < size; ++i)
        result += ((i + 1) & 1 ? 1 : -1) * matrix[0][i] * det_1(minor(0, i).get(), size - 1);

    return result;
}

//without extra memory allocation
double det_2(double** matrix, const uint32_t size)
{
    if (size == 1)
        return matrix[0][0];

    std::function<double(double**, uint32_t, uint32_t, uint32_t, uint32_t, bool*)> determinant
    {
        [&determinant](double** l_matrix, uint32_t l_size, uint32_t row, uint32_t s_col, uint32_t e_col, bool* check_ar)
        {
            while (check_ar[s_col])
                s_col++;

            while (check_ar[e_col])
                e_col--;

            if (l_size == 2)
                return l_matrix[row][s_col] * l_matrix[row + 1][e_col] - l_matrix[row + 1][s_col] * l_matrix[row][e_col];

            double result = 0;
            for (uint32_t i = 0, di = 0; i < l_size; ++i, ++di)
            {
                while (check_ar[s_col + di])
                    di++;

                check_ar[s_col + di] = 1;
                result += ((i + 1) & 1 ? 1 : -1) * l_matrix[row][s_col + di] * determinant(l_matrix, l_size - 1, row + 1, s_col, e_col, check_ar);
                check_ar[s_col + di] = 0;
            }

        return result;
        }
    };

    std::unique_ptr<bool> check_column_array(new bool[size]());
    return determinant(matrix, size, 0, 0, size - 1, check_column_array.get());
}

//without extra memory allocation & non-recursive function
//the fastest one
double det_3(double** matrix, const uint32_t size)
{
    auto nextReshuffle = [&size](int32_t* arr) -> bool
    {

        int32_t i = size - 2;
        uint32_t j = 0;

        while (i > -1 && arr[i] > arr[i + 1])
            i--;

        if (i > -1)
        {

            j = i + 1;

            while (j < size && arr[j + 1] > arr[i])
                j++;

            //swap(arr[i], arr[j]);
            arr[i] ^= arr[j] ^= arr[i] ^= arr[j];

            for (j = i + 1; j <= (size + i - 1) / 2; j++)
            {
                //swap(arr[j], arr[dimension - j + i]);
                arr[j] ^= arr[size - j + i] ^= arr[j] ^= arr[size - j + i];
            }

            return true;
        }

        return false;
    };


    double determinant = 0.0;

    //the array of indexes for inversions counting
    int32_t* indexes = new int32_t[size]();

    for (size_t i = 0; i < size; i++)
        indexes[i] = i;

    do
    {
        //number of inversions
        uint32_t inversions = 0;

        double term = 1.0;

        //inversions counting
        for (size_t j = 0; j < size - 1; j++)
            for (size_t k = j + 1; k < size; k++)
                if (indexes[j] > indexes[k])
                    inversions++;

        for (size_t j = 0; j < size; j++)
            term *= matrix[indexes[j]][j];

        //odd part in the determinant sum must be multiplied by -1
        if (inversions & 1)
            term *= -1;

        determinant += term;

    } while (nextReshuffle(indexes));

    delete[] indexes;

    return determinant;
}
