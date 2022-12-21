# matrix_code_generator.py

class SquareMatrix(object):
    def __init__(self, size):
        self.size = size
        self.ele = [['matrix.ele[%d][%d]' % (i, j) for j in range(size)] for i in range(size)]

    def calc_sub_matrix(self, r, c):
        sub_mat = SquareMatrix(self.size - 1)
        for i in range(self.size):
            for j in range(self.size):
                if i != r and j != c:
                    sub_mat.ele[i if i < r else i - 1][j if j < c else j - 1] = self.ele[i][j]
        return sub_mat

    def calc_determinant(self):
        if self.size == 1:
            return self.ele[0][0]
        elif self.size == 2:
            return self.ele[0][0] + '*' + self.ele[1][1] + ' - ' + self.ele[1][0] + '*' + self.ele[0][1]
        else:
            det = ''
            for j in range(self.size):
                if j > 0:
                    if j % 2 == 1:
                        det += ' - '
                    else:
                        det += ' + '
                det += self.ele[0][j] + '*('
                det += self.calc_sub_matrix(0, j).calc_determinant()
                det += ')'
            return det

    def calc_inverse(self):
        inv_mat = SquareMatrix(self.size)
        for i in range(self.size):
            for j in range(self.size):
                sub_mat = self.calc_sub_matrix(i, j)
                sub_mat_det = sub_mat.calc_determinant()
                cofactor = ('(' if (i + j) % 2 == 0 else '-(') + sub_mat_det + ')'
                inv_mat.ele[j][i] = cofactor + ' / det';
        return inv_mat

if __name__ == '__main__':
    matrix = SquareMatrix(4)
    #det = matrix.calc_determinant()
    #print(det)
    mat_inv = matrix.calc_inverse()
    for i in range(matrix.size):
        for j in range(matrix.size):
            print('this->ele[%d][%d] = %s;' % (i, j, mat_inv.ele[i][j]))