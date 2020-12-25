#include <iostream>
#include <vector>
#include <cmath> 
#include <algorithm>
#include <chrono>
#include <fstream>
#include "mpi.h"

using namespace std;

// классическое правило Эратосфена
vector<long int> Eratosthenes(long int n, vector<bool>& SIEVE)
{
    vector<long int> result;
    SIEVE[0] = SIEVE[1] = false;

    for (long int i = 2; i < n; ++i)
    {
        for (int j = i + i; j < n; j += i)
        {
            SIEVE[j] = false;
        }
        if (SIEVE[i])
            result.push_back(i);
    }
    return result;
}

// выполнение проверки в заданных границах
void EratosthenesTick(long int start, long int end, long int* base_primes, long int base_size, bool* local_sieve)
{
    long int size = end - start;

    for (long int i = start; i < end; ++i)
    {
        for (long int j = 0; j < base_size; ++j)
        {
            if (i % base_primes[j] == 0)
            {
                local_sieve[i - start] = false; // в локальном массиве ставим ложь
            }
        }
    }
}

// Работа с блоком
void BlockBolting(long int border, long int step, int ProcRank, bool *local_sieve, long int* base_primes, long int base_prime_size)
{
    long int first_elem = floor(border + step * (ProcRank));
    long int last_elem = floor(border + step * (ProcRank + 1));
    cout << "~Proc" << ProcRank << " deals with [" << first_elem << ";" << last_elem << "]" << endl;
    EratosthenesTick(first_elem, last_elem, base_primes, base_prime_size, local_sieve);
}

void main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    ofstream fout;
    MPI_Status status;
    
    vector<long int> SIZE = { 10000 }; //{ 1000, 10000, 100000, 1000000, 10000000 };
  
    long int border; // граница sqrt(n)
    long int step;  // фактически длина блока
    int ProcNum; // число процессов
    int ProcRank; // номер процесса


    long int* base_primes; // базовые простые числа
    vector<long int> primes; // все праймы
    vector<bool> SIEVE; // полное решето
    bool* local_sieve; // местный массив

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
    {
        cout << "_MPI variant_ "<< endl;
        cout << "ProcNum: " << ProcNum << endl;
        cout << "--------------------" << endl;
    }

    for (int curSize = 0; curSize < SIZE.size(); ++curSize)
    {
        SIEVE.resize(SIZE[curSize], true);
        SIEVE[0] = SIEVE[1] = false;

        // находим базовые простые
        border = floor(sqrt(SIZE[curSize]));
        vector<long int> v_base_primes = Eratosthenes(border, SIEVE);
        long int base_prime_size = v_base_primes.size();
        base_primes = (long int*)malloc(base_prime_size * sizeof(long int));
        copy(v_base_primes.begin(), v_base_primes.end(), base_primes);

        // нулевой поток
        if (ProcRank == 0)
        {
            step = floor((SIZE[curSize] - border) / (ProcNum)); // вычисление размера одного блока
            auto begin = chrono::high_resolution_clock::now();

            // отправка на потоки
            for (int i = 1; i < ProcNum; ++i)
            {
                MPI_Send(&step, 1, MPI_LONG_INT, i, 50, MPI_COMM_WORLD);
            }
            // работа со своим блоком
            local_sieve = (bool*)malloc(step * sizeof(bool));
            BlockBolting(border, step, ProcRank, local_sieve, base_primes, base_prime_size);
            for (int i = 0; i < step; ++i)
            {
                SIEVE[border + i] = local_sieve[i];
            }
            // приём и обработка
            for (int i = 1; i < ProcNum; ++i)
            {

                bool* rbuf; // буфер для приёма данных из потоков
                rbuf = (bool*)malloc(step * sizeof(bool));

                cout << "~root" << ProcRank << " recieving from <= " << i << endl;

                // получаем данные в буфер
                MPI_Recv(rbuf, step, MPI_C_BOOL, i, 50, MPI_COMM_WORLD, &status);

                cout << "~root" << ProcRank << " recieved data from " << i << endl;

                // на основе полученнных данных, изменяем решето
                for (int j = 0; j < step; j++)
                {
                    SIEVE[border + (i - 1) * step + j] = rbuf[j];
                }

                free(rbuf);
            }
            auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin).count();
            cout << "--------------------" << endl;
            cout << "Size: " << SIZE[curSize] << endl;
            cout << "Time: " << elapsedTime << "ms" << endl;
            cout << "--------------------" << endl;
        }
        else
        {
         

            // принимаем данные из потока 0
            MPI_Recv(&step, 1, MPI_LONG_INT, 0, 50, MPI_COMM_WORLD, &status);

            cout << "~Proc" << ProcRank << " + recieved data from root0: " << endl;

            local_sieve = (bool*)malloc(step * sizeof(bool));
            BlockBolting(border, step, ProcRank, local_sieve, base_primes, base_prime_size);

            cout << "~Proc" << ProcRank << " sending... => root0" << endl;

            // посылаем локальные данные в нулевой поток
            MPI_Send(local_sieve, step, MPI_C_BOOL, 0, 50, MPI_COMM_WORLD);

            cout << "~Proc" << ProcRank << " already send => root0" << endl;
        }
        free(base_primes);
        free(local_sieve);
        /*
        // вывод в файл
        if (ProcRank == 0)
        {
            ofstream fout;
            fout.open("file.txt", ios::app);
            fout << endl;
            fout << SIZE << endl;
            for (int i = 0; i < SIEVE.size(); i++)
            {
                if (SIEVE[i])
                    fout << i << " ";
            }
            fout << endl;
            fout.close();
        }
         */
    }
    
   
    
    MPI_Finalize();
}

