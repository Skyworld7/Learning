#include<iostream>
#include<complex>
#include<mpi/mpi.h>
#include<fstream>
using namespace std;
const int data_tag = 0;
const int result_tag = 1;
const int terminate_tag = 2;
const int num_x = 640;
const int num_y = 480;
const float scale_x = 1.0 / 160;
const float scale_y = 1.0 / 160;
const complex<float> min_z = { -2.0,-1.5 };
struct Message {
	int id;
	int row;
	int color[num_x];
};
int main(int argc, char* argv[]) {
	int cal_pixel(complex<float> c);

	int id;//���̱��
	int num_proc;//������
	int count; // ���ڽ��еĽ���
	int row; // �������͵ľ����к�
	int picture[num_y][num_x];
	MPI_Status stat;
	Message result;//���ݽ��
	complex<float> c;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	MPI_Datatype Messagetype;
	MPI_Datatype oldtype[2] = { MPI_INT,MPI_INT };
	int blockcount[2] = { 2,num_x };
	MPI_Aint offset[2] = { 0,2 * sizeof(int) };
	MPI_Type_struct(2, blockcount, offset, oldtype, &Messagetype);
	MPI_Type_commit(&Messagetype);

	if (!id) {//������
		count = 0;
		row = 0;
		for (int i = 1; i < num_proc; i++) { // ���ͳ�ʼ���и�������
			MPI_Send(&row, 1, MPI_INT, i, data_tag, MPI_COMM_WORLD);
			count++;
			row++;
		}

		do {
			MPI_Recv(&result, 1, Messagetype, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD, &stat);//���ܸ����̵�������
		  //  cout<<"Recv row:"<<result.row<<" id: "<<result.id<<" Tag:"<<stat.MPI_TAG<<endl;
			--count;//�ڼ���Ľ�����-1
			for (int i = 0; i < num_x; ++i) {//���������
				picture[result.row][i] = result.color[i];
			}
			if (row < num_y) {
				//���������û������
				MPI_Send(&row, 1, MPI_INT, stat.MPI_SOURCE, data_tag, MPI_COMM_WORLD);//�ոս�������Ľ���stat.MPI_SOURCE����������
				++count;//���������+1
				++row;//������һ����
			}
			else {//���е������Ѿ�������
				MPI_Send(&row, 1, MPI_INT, stat.MPI_SOURCE, terminate_tag, MPI_COMM_WORLD);//������ֹ�ź�
			}
		} while (count > 0);
		ofstream f1("Mandelbrot.txt", ios::out);//������
		for (int i = 0; i < num_y; ++i) {
			for (int j = 0; j < num_x; ++j) {
				f1 << picture[i][j] << " ";
			}
			f1 << endl;
		}
		f1.close();
	}
	else {
		MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

		while (stat.MPI_TAG == data_tag) {
			c.imag(min_z.imag() + row * scale_y);
			for (int i = 0; i < num_x; ++i) {
				c.real(min_z.real() + i * scale_x);
				result.color[i] = cal_pixel(c);
			}
			//cout<<id<<":"<<endl;
			result.id = id;
			result.row = row;
			MPI_Send(&result, 1, Messagetype, 0, result_tag, MPI_COMM_WORLD);
			MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
		}
	}
	MPI_Finalize();//����MPI��������
	return 0;
}
int cal_pixel(complex<float> c) {
	int count = 0; // number of iterations
	int max = 256; // maximum iteration is 256
	complex<float> z{ 0,0 }; // initialize complex number z
	do {
		z = z * z + c;
		count++; // update iteration counter
	} while ((norm(z) < 4.0) && (count < max));
	return count;
}